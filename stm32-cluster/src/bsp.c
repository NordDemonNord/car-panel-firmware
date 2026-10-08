/* 
    Name: bsp.c
    Purpose: Initial configuration of the STM32H743:
                1. Power supply configuration
                2. Switching the clock source from HSI to HSE
                3. Configuring PLL dividers and multipliers
    Author: Nord
*/

#include "bsp.h"
#include "stm32h7xx.h"


static void power_init(void); /* prototypes: lets bsp_init sit at the top */



/* Board bring-up: runs every init step in order */
void bsp_init(void)

{

    power_init();

}



/* Power supply configuration */
static void power_init(void)

{

    /* create a new var, cause PWR_CR3 reg can be changed only once */
    uint32_t cr3 = PWR->CR3;

    cr3 &= ~PWR_CR3_BYPASS; /* Set 0 to BYPASS bit */
    cr3 |= PWR_CR3_LDOEN; /* Set 1 to LDOEN bit */

    PWR->CR3 = cr3; /* Apply changes in one action */

    /* wait until the supply reports the voltage is ready */
    while ( ( PWR->CSR1 & PWR_CSR1_ACTVOSRDY ) == 0 )
    {
    }    

}