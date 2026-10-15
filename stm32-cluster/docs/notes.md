# Firmware notes

Lessons learned while writing the register-level firmware.

## Waiting for a status bit: compare with 0, never with 1

```c
while ((PWR->CSR1 & PWR_CSR1_ACTVOSRDY) == 0)
{
}
```

`reg & MASK` keeps only the bit under the mask and clears everything else:

- bit is 0 -> result is `0`
- bit is 1 -> result is **the mask itself** (e.g. `0x2000` for bit 13), not `1`

So `== 1` only works for bit 0. For any other bit it never becomes true and
the loop hangs forever. Always test against zero: `== 0` to wait while the bit
is clear, `!= 0` to wait while it is set.

The loop is not optimised away because CMSIS declares peripheral registers
`volatile` (`__IO`): the compiler re-reads the register on every iteration.

Weak spot: if the bit never comes up, the MCU hangs silently. Production code
adds a timeout.

## Single-write registers: build the value first

`PWR_CR3` accepts only one write of the supply configuration after power-on.
The usual `&=` then `|=` is two writes, and the first one locks a half-built
value. Read into a variable, modify it, write once:

```c
uint32_t cr3 = PWR->CR3;
cr3 &= ~PWR_CR3_BYPASS;
cr3 |=  PWR_CR3_LDOEN;
PWR->CR3 = cr3;
```

## Divider fields: check the encoding

Not every field stores the value as-is. On the H743:

| Field   | Register        | Stored value |
|---------|-----------------|--------------|
| DIVM1   | RCC_PLLCKSELR   | M            |
| DIVN1   | RCC_PLL1DIVR    | N - 1        |
| DIVP1   | RCC_PLL1DIVR    | P - 1        |

Writing N instead of N - 1 does not hang anything; it silently shifts the
frequency. Always read the field description before writing.

## Writing a multi-bit field: clear, then shift the value in

```c
RCC->PLL1DIVR &= ~RCC_PLL1DIVR_N1_Msk;
RCC->PLL1DIVR |=  (159UL << RCC_PLL1DIVR_N1_Pos);
```

- `_Msk` says where the field is; `_Pos` is the bit number where it starts.
  `_Pos` is not a value — the value is shifted by it.
- `|=` only sets bits, it never clears them. Without the clear, the reset
  value leaks in: DIVM1 resets to 32, so `|= 5` gives 37.
- A single bit needs one operation: `|=` to set, `&= ~` to clear.

The rule does not apply to write-only registers (`BSRR`: plain `=`),
write-1-to-clear flags (`rc_w1`: a read-modify-write clears other flags) and
write-once registers (`PWR_CR3`, see above). The access type is printed above
each bit in the reference manual.

## CMSIS `_0.._N` suffixes are not consistent

- `RCC_PLLCKSELR_DIVM1_5` is bit 5 of the field, i.e. value 32 — not 5.
- `RCC_PLLCFGR_PLL1RGE_2` is the field value 2.

Before using a suffixed macro, check its definition in `stm32h743xx.h`.

## Registers vs fields

A CMSIS peripheral struct has members only for registers. `HPRE`, `D1PPRE`,
`SW` are fields *inside* registers, so `RCC->HPRE` does not compile:

| Field   | Register     |
|---------|--------------|
| HPRE, D1PPRE | RCC_D1CFGR |
| D2PPRE1, D2PPRE2 | RCC_D2CFGR |
| D3PPRE  | RCC_D3CFGR   |
| SW, SWS | RCC_CFGR     |

## HSE: crystal vs external clock

`HSEBYP = 1` is for a ready-made clock signal on OSC_IN: it turns off the
crystal amplifier. The board has a crystal, so `HSEBYP` stays 0.

## Switching SYSCLK to PLL1: order matters

While the core still runs on HSI:

1. Flash wait states, before the frequency goes up. AXI 200 MHz at VOS1:
   `LATENCY = 2`, `WRHIGHFREQ = 10` (a code, not "10 cycles").
   Read `LATENCY` back until it shows the new value.
