/*
    Name:    main.c
    Purpose: main loop of the STM32H743 instrument cluster
    Author:  Nord

    Current state: USART1 debug console, TX only, polling.
    Sends 'A' every 500 ms at 115200 8N1.
    PA9 (USART1_TX) -> CP2102 RX, common GND.
*/

#include "stm32h7xx.h"
#include "bsp.h"
#include <stdint.h>

void delay_ms(uint32_t ms);
void usart1_init(void);
void usart1_transmit(uint8_t b);


int main(void)

{

   bsp_init(); /* power, HSE, PLL1 400 MHz, buses, SysTick */

   usart1_init();

   while (1)
   {
      usart1_transmit('A');
      delay_ms(500);
   }

}


/* --- USART1 --- */
/*
   PA9  - USART1_TX (AF7)
   PA10 - USART1_RX (AF7)
   Kernel clock: APB2 (rcc_pclk2) = 100 MHz, USART_PRESC = /1 (reset value)
*/

void usart1_init(void)

{

   RCC->AHB4ENR |= RCC_AHB4ENR_GPIOAEN_Msk; /* enable clock for GPIOA */
   (void)RCC->AHB4ENR; /* dummy read: wait until the clock is on (errata ES0392) */

   RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk; /* enable clock for USART1 */
   (void)RCC->APB2ENR;

   GPIOA->MODER &= ~GPIO_MODER_MODER10_Msk;
   GPIOA->MODER |= GPIO_MODER_MODER10_1; /* PA10: MODER = 10, alternate function */

   GPIOA->MODER &= ~GPIO_MODER_MODER9_Msk;
   GPIOA->MODER |= GPIO_MODER_MODER9_1; /* PA9: MODER = 10, alternate function */

   GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL9_Msk;
   GPIOA->AFR[1] |= ( 7UL << GPIO_AFRH_AFSEL9_Pos ); /* PA9 = AF7, USART1_TX */

   GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL10_Msk;
   GPIOA->AFR[1] |= ( 7UL << GPIO_AFRH_AFSEL10_Pos ); /* PA10 = AF7, USART1_RX */

   USART1->BRR = 100000000UL / 115200UL; /* 868: 115200 baud from 100 MHz APB2, written while UE = 0 */

   USART1->CR1 |= USART_CR1_UE_Msk; /* enable USART1 */

   USART1->CR1 |= USART_CR1_TE_Msk; /* enable the transmitter */

}


/* Send one byte: wait until TDR is empty, then write it */

void usart1_transmit(uint8_t b)

{

   while ( ( USART1->ISR & USART_ISR_TXE_TXFNF ) == 0 )
   {
   }

   USART1->TDR = b; /* plain write: TDR is a data register, not a set of flags */

}
