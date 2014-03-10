#include <stdint.h>
#include <stdbool.h>

#include <math.h>

#include "efm32gg.h"


void setupDAC() {

	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0; /* Enable DAC clk */

	*DAC0_CTRL = 0x50010;

	*DAC0_CH0CTRL |= DAC_ENABLE_CH;
	*DAC0_CH1CTRL |= DAC_ENABLE_CH;

	/* Trigger on PRS channel 0 */
	//*DAC0_CH0CTRL |= DAC_PRS_TRIGGER;
	//*DAC0_CH1CTRL |= DAC_PRS_TRIGGER;





  	/*
	    TODO enable and set up the Digital-Analog Converter
	    
	    1. Enable the DAC clock by setting bit 17 in CMU_HFPERCLKEN0
	    2. Prescale DAC clock by writing 0x50010 to DAC0_CTRL
	    3. Enable left and right audio channels by writing 1 to DAC0_CH0CTRL and DAC0_CH1CTRL
	    4. Write a continuous stream of samples to the DAC data registers, DAC0_CH0DATA and DAC0_CH1DATA, for example from a timer interrupt
  	*/
}

void disableDAC() {
	*DAC0_CTRL = 0;

	*DAC0_CH0CTRL = 0;
	*DAC0_CH1CTRL = 0;
	*CMU_HFPERCLKEN0 &= ~CMU2_HFPERCLKEN0_DAC0;
}
