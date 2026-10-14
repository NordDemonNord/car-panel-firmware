/*
    Name:    main.c
    Purpose: main loop of the STM32H743 instrument cluster
    Author:  Nord

    Current state: blink LED on PC11 once per second
    (PC11 -> resistor -> LED -> GND, active-high).
*/

#include "stm32h7xx.h"
#include "bsp.h"
#include <stdint.h>

void delay_ms(uint32_t ms);


int main(void)

{

   bsp_init(); /* bsp for HSE configuration */

   /* Enable the GPIOC clock (AHB4): without it the port ignores every write */
   RCC->AHB4ENR |= RCC_AHB4ENR_GPIOCEN;

   /* Dummy read: the port needs a few cycles after the clock is enabled
      before it responds (STM32H7 errata ES0392) */
   (void)RCC->AHB4ENR;

   /* PC11 = general purpose output (MODER11 = 01).
      Port C resets to 11 (analog), so clear both bits before setting */
   GPIOC->MODER &= ~GPIO_MODER_MODER11;
   GPIOC->MODER |= GPIO_MODER_MODER11_0;

      while (1)
   {
      GPIOC->BSRR = GPIO_BSRR_BS11;   /* LED on */
      delay_ms(500);

      GPIOC->BSRR = GPIO_BSRR_BR11;   /* LED off */
      delay_ms(500);
   }
}
