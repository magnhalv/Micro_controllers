#include <stdint.h>
#include <stdbool.h>

#include <math.h>

#include "efm32gg.h"


void setupDAC() {

	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0; /* Enable DAC clk gate */

	*DAC0_CTRL = 0x50010;

	*DAC0_CH0CTRL |= DAC_ENABLE_CH;
	*DAC0_CH1CTRL |= DAC_ENABLE_CH;

	/* Trigger on PRS channel 0 */
	//*DAC0_CH0CTRL |= DAC_PRS_TRIGGER;
	//*DAC0_CH1CTRL |= DAC_PRS_TRIGGER;

}

void disableDAC() {
	*DAC0_CTRL = 0;

	*DAC0_CH0CTRL = 0;
	*DAC0_CH1CTRL = 0;
	*CMU_HFPERCLKEN0 &= ~CMU2_HFPERCLKEN0_DAC0;
}
