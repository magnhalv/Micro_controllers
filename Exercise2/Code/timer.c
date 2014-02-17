#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

/* function to setup the timer */
void setupTimer(uint16_t period)
{
    *CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_LETIMER0;
    *CMU_HFCORELKEN0 |= CMU_HFCORECLKEN0_LE;
    *CMU_LFACLKEN0 |= CMU_LFACLKEN0_LETIMER0;
    *CMU_LFCLKSEL |= CMU_LFCLKSEL_LFAE;

    *LETIMER0_CTRL |= LETIMER0_CTRL_COMP0_TV; /* Set COMP0 as top value */
    *LETIMER0_CTRL |= LETIMER0_CTRL_BUFMOD; /* Enable buffered mode */
    *LETIMER0_COMP0 = 0x00ff;

}


