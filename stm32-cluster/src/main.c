/*
    Name:    main.c
    Purpose: main loop of the STM32H743 instrument cluster
    Author:  Nord

    Current state: get CAN frames from PC( CANable )  --- FDCAN2 ---
      and send it to Pi over UART.                    --- USART2 ---
      --- Only speed frames to test the link ---
*/

#include "stm32h7xx.h"
#include "bsp.h"
#include "usart1.h"
#include "usart2.h"
#include "slcan.h"
#include "fdcan2.h"
#include <stdint.h>

/* CAN frames IDs */
#define SPEED_FRAME_ID 0x1A0u

/* CAN frame blocks, recevied from CANable */
uint32_t ID_frame     = 0;
uint32_t DLC_frame    = 0;
uint8_t  can_status   = 0; /* are there frames in Rx buffer */ 
uint32_t frame_length = 0;
uint8_t  DATA_frame[CAN_FRAME_DATA_LENGTH_MAX];


int main(void)

{

   char     speed_frame[ SLCAN_FRAME_MAX_LENGTH ]; /* SLCAN text frame */

   bsp_init();       /* power, HSE, PLL1 400 MHz, buses, SysTick */

   usart1_init();    /* debug console */

   usart2_init();    /* link to the Pi */

   fdcan2_init();    /* link to the PC */

   while(1)

   {

      can_status = fdcan2_receive(&ID_frame, &DLC_frame, DATA_frame); /* get ID, DLC and data from CAN frame */

      if ( can_status )

      {

         if ( ID_frame == SPEED_FRAME_ID )

         {

            frame_length = slcan_build_frame( speed_frame, ID_frame, DATA_frame, DLC_frame );

            usart2_transmit_buffer( speed_frame, frame_length );

         } 

      }

   }

}