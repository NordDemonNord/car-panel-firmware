/*
    Name: bsp.c
    Purpose: Initial configuration of the STM32H743:
                1. Power supply configuration (LDO, VOS1)
                2. Enabling the HSE crystal (25 MHz)
                3. Configuring PLL1 (HSE -> 400 MHz)
                4. Flash latency (wait states)
                5. Bus prescalers (AXI 200 MHz, APB 100 MHz)
                6. Switching SYSCLK to PLL1
                7. SysTick 1 ms delay (used to verify the 400 MHz clock)
    Author: Nord
*/

#include "bsp.h"
#include "stm32h7xx.h"

#define CORE_CLOCK_HZ   400000000UL                  /* 400 MHz after sysclk_switch */
#define TICKS_PER_MS    ( CORE_CLOCK_HZ / 1000UL )   /* 400 000 ticks in 1 ms */



static void power_init(void); /* prototypes: lets bsp_init sit at the top */
static void hse_init(void); 
static void pll_init(void);
static void flash_init(void);
static void buses_init(void);
static void sysclk_switch(void);
static void systick_init(void);


/* Board bring-up: runs every init step in order */
void bsp_init(void)

{

    power_init();
    hse_init();
    pll_init();
    flash_init();
    buses_init();
    sysclk_switch();
    systick_init();

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


/* Configure flash latency */
static void flash_init(void)

{

    /* latency waiting */
    FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS; /* set 2 wait states */

    while ( ( FLASH->ACR & FLASH_ACR_LATENCY_Msk ) != FLASH_ACR_LATENCY_2WS )
    {
    }

    /* writing latency */
    FLASH->ACR &= ~FLASH_ACR_WRHIGHFREQ_Msk;
    FLASH->ACR |= FLASH_ACR_WRHIGHFREQ_1; /* flash write signal delay for 185-210 MHz AXI (VOS1) */

}


/* configure buses dividers */
static void buses_init(void)

{

    /* --- D1 --- */
    RCC->D1CFGR &= ~RCC_D1CFGR_HPRE_Msk;
    RCC->D1CFGR |= RCC_D1CFGR_HPRE_DIV2; /* set 2 for AXI and AHB buses */
    
    RCC->D1CFGR &= ~RCC_D1CFGR_D1PPRE_Msk;
    RCC->D1CFGR |= RCC_D1CFGR_D1PPRE_DIV2; /* set 2 for APB3 bus */

    /* --- D2 --- */
    RCC->D2CFGR &= ~ RCC_D2CFGR_D2PPRE1_Msk;
    RCC->D2CFGR |= RCC_D2CFGR_D2PPRE1_DIV2; /* set 2 for APB1 */

    RCC->D2CFGR &= ~ RCC_D2CFGR_D2PPRE2_Msk;
    RCC->D2CFGR |= RCC_D2CFGR_D2PPRE2_DIV2; /* set 2 for APB2 */

    /* --- D3 --- */
    RCC->D3CFGR &= ~ RCC_D3CFGR_D3PPRE_Msk;
    RCC->D3CFGR |= RCC_D3CFGR_D3PPRE_DIV2; /* set 2 for APB4 */

}


/* switch system clock source to PLL1 */
static void sysclk_switch(void)

{

    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    RCC->CFGR |= RCC_CFGR_SW_PLL1; /* sysclk switch to PLL1 */

    while ( ( RCC->CFGR & RCC_CFGR_SWS_Msk ) != RCC_CFGR_SWS_PLL1 )
    {
    }

}


/* SysTick: core timer, fires COUNTFLAG every 1 ms */
static void systick_init(void)

{

    SysTick->CTRL = 0;                          /* stop the timer while configuring */

    SysTick->LOAD = TICKS_PER_MS - 1UL;         /* count 399 999 .. 0 = 400 000 ticks */

    SysTick->VAL = 0;                           /* reset the current counter value */

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk  /* clock from the core (400 MHz) */
                  | SysTick_CTRL_ENABLE_Msk;    /* start; no interrupt (TICKINT = 0) */

}


/* Busy-wait for ms milliseconds using SysTick */
void delay_ms(uint32_t ms)

{

    for ( uint32_t i = 0; i < ms; i++ )
    {
        /* COUNTFLAG goes to 1 when the counter hits 0, and clears itself on read */
        while ( ( SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk ) == 0 )
        {
        }
    }

}