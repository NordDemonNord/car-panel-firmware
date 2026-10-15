# Car Panel Firmware

Firmware and tools for the microcontroller nodes of the
[Car Dashboard](https://github.com/NordDemonNord/car-panel-os) project:
the STM32 instrument cluster, the ESP32 CAN signal simulator, and host-side
scripts for driving the CAN bus from a PC.

This is a learning project. The STM32 firmware is written at register level —
CMSIS headers only, no HAL, no code generators — to understand the hardware
rather than hide it.

---

## System overview

```
   [PC + CANable]  or  [ESP32 signal simulator]
            │  CAN bus, classic CAN 2.0, 120 Ω at both ends
            ▼
   ┌──────────────────────┐   UART (USART2)   ┌──────────────────────┐
   │  STM32H743IIT6       │──────────────────▶│  Raspberry Pi 4      │
   │  bare-metal          │  via CP2102       │  Yocto Linux, Qt 6   │
   │  instrument cluster  │  USB-UART bridge  │  infotainment        │
   └──────────────────────┘                   └──────────────────────┘
```

The STM32 receives vehicle data over CAN, drives the cluster display, and
forwards the data the infotainment needs to the Raspberry Pi over UART.
The Raspberry Pi side lives in
[car-panel-os](https://github.com/NordDemonNord/car-panel-os).

## Repository layout

```
common/          definitions shared by all nodes: CAN message IDs and
                 payload layout, STM32 → Pi UART protocol
stm32-cluster/   STM32H743IIT6 firmware (bare-metal, CMSIS only)
esp32-can-sim/   ESP32 CAN signal simulator (planned)
tools/canable/   SocketCAN scripts for the CANable adapter
docs/            protocol specs, wiring, measurements
```

`common/` is the single source of truth for everything that crosses a wire:
every node includes the same definitions, so the IDs and frame layouts cannot
drift apart.

## Hardware

| Node | Part | Notes |
|---|---|---|
| Cluster MCU | STM32H743IIT6 core board | Cortex-M7, 2 MB flash, 1 MB RAM, external SDRAM, LCD connector, HSE 25 MHz |
| CAN transceiver | HW-021 (TJA1050) | 5 V supply; RXD outputs 5 V → MCU RX pin must be 5 V tolerant |
| PC CAN adapter | CANable | built-in transceiver and switchable 120 Ω terminator |
| UART bridge | CP2102 | STM32 → Raspberry Pi, 3.3 V logic |
| Debug probe | ST-Link | SWD |

### STM32 pin assignment

| Function | Pin | Notes |
|---|---|---|
| FDCAN2_TX | PB13 | → HW-021 TX (no crossover) |
| FDCAN2_RX | PB12 | ← HW-021 RX, FT pin |
| USART2_TX (to Pi) | PD5 | → CP2102 RX |
| USART2_RX (from Pi) | PD6 | ← CP2102 TX |
| USART1_TX (debug console) | PA9 (AF7) | → CP2102 RX on the header, 115200 8N1 |
| USART1_RX (debug console) | PA10 (AF7) | ← CP2102 TX |
| User LED | PC11 | external LED, active-high (PC11 → resistor → LED → GND) |

Pins were chosen to avoid the peripherals already wired on the board:
FMC (SDRAM), LTDC (LCD connector), SDMMC1 (microSD) and USB. The rejected
candidates and the reasons are in [docs/pin-assignment.md](docs/pin-assignment.md).

### CAN bus wiring

| From | To |
|---|---|
| CANable CAN_H / CAN_L | HW-021 CAN_H / CAN_L |
| CANable GND | STM32 GND |
| HW-021 VCC / GND | STM32 board 5V / GND |

Terminators enabled on both ends (CANable switch, HW-021 jumper) — about
60 Ω between CAN_H and CAN_L with power off.

## Toolchain

- `arm-none-eabi-gcc` — cross-compiler
- CMSIS Core (Arm) and `cmsis_device_h7` (ST) — git submodules, pinned to tags
- OpenOCD + ST-Link — flashing (`make flash`), also the GDB server
- `gdb-multiarch` + VS Code Cortex-Debug with the STM32H743 SVD — debugging

## Build and flash

```
git submodule update --init   # CMSIS headers
cd stm32-cluster
make          # build/cluster.elf
make flash    # program over ST-Link (OpenOCD), verify, reset
make clean
```

## Serial console

USART1 is the debug console: 115200 8N1, through a CP2102 on `/dev/ttyUSB0`.

```
picocom -b 115200 /dev/ttyUSB0      # exit: Ctrl+A, Ctrl+X
ls -l /dev/serial/by-id/            # which ttyUSB* is which adapter
```

The user needs to be in the `dialout` group for `/dev/ttyUSB*`.

## Debugging

OpenOCD runs as a server: telnet on port 4444, GDB on port 3333.

```
# terminal 1
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg

# terminal 2, from stm32-cluster/
gdb-multiarch build/cluster.elf
(gdb) target extended-remote localhost:3333
(gdb) monitor reset halt
(gdb) break sysclk_switch
(gdb) continue
```

Quick register dump without an interactive session: run, stop after 2 s,
print the registers. Wrap each command in `echo [...]`, otherwise `-c` mode
prints nothing:

```
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
  -c "init; reset run; sleep 2000; halt; echo [mdw 0x40011000]; exit"
```

Map a raw PC value to a source line:
`arm-none-eabi-addr2line -e build/cluster.elf <pc>`.
Both need `-g` in `CFLAGS`; the debug info stays in the `.elf` and does not
reach the flash.

## Status

- [x] Hardware bring-up: pin assignment, CAN bus wiring and termination
- [x] Raspberry Pi side: CP2102 driver in the image, loopback-tested
- [x] STM32: startup code, linker script, Makefile, LED on PC11 (HSI)
- [x] STM32: power (LDO, VOS1), HSE 25 MHz, PLL1 locked at 400 MHz
- [x] STM32: SYSCLK on PLL1 — flash wait states, bus prescalers (AXI 200 MHz, APB 100 MHz)
- [x] STM32: SysTick 1 ms delay (polling); clock verified — LED at 1 Hz, 10 blinks in 10 s
- [x] STM32: debug console on USART1 (PA9, 115200, TX by polling) — verified in picocom, which also confirms APB2 = 100 MHz
- [ ] STM32: UART link to the Raspberry Pi (USART2, PD5/PD6)
- [ ] STM32: FDCAN in internal loopback
- [ ] PC + CANable → STM32 over CAN
- [ ] CAN message set and UART protocol in `common/`
- [ ] ESP32 CAN signal simulator
