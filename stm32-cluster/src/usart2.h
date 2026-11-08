#ifndef USART2_H
#define USART2_H

#include <stdint.h>

void    usart2_transmit_buffer( const char *buffer, const uint32_t length ); 
void    usart2_init( void );

#endif