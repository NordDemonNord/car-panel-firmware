#ifndef BSP_H
#define BSP_H

/* Board crystal frequency, Hz */
#define BSP_HSE_HZ  25000000UL

/* Bring the board up: clocks, then pins. Call first in main(). */
void bsp_init(void);

#endif /* BSP_H */