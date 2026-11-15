#ifndef CAN_H
#define CAN_H

#include <stdint.h>

void twai_init( void );
void twai_transmit( uint32_t id, uint8_t *data, uint8_t len );

#endif