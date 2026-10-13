/* 
    Name: bsp.c
    Purpose: Initial configuration of the STM32H743:
                1. Power supply configuration
                2. Enabling the HSE crystal (25 MHz)
                3. Configuring PLL dividers and multipliers
    Author: Nord
*/

#include "bsp.h"
#include "stm32h7xx.h"


static void power_init(void); /* prototypes: lets bsp_init sit at the top */
static void hse_init(void); 
static void pll_init(void);



/* Board bring-up: runs every init step in order */
void bsp_init(void)

{

    power_init();
    hse_init();
    pll_init();

}



/* Power supply configuration */
static void power_init(void)

{

    /* create a new var, cause PWR_CR3 reg can be changed only once */
    uint32_t cr3 = PWR->CR3;

    cr3 &= ~PWR_CR3_BYPASS_Msk; /* Set 0 to BYPASS bit */
    cr3 |= PWR_CR3_LDOEN_Msk; /* Set 1 to LDOEN bit */

    PWR->CR3 = cr3; /* Apply changes in one action */

   
    /* wait until the supply reports the voltage is ready */
    while ( ( PWR->CSR1 & PWR_CSR1_ACTVOSRDY ) == 0 )
    {
    }    


    PWR->D3CR |= PWR_D3CR_VOS_Msk; /* set 11 to VOS(VOS1), VOS1 to support 400 MHz */


    /* wait until the supply reports the voltage mode is ready */
    while ( ( PWR->D3CR & PWR_D3CR_VOSRDY ) == 0 )
    {
    }    

}



/* Enable HSE */
static void hse_init(void)

{

    RCC->CR |= RCC_CR_HSEON_Msk; /* enable HSE */

    while ( ( RCC->CR & RCC_CR_HSERDY ) == 0 )
    {
    }

}


/* Enable and configure PLL1 */
static void pll_init(void)

{

    /* RCC-> PLLCKSELR */

    RCC->PLLCKSELR &= ~RCC_PLLCKSELR_PLLSRC_Msk; /* Clear field */
    
    RCC->PLLCKSELR |= RCC_PLLCKSELR_PLLSRC_HSE; /* switch pll src from hsi to hse */

    RCC->PLLCKSELR &= ~RCC_PLLCKSELR_DIVM1_Msk;
    
    RCC->PLLCKSELR |= ( 5UL << RCC_PLLCKSELR_DIVM1_Pos ); /* Set M div */

    /* RCC->PLLCFGR */

    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1RGE_Msk;

    RCC->PLLCFGR |= RCC_PLLCFGR_PLL1RGE_2; /* set the range of pll enter val( 4...8 MHz ) */ 
    
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1VCOSEL_Msk; /* Set VCO range( 192 to 960 MHz ) */

    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLL1FRACEN_Msk; /* Integer N only: fractional multiplier off */

    RCC->PLLCFGR |= RCC_PLLCFGR_DIVP1EN_Msk; /* Enable P chanel */

    /* RCC->PLLDIVR */

    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_N1_Msk;

    RCC->PLL1DIVR |= ( 159UL << RCC_PLL1DIVR_N1_Pos ); /* Set N div */

    RCC->PLL1DIVR &= ~RCC_PLL1DIVR_P1_Msk;

    RCC->PLL1DIVR |= ( 1UL << RCC_PLL1DIVR_P1_Pos ); /* Set P div */

    RCC->CR |= RCC_CR_PLL1ON; /* enable pll1 */

    while ( ( RCC->CR & RCC_CR_PLL1RDY ) == 0 )
    {
    }

}