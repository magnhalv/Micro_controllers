#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void setupDMA() {
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_PRS;
	*CMU_HFCORECLKEN0 |= CMU2_HFCORECLKEN0_DMA;	
}