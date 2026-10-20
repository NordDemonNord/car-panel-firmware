#ifndef USART1_H
#define USART1_H

#include <stdint.h>

void  usart1_transmit_buffer( const char *buffer, const uint32_t length ); 
void usart1_init( void );

#endif