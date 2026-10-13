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
