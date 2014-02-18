#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

/* function to setup the timer */
void setupTimer(uint16_t period)
{
    *CMU_HFCORECLKEN0 |= CMU2_HFCORECLKEN0_LE; /* Enable LE clk */
    *CMU_LFACLKEN0 |= CMU2_LFACLKEN0_LETIMER0; /* Enable LETIMER0 clk */
    uint32_t temp = *CMU_LFCLKSEL;
    temp &= ~3; 
    *CMU_LFCLKSEL = CMU2_LFCLKSEL_LFAE | temp; /* Set ULFRCO as source clk for LETIMER0 */

    *LETIMER0_CTRL |= LETIMER0_CTRL_COMP0_TV; /* Set COMP0 as top value */
    *LETIMER0_CTRL |= LETIMER0_CTRL_ONESHOT; /* Enable ONESHOT mode */
    
    /* Set default values */
    *LETIMER0_COMP0 = 0x0001;
    *LETIMER0_REP0 = 0x8;

    /* Enable interrupt */
    *LETIMER0_IEN |= LETIMER0_IEN_UF;
   

}


