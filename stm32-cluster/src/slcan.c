/*
    Name: slcan.c
    Purpose: build slcan frames
    Author: Nord
*/

#include "slcan.h"

static const char HEX_DIGITS[16] = { '0', '1', '2', '3',
                                     '4', '5', '6', '7',
                                     '8', '9', 'A', 'B', 
                                     'C', 'D', 'E', 'F' };


/* to convert one byte into two hex characters */
static void byte_to_hex( uint8_t value, char *out )

{

    out[0] = HEX_DIGITS[    ( value >> 4 )   & 0x0F ];
    out[1] = HEX_DIGITS[      value          & 0x0F ];

}


/* build a slcan frame */
uint32_t slcan_build_frame(char *out, uint16_t id, const uint8_t *data, uint8_t dlc)
{

    uint32_t pos = 0; /* position in slcan frame buffer */

    /*  DLC cannot be more than 8 bytes */
    if ( dlc > 8 )
    
    {

        return 0;

    }


    /* --- Frame type --- */
    out[pos] = 't';
    pos ++;


    /* --- Frame ID --- */
    out[pos] = HEX_DIGITS[ ( id >> 8 ) & 0x0F ];
    pos ++;
    out[pos] = HEX_DIGITS[ ( id >> 4 ) & 0x0F ];
    pos ++;
    out[pos] = HEX_DIGITS[ ( id      ) & 0x0F ];
    pos ++;


    /* --- Frame DLC --- */
    out[pos] = HEX_DIGITS[ dlc ];
    pos ++;


    /* --- Frame DATA --- */
    for ( uint8_t i = 0 ; i < dlc; i++ )

    {

        byte_to_hex( data[i], &out[pos] );
        pos += 2;

    }


    /* --- Frame end --- */
    out[pos] = '\r';
    pos ++;

    return pos; /* frame length */

}
