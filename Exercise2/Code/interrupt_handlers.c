#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "efm32gg.h"
#include "game_over.h"
#include "swtheme.h"


#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))




static uint32_t sample;

void gpio_interrupt () {
	
	uint32_t input_status;

	*GPIO_IFC = 0xff;
	input_status = *GPIO_PC_DIN;
	*GPIO_PA_DOUT = (input_status << 8) + 0xff;

	// input_status = ~input_status;
	// for (int i = 0; i < 8; i++) {
	// 	if (CHECK_BIT(input_status, i)) {
	// 		period_offset = i*8;
	// 	}
	// }
	setupDAC();
	setupTimer();
	sample = 0;
	*LETIMER0_CMD = 0x5;
	 
}
/* LETIMER0 interrupt handler */
void __attribute__ ((interrupt)) LETIMER0_IRQHandler() 
{  
	*LETIMER0_IFC = 0x1f;
	unsigned char sound = swtheme[sample];
	sample++;
	if (sound == 0) {
		disableTimer();
		disableDAC();
	}
	else {
		*DAC0_CH0DATA = sound;
  		*DAC0_CH1DATA = sound;
  	}
}


/* GPIO even pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler() 
{
    gpio_interrupt();
}

/* GPIO odd pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler() 
{
	gpio_interrupt();
}

