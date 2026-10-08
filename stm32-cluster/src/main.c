/*
    Name:    main.c
    Purpose: main loop of the STM32H743 instrument cluster
    Author:  Nord

    Current state: turns on an external LED on PC11
    (PC11 -> resistor -> LED -> GND, active-high).
*/

#include "stm32h7xx.h"

int main(void)
{
    /* Enable GPIOC clock (AHB4). Without it the port ignores every write. */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;

    /* Dummy read: the port needs a few cycles after the clock is enabled
       before it responds (STM32H7 errata ES0392). */
    (void)RCC->AHB4ENR;

    /* PC11 -> general-purpose output (MODER11 = 01).
       Port C resets to 11 (analog), so clear both bits before setting bit 0. */
    GPIOC->MODER &= ~GPIO_MODER_MODER11;
    GPIOC->MODER |= GPIO_MODER_MODER11_0;

    /* Drive PC11 high to light the LED. BSRR is a single write,
       no read-modify-write. */
    GPIOC->BSRR = GPIO_BSRR_BS11;

    /* No OS to return to: main must never exit. */
    while (1)
    {
    }
}
