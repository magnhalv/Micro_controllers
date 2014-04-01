#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

/* function to setup the timer */
void setupTimer(uint32_t div_clock)
{
    *CMU_HFCORECLKEN0 |= CMU2_HFCORECLKEN0_LE; /* Enable LE clk */
    *CMU_LFACLKEN0 |= CMU2_LFACLKEN0_LETIMER0; /* Enable LETIMER0 clk */
    
    /* 32 kHz clk */
    *CMU_OSCENCMD |= CMU2_OSCENCMD_LFRCOEN_E;
    *CMU_LFCLKSEL |= CMU2_LFCLKSEL_LFA_LFRCO; /* Set LFRCO as source clk for LFACLK*/

    /* Divides the 32 kHz clock by 2^div_clock */
    *CMU_LFAPRESC0 = div_clock << 8; 
   
    *LETIMER0_CTRL |= LETIMER0_CTRL_COMP0_TV; /* Set COMP0 as top value */
    *LETIMER0_COMP0 = 1; /* Set top value */

    /* Enable interrupt on underflow */
    *LETIMER0_IEN |= LETIMER0_IEN_UF;

    /* DMA attempt */
    // *LETIMER0_CTRL |= LETIMER0_CTRL_TOGGLE_O0;
    // *LETIMER0_CTRL |= 1 << 5;
    // *LETIMER0_ROUTE |= 3;



   

}

void disableTimer() {
    *LETIMER0_CMD = 1 << 1; //Stop timer. 
    *CMU_OSCENCMD &= ~CMU2_OSCENCMD_LFRCOEN_E; //Disable LFRCO when sound is done playing. 
    *LETIMER0_IEN = 0;
    *CMU_HFCORECLKEN0 &= ~CMU2_HFCORECLKEN0_LE;
    *CMU_LFACLKEN0 &= ~CMU2_LFACLKEN0_LETIMER0;
}


