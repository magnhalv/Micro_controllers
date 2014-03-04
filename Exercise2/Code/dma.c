#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void setupDMA() {
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_PRS;
	*CMU_HFCORECLKEN0 |= CMU2_HFCORECLKEN0_DMA;	

	/* Setup PRS */
	*PRS_CH0_CTRL |= PRS_CTRL_LETIMER0; //LETIMER0_CH0 set as default trigger.

	/* DAC PRS */
	*DAC0_CH0CTRL |= DAC_PRS_TRIGGER;
	*DAC0_CH1CTRL |= DAC_PRS_TRIGGER;
}