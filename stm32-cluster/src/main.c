/*
    Name:    main.c
    Purpose: main loop of the STM32H743 instrument cluster
    Author:  Nord

    Current state: sends speed data in slcan frame over usart1.
*/

#include "stm32h7xx.h"
#include "bsp.h"
#include "usart1.h"
#include "slcan.h"
#include <stdint.h>


#define SPEED_FRAME_ID 0x1A0u
#define SPEED_FRAME_FACTOR 100u
#define SPEED_FRAME_DATA_LEN 2

void data_pack( uint8_t *raw_data_out, uint32_t value, uint8_t factor );


int main(void)

{

   char speed_frame[ SLCAN_FRAME_MAX_LENGTH ]; /* empty array for ID, DLC and DATA fields */
   uint8_t speed_data[ SPEED_FRAME_DATA_LEN ]; /* array of only DATA field */

   bsp_init(); /* power, HSE, PLL1 400 MHz, buses, SysTick */

   usart1_init(); /* init usart1 for transmitting data to Pi */

   while(1)

   {

      data_pack( speed_data, 120, SPEED_FRAME_FACTOR ); /* speed frame, speed = 120 km/h */

      /* build and transmit speed frame */
      uint32_t frame_length = slcan_build_frame( speed_frame, SPEED_FRAME_ID, speed_data, SPEED_FRAME_DATA_LEN );
      usart1_transmit_buffer( speed_frame, frame_length );

      delay_ms(500);

      data_pack( speed_data, 60, SPEED_FRAME_FACTOR ); /* speed frame, speed = 60 km/h */

      frame_length = slcan_build_frame( speed_frame, SPEED_FRAME_ID, speed_data, SPEED_FRAME_DATA_LEN );
      usart1_transmit_buffer( speed_frame, frame_length );

      delay_ms(500);

   }

}


void data_pack( uint8_t *raw_data_out, uint32_t value, uint8_t factor )

{

   uint32_t value_raw = value * factor; /* apply a factor */

   /* building raw data for speed frame */
   raw_data_out[0] = ( value_raw >> 8 ) & 0xFF;
   raw_data_out[1] = ( value_raw      ) & 0xFF;

}