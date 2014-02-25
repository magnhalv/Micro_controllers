#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

/* function to setup the timer */
void setupTimer(uint16_t period)
{
    *CMU_HFCORECLKEN0 |= CMU2_HFCORECLKEN0_LE; /* Enable LE clk */
    *CMU_LFACLKEN0 |= CMU2_LFACLKEN0_LETIMER0; /* Enable LETIMER0 clk */
    //uint32_t temp = *CMU_LFCLKSEL;
    //temp &= ~3; 
    //*CMU_LFCLKSEL = CMU2_LFCLKSEL_ENABLE_ULFRCO | temp; /* Set ULFRCO as source clk for LETIMER0 */
    /* 14 MHz clk */
    *CMU_LFCLKSEL |= CMU2_LFCLKSEL_LFA_HFCORECLK; /* Set HFCORECLK/2 as source clk for LFACLK*/
    
    /* 213 Hz */
    *CMU_LFAPRESC0 = 0x8 << 8;

    *LETIMER0_CTRL |= LETIMER0_CTRL_COMP0_TV; /* Set COMP0 as top value */
    *LETIMER0_CTRL |= LETIMER0_CTRL_ONESHOT; /* Enable ONESHOT mode */
    
    /* Set default values */
    *LETIMER0_COMP0 = 1;
    //*LETIMER0_REP0 = 0x8;

    /* Enable interrupt on underflow */
    *LETIMER0_IEN |= LETIMER0_IEN_UF;
   

}


