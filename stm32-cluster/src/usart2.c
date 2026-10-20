/*
    Name: usart2.c
    Purpose: UART link to the Raspberry Pi (SLCAN frames)
    Author: Nord
*/

#include "stm32h7xx.h"
#include "usart2.h"
#include <stdint.h>


/* --- USART2 --- */
/*
   PD5 - USART2_TX (AF7)
   PD6 - USART2_RX (AF7)
   Kernel clock: APB1 (rcc_pclk1) = 100 MHz: USART234578SEL = pclk1, USART_PRESC = /1 (reset values)
*/
void usart2_init(void)

{

   RCC->AHB4ENR |= RCC_AHB4ENR_GPIODEN_Msk; /* enable clock for GPIOD */
   (void)RCC->AHB4ENR; /* dummy read: wait until the clock is on (errata ES0392) */

   RCC->APB1LENR |= RCC_APB1LENR_USART2EN_Msk; /* enable clock for USART2 */
   (void)RCC->APB1LENR;

   GPIOD->MODER &= ~GPIO_MODER_MODER6_Msk;
   GPIOD->MODER |= GPIO_MODER_MODER6_1; /* PD6: MODER = 10, alternate function */

   GPIOD->MODER &= ~GPIO_MODER_MODER5_Msk;
   GPIOD->MODER |= GPIO_MODER_MODER5_1; /* PD5: MODER = 10, alternate function */

   GPIOD->AFR[0] &= ~GPIO_AFRL_AFSEL5_Msk;
   GPIOD->AFR[0] |= ( 7UL << GPIO_AFRL_AFSEL5_Pos ); /* PD5 = AF7, USART2_TX */

   GPIOD->AFR[0] &= ~GPIO_AFRL_AFSEL6_Msk;
   GPIOD->AFR[0] |= ( 7UL << GPIO_AFRL_AFSEL6_Pos ); /* PD6 = AF7, USART2_RX */

   USART2->BRR = 100000000UL / 115200UL; /* 868: 115200 baud from 100 MHz APB1, written while UE = 0 */

   USART2->CR1 |= USART_CR1_UE_Msk; /* enable USART2 */

   USART2->CR1 |= USART_CR1_TE_Msk; /* enable the transmitter */

}


void  usart2_transmit_buffer( const char *buffer, const uint32_t length )

{

   for ( uint32_t i = 0; i < length; i++ )

   {

      /* wait until the transmit data register is empty (TXE = 1) */
      while ( ( USART2->ISR & USART_ISR_TXE_TXFNF_Msk ) == 0 )
      {
      }

      USART2->TDR = buffer[i];

   }

}