/* 

    Name:       can_tx.c

    Purpose:    two functions:
                1. Init and configure TWAI
                2. transmit data to STM32H7
    
    Author:     Nord

*/


#include "can_tx.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include <stdint.h>

twai_node_handle_t node_hdl = NULL;


/* --- Init TWAI --- */
void twai_init( void )

{

    twai_onchip_node_config_t node_config = 

    {

        .io_cfg.tx          = 21,           /* TX pin */ 
        .io_cfg.rx          = 22,           /* RX pin */
        .bit_timing.bitrate = 500000,       /* bitrate 500 kbt per sec */
        .tx_queue_depth     = 5,            /* transmit queue depth */

    };

    // Create a new TWAI controller driver instance
    ESP_ERROR_CHECK(twai_new_node_onchip(&node_config, &node_hdl));
    
    // Start the TWAI controller
    ESP_ERROR_CHECK(twai_node_enable(node_hdl));

}



/* --- Transmit TWAI frame --- */
void twai_transmit( uint32_t id, uint8_t *data, uint8_t len )

{

    /* Build frame */
    twai_frame_t tx_msg = 

    {

        .header.id  = id,               /* ID */
        .header.ide = false,            /* always not extended in this project */
        .buffer     = data,             /* DATA */
        .buffer_len = len,              /* DLC */

    };

    ESP_ERROR_CHECK(twai_node_transmit(node_hdl, &tx_msg, 0));  // Timeout = 0: returns immediately if queue is full
    
    ESP_ERROR_CHECK(twai_node_transmit_wait_all_done(node_hdl, -1));  // Wait for transmission to finish

}