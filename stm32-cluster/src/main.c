/*
    Name:    main.c
    Purpose: main loop of the STM32H743 instrument cluster
    Author:  Nord
*/

#include "stm32h7xx.h"

int main(void)
{
    /* 1. Enable the clock of the whole GPIOC port (bus AHB4).
          Without a clock the port ignores every write. */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;

    /* 2. Make PC13 an output.
          MODER has 2 bits per pin; pin 13 uses bits 27..26:
            00 = input (reset value)
            01 = general-purpose output   <- we need this
            10 = alternate function
            11 = analog
          Step 2a: clear both bits -> 00 */
    GPIOC->MODER &= ~GPIO_MODER_MODE13;
    /*    Step 2b: set the lower bit -> 01 */
    GPIOC->MODER |= GPIO_MODER_MODE13_0;

    /* 3. Turn the LED on.
          The LED on PC13 is most likely active-low (lit when the pin is 0),
          so we drive the pin LOW. BSRR: writing 1 to BR13 resets PC13 to 0. */
    GPIOC->BSRR = GPIO_BSRR_BR13;

    /* 4. main() must never return on a microcontroller: there is no OS to return to. */
    while (1)
    {
    }
}