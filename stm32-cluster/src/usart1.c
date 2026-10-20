/*
    Name: usart1.c
    Purpose: init usart1 and transmit slcan frames to Pi through it
    Author: Nord
*/

#include "stm32h7xx.h"
#include "usart1.h"
#include <stdint.h>


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


void  usart1_transmit_buffer( const char *buffer, const uint32_t length )

{

   for ( uint32_t i = 0; i < length; i++ )

   {

      /* wait until the transmit data register is empty (TXE = 1) */
      while ( ( USART1->ISR & USART_ISR_TXE_TXFNF_Msk ) == 0 )
      {
      }

      USART1->TDR = buffer[i];

   }

}