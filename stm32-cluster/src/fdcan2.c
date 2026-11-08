/*
    Name: fdcan2.c
    Purpose: recive fdcan frames from CANable( PC -> CANable -> STM32 )
    Author: Nord
*/

/*
    Parts:
    CANable V2.0
    Transceiver(HW-021)

    Pinout:
    RX - PB12
    TX - PB13

    Voltage - 5V
*/

#include "stm32h7xx.h"
#include "usart1.h"
#include <stdint.h>


static const uint16_t F0SA = 0U; /* memory shift in RAM */


void fdcan2_init(void)

{

            /* --- Clock unit --- */

    /* FDCAN kernel clock: HSE (25 MHz). FDCANSEL = 00 is the reset value,
    written explicitly so the choice is visible in code. */
    RCC->D2CCIP1R &= ~RCC_D2CCIP1R_FDCANSEL_Msk;

    /* FDCAN bus clock (FDCAN1 and FDCAN2 share one enable bit). */
    RCC->APB1HENR |= RCC_APB1HENR_FDCANEN;

    /* GPIOB clock. */
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN;

    /* PB12 = FDCAN2_RX, PB13 = FDCAN2_TX: alternate function mode (10). */
    GPIOB->MODER &= ~(GPIO_MODER_MODE12_Msk | GPIO_MODER_MODE13_Msk);
    GPIOB->MODER |=  (2U << GPIO_MODER_MODE12_Pos) | (2U << GPIO_MODER_MODE13_Pos);

    /* AF9 = FDCAN2 on PB12/PB13. Pins 8..15 live in AFR[1] (AFRH). */
    GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL12_Msk | GPIO_AFRH_AFSEL13_Msk);
    GPIOB->AFR[1] |=  (9U << GPIO_AFRH_AFSEL12_Pos) | (9U << GPIO_AFRH_AFSEL13_Pos);

            /* --- Configuration unit --- */

    /*  Nominal bit timing, 500 kbit/s from 25 MHz HSE:
        speed (baud) = 1 / bit time
        bit time (ticks) = ( NBRP + 1 ) * ( 1 + ( NTSEG1 + 1 ) + ( NTSEG2 + 1 ) )
                                                    ( BS1 )         ( BS2 )
        tq = 2 ticks, bit = 1 + 21 + 3 = 25 tq = 50 ticks, sample point = 22/25 = 88 %
    */

    /* Only when FDCAN_CCCR_INIT is set fdcan can be configured */
    FDCAN2->CCCR |= FDCAN_CCCR_INIT_Msk;

    /* Wait until INIT is set (crosses into the FDCAN kernel clock domain) */
    while ( ( FDCAN2->CCCR & FDCAN_CCCR_INIT_Msk ) == 0 )
    {
    }

    /* FDCAN_CCCR_CCE also should be set to be able to configure, but after FDCAN_CCCR_INIT */
    FDCAN2->CCCR |= FDCAN_CCCR_CCE_Msk;

    /* NTSEG1 = 20 -> BS1 = 21 tq */
    FDCAN2->NBTP &= ~FDCAN_NBTP_NTSEG1_Msk;
    FDCAN2->NBTP |= ( 20U << FDCAN_NBTP_NTSEG1_Pos );

    /* NTSEG2 = 2 -> BS2 = 3 tq */
    FDCAN2->NBTP &= ~FDCAN_NBTP_NTSEG2_Msk;
    FDCAN2->NBTP |= ( 2U << FDCAN_NBTP_NTSEG2_Pos );

    /* NBRP = 1 -> prescaler 2, tq = 2 ticks */
    FDCAN2->NBTP &= ~FDCAN_NBTP_NBRP_Msk;
    FDCAN2->NBTP |= ( 1U << FDCAN_NBTP_NBRP_Pos );

    /* NSJW = 2 -> SJW = 3 tq (= BS2, max allowed) */
    FDCAN2->NBTP &= ~FDCAN_NBTP_NSJW_Msk;
    FDCAN2->NBTP |= ( 2U << FDCAN_NBTP_NSJW_Pos );

            /* RAM message unit */

    /* Number of FIFO 0 elements */
    FDCAN2->RXF0C &= ~FDCAN_RXF0C_F0S_Msk;
    FDCAN2->RXF0C |= ( 20U << FDCAN_RXF0C_F0S_Pos );

    /* Start address in RAM */
    FDCAN2->RXF0C &= ~FDCAN_RXF0C_F0SA_Msk;
    FDCAN2->RXF0C |= ( F0SA << FDCAN_RXF0C_F0SA_Pos );

    /* GFC, SIDFC, RXESC left at reset: no filters, all frames -> FIFO0, 8-byte data */

    /* end configuration, CCE resets automaticaly */
    FDCAN2->CCCR &= ~FDCAN_CCCR_INIT_Msk;

    /* Wait until INIT is reset */
    while ( ( FDCAN2->CCCR & FDCAN_CCCR_INIT_Msk ) != 0 )
    {
    }

}


int fdcan2_receive(uint32_t *ID, uint32_t *DLC, uint8_t *DATA)

{

    uint32_t fifo_index;            /* number: which cell, 0..19 */
    volatile uint32_t *cell;        /* pointer: where that cell lives in message RAM */

    /* Buffer population check */
    if ( ( FDCAN2->RXF0S & FDCAN_RXF0S_F0FL_Msk ) != 0 )

    {
        
        /* get index in FIF0 buffer, read only */
        fifo_index = ( ( FDCAN2->RXF0S & FDCAN_RXF0S_F0GI_Msk ) >> FDCAN_RXF0S_F0GI_Pos );
        
        /* frame address in RAM */
        cell = ( volatile uint32_t * )( SRAMCAN_BASE + F0SA * 4U + fifo_index * 16U );

        /* --- ID --- */
        uint32_t R0 = cell[0];
        *ID = ( ( R0 >> 18 ) & 0x7FF );

        /* --- DLC --- */
        uint32_t R1 = cell[1];
        *DLC = ( ( R1 >> 16 ) & 0xF );

        /* --- DATA --- */
        uint32_t R2 = cell[2];
        uint32_t R3 = cell[3];

        /* first 4 bytes */
        DATA[0] =      R2              & 0xFF;
        DATA[1] =    ( R2 >> 8  )      & 0xFF;
        DATA[2] =    ( R2 >> 16 )      & 0xFF;
        DATA[3] =    ( R2 >> 24 )      & 0xFF;

        /* 5, 6, 7 an 8 bytes */
        DATA[4] =      R3              & 0xFF;
        DATA[5] =    ( R3 >> 8  )      & 0xFF;
        DATA[6] =    ( R3 >> 16 )      & 0xFF;
        DATA[7] =    ( R3 >> 24 )      & 0xFF;

        FDCAN2->RXF0A = ( fifo_index << FDCAN_RXF0A_F0AI_Pos ); /* shift the index in FIF0 buffer */

        return 1;

    } else 

    {

        return 0; /* no CAN frames in buffer */

    }

}
