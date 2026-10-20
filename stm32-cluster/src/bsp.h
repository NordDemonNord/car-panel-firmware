#ifndef BSP_H
#define BSP_H

/* Board crystal frequency, Hz */
#define BSP_HSE_HZ  25000000UL

#include <stdint.h>

/* Bring the board up: clocks, then pins. Call first in main(). */
void bsp_init(void);

/* Busy-wait for ms milliseconds using SysTick */
void delay_ms(uint32_t ms);

#endif /* BSP_H */