#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

typedef struct {
	uint32_t control;
	uint32_t *dest_pointer;
	const char *src_pointer;
} DMA_descriptor;

static DMA_descriptor descriptor;

void setupDMA(const char* source_addr) {
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_PRS;
	*CMU_HFCORECLKEN0 |= CMU2_HFCORECLKEN0_DMA;	

	/* LETIMER0_CH0 set as default trigger on PRS CH0. */
	*PRS_CH0_CTRL |= PRS_CTRL_LETIMER0; 
	//*PRS_CH0_CTRL |= 1 << 28;
 	/*
	*DMA_CH0_CTRL |= DMA_CHn_CTRL_DAC0;

	descriptor.control = 0x00000000
	descriptor.dest_pointer = DAC0_CH0DATA;
	descriptor.src_pointer = source_addr;
	*DMA_CTRLBASE = &descriptor;
	*/

}