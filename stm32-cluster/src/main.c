/*
    Name:    main.c
    Purpose: main loop of the STM32H743 instrument cluster
    Author:  Nord

    Current state: sends a test speed frame (SLCAN, ID 0x1A0) to the Pi
    over USART2 every 500 ms, toggling 120 <-> 60 km/h.
    USART1 is the debug console.
*/

#include "stm32h7xx.h"
#include "bsp.h"
#include "usart1.h"
#include "usart2.h"
#include "slcan.h"
#include <stdint.h>


/* Speed frame layout: uint16, 0.01 km/h per bit, Motorola byte order */
#define SPEED_FRAME_ID 0x1A0u
#define SPEED_FRAME_FACTOR 100u
#define SPEED_FRAME_DATA_LEN 2

void data_pack( uint8_t *raw_data_out, uint32_t value, uint8_t factor );


int main(void)

{

   char speed_frame[ SLCAN_FRAME_MAX_LENGTH ]; /* SLCAN text frame */
   uint8_t speed_data[ SPEED_FRAME_DATA_LEN ]; /* raw CAN data bytes */

   bsp_init(); /* power, HSE, PLL1 400 MHz, buses, SysTick */

   usart1_init(); /* debug console */

   usart2_init(); /* link to the Pi */

   while(1)

   {

      data_pack( speed_data, 120, SPEED_FRAME_FACTOR ); /* 120 km/h */

      /* build and transmit speed frame */
      uint32_t frame_length = slcan_build_frame( speed_frame, SPEED_FRAME_ID, speed_data, SPEED_FRAME_DATA_LEN );
      usart2_transmit_buffer( speed_frame, frame_length );

      delay_ms(500);

      data_pack( speed_data, 60, SPEED_FRAME_FACTOR ); /* 60 km/h */

      frame_length = slcan_build_frame( speed_frame, SPEED_FRAME_ID, speed_data, SPEED_FRAME_DATA_LEN );
      usart2_transmit_buffer( speed_frame, frame_length );

      delay_ms(500);

   }

}


/* scale a value and pack it into 2 bytes, Motorola order */
void data_pack( uint8_t *raw_data_out, uint32_t value, uint8_t factor )

{

   uint32_t value_raw = value * factor; /* physical -> raw */

   /* Motorola: high byte first */
   raw_data_out[0] = ( value_raw >> 8 ) & 0xFF;
   raw_data_out[1] = ( value_raw      ) & 0xFF;

}