2. Bus prescalers. `HPRE` resets to /1, which would put AXI/AHB at 400 MHz
   (max 240). `HPRE /2`, every APB `/2` (max 120).
3. `SW = PLL1`, then wait for `SWS = PLL1`. `SW` is the request, `SWS` is the
   hardware confirmation.

| Clock      | Frequency |
|------------|-----------|
| CPU        | 400 MHz   |
| AXI / AHB  | 200 MHz   |
| APB1–4     | 100 MHz   |

## Verifying the clock without an oscilloscope

- Registers: `RCC_CFGR & 0x3F == 0x1B` (SW = SWS = 011, PLL1). This proves the
  configuration, not the frequency.
- Frequency: SysTick from the core clock, `LOAD = 400000 - 1` for 1 ms,
  LED toggled every 500 ms. 10 blinks must take 10 s; on HSI 64 MHz the same
  code would take about 62 s.

## Debugger

- `-g` adds debug info to the `.elf` only; flash size does not change.
- gdb cannot `print RCC->CFGR`: `RCC` is a macro and macros are not in the
  debug info. Read by address: `x/x 0x58024410`.
- Breakpoints in flash are hardware breakpoints (no `BKPT` patching in flash):
  the Cortex-M7 has 8.
- After a halt, check `pc` first (`addr2line`), then `xPSR`: low bits != 0
  means the core is inside an exception (3 = HardFault). An `msp` below
  `0x20000000` means a stack overflow.


## Pin alternate functions: MODER + AFR

A pin driven by a peripheral needs two settings:

1. `MODER = 10` — the pin is controlled by a peripheral, not by `ODR`.
2. `AFSELn` = the AF number — *which* peripheral. 4 bits per pin, so two
   registers: `AFR[0]` (AFRL) for pins 0–7, `AFR[1]` (AFRH) for pins 8–15.

The AF number is in the **datasheet** (DS12110, "Port x alternate functions"
tables), not in the reference manual. PA9 → USART1_TX is AF7.

Forget the AFR write and the pin stays on AF0: the peripheral works, but its
signal never reaches the pin.

Port A resets to `11` (analog) on most pins. Clearing only `_1` and then
setting it leaves `11`. Clear the whole field with `_Msk`.

## USART: baud rate

`BRR = f_ck / baud` with `OVER8 = 0` (16 samples per bit).
`f_ck` for USART1 is APB2 = 100 MHz, `USART_PRESC` resets to /1.

100 000 000 / 115 200 = 868 → real rate 115 207 baud, error 0.006 %.
Anything under about 2 % is fine. Write it as an expression so it follows
the clock: `USART1->BRR = 100000000UL / 115200UL;`.

The CMSIS `DIV_MANTISSA` / `DIV_FRACTION` split is a leftover from older
STM32s. With `OVER8 = 0` the whole register is one number.

`BRR` can only be written while `UE = 0`. The order from the reference manual
("Character transmission procedure"): word length `M` → `BRR` → `UE = 1` →
`TE = 1` → write `TDR`.

## USART: sending a byte

- `TXE` (`USART_ISR_TXE_TXFNF`) = 1 → `TDR` is empty, a new byte can go in.
- `TC` = 1 → everything has left the wire, stop bit included. Only needed
  before switching the USART off.
- Write `TDR` with `=`, never `|=`: it is a data register, a read-modify-write
  mixes the old byte into the new one.
- `'A'` is one character (65). `"A"` is a string: an address in memory.

## UART wiring

TX is an output, RX is an input, so the wires cross: STM TX → adapter RX.
TX → TX connects two outputs and nothing is received. If nothing arrives,
swap the two wires first: it costs nothing at 3.3 V.

Isolating a silent link without a scope:

1. Adapter loopback: short its TX and RX, type in picocom. Characters come
   back → adapter, port and terminal are fine (picocom has local echo off).
2. MCU registers via OpenOCD: GPIOA `MODER`/`AFRH`, USART1 `CR1`/`BRR`/`ISR`.
   `TC = 1` in `ISR` proves the byte has left the USART, so the fault is on
   the wire side.
