/*

    Name:           main.c

    Purpose:        transmit twai frames to STM32H7

    Author:         Nord

    Current state:  debugging without accses to hadrware, 
                    should transmit speed frame

    TODO:           change example TX and RX pins to right ones

*/

#include <stdint.h>
#include "can_tx.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/* --- TWAI FRAMES --- */

/* Speed */
#define TWAI_SPEED_ID 0x1A0u    /* ID */
#define TWAI_SPEED_FACTOR 100u  /* FACTOR */
uint8_t speed_buff[2];          /* DATA */


void app_main( void )

{

    twai_init();

    while ( 1 )

    {

        for ( uint8_t speed = 10; speed < 220; speed += 10 )

        {

            uint32_t speed_data = speed * TWAI_SPEED_FACTOR;

            speed_buff[0] = ( speed_data >> 8 ) & 0xFF;

            speed_buff[1] = ( speed_data )      & 0xFF;

            twai_transmit (TWAI_SPEED_ID, speed_buff, 2 );

            vTaskDelay(pdMS_TO_TICKS(100));

        }

        for ( uint8_t speed = 220; speed > 0; speed -= 10 )

        {

            uint32_t speed_data = speed * TWAI_SPEED_FACTOR;

            speed_buff[0] = ( speed_data >> 8 ) & 0xFF;

            speed_buff[1] = ( speed_data )      & 0xFF;

            twai_transmit(TWAI_SPEED_ID, speed_buff, 2 );

            vTaskDelay(pdMS_TO_TICKS(100));

        }

    }

}