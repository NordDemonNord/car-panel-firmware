# STM32 pin assignment

Board: STM32H743IIT6 core board (LQFP176) with external SDRAM, LCD connector,
microSD slot and onboard USB-UART bridge.
Reference: datasheet DS12110 — Table 9 "Pin/ball definition", Tables 10–20
"Alternate functions".

The board already routes several wide peripherals to fixed pins, so every
candidate pin was checked against them before use:

| Peripheral on board | Pins it occupies (examples) |
|---|---|
| FMC → SDRAM | PD0/PD1 (D2/D3), PE0/PE1, PE7/PE8, … |
| LTDC → LCD connector | PB8/PB9 (B6/B7), PB10/PB11 (G4/G5), PA3 (B5), PH13/PH14 (G2/G3), … |
| SDMMC1 → microSD | PC8–PC12, PD2 |
| USB OTG | PA11/PA12 |
| Onboard USB-UART bridge | PA9/PA10 (USART1, presumed) |

## CAN — FDCAN2

| Signal | Pin | I/O structure |
|---|---|---|
| FDCAN2_TX | PB13 | FT_u |
| FDCAN2_RX | PB12 | FT_u |

The HW-021 (TJA1050) runs from 5 V and drives its RXD output at 5 V, so the
RX pin must be 5 V tolerant (FT). The `_u` suffix only matters when the pin is
used for USB OTG_HS; otherwise it is supplied by VDD. PB13 was checked for
continuity against the onboard 8-pin chip (SPI2_SCK is on the same pin): not
connected.

Rejected FDCAN candidates:

| Pins | Reason |
|---|---|
| PD0/PD1 (FDCAN1) | FMC_D2/D3 — SDRAM data bus |
| PB8/PB9 (FDCAN1) | LCD_B6/B7 |
| PH13/PH14 (FDCAN1) | LCD_G2/G3 — possible LCD conflict |
| PA11/PA12 (FDCAN1) | USB |

The controller runs in classic CAN 2.0 mode: neither the TJA1050 nor the ESP32
TWAI controller supports CAN FD.

Wiring between MCU and CAN transceiver is **not crossed**: MCU TX → transceiver
TXD (its input), transceiver RXD (its output) → MCU RX.

## UART to the Raspberry Pi — USART2

| Signal | Pin | Connects to |
|---|---|---|
| USART2_TX | PD5 | CP2102 RX |
| USART2_RX | PD6 | CP2102 TX |

UART wiring **is crossed**. Common GND; the adapter is set to 3.3 V logic and
its VCC pin is left unconnected (the board is powered from its own USB).

PD5 doubles as FMC_NWE, which SDRAM does not use. PD6 doubles as LCD_B2 —
unused in RGB565, but used if the LCD connector is wired for 24 bit.
**TODO:** continuity-check PD6 against the LCD connector.

Rejected UART candidates:

| Pins | Reason |
|---|---|
| PA9/PA10, PB14/PB15 (USART1) | USART1 is the onboard debug bridge |
| PB10/PB11 (USART3) | LCD_G4/G5 |
| PA2/PA3 (USART2) | PA3 = LCD_B5 |
| PC10/PC11, PC12/PD2 (UART4/5) | SDMMC1 — microSD |
| PE7/PE8, PE0/PE1 (UART7/8) | FMC — SDRAM |

## Other

| Function | Pin | Notes |
|---|---|---|
| User LED | PC13 | in the backup power domain: limited drive, should sink rather than source current — likely active-low |
| HSE | 25 MHz crystal | |
| Debug | SWD (ST-Link) | ST-Link 3.3 V output not connected; board powered from USB so the 5V header pin is live |

The pin labels on the header are printed on the back of the board and partly
abbreviated ("B13" = PB13). Orient by the 5V/GND corner.
