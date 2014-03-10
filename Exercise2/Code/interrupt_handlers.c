#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "efm32gg.h"
#include "swtheme.h"
#include "game_over.h"
#include "battle003.h"


#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))




static uint32_t sample;
static uint32_t nofSamples;
static unsigned char *sound_array;

void gpio_interrupt () {
	
	uint32_t input_status;
	uint32_t div_clock;

	*GPIO_IFC = 0xff;
	input_status = *GPIO_PC_DIN;
	*GPIO_PA_DOUT = (input_status << 8) + 0xff;
	uint32_t button_pressed = 0;
	// input_status = ~input_status;
	for (int i = 0; i < 8; i++) {
	 	if (CHECK_BIT(~input_status, i)) {
	 		button_pressed = i;
	 	}
	}
	switch (button_pressed) {
		case 0: 
			sound_array = swtheme;
			nofSamples = sizeof(swtheme);
			div_clock = 1;
			break;
		case 1: 
			sound_array = battle003;
			nofSamples = sizeof(battle003);
			div_clock = 0;
			break;
		case 2: 
			sound_array = game_over;
			nofSamples = sizeof(game_over);
			div_clock = 1;
			break;
		default:
			return;
	}

	setupDAC(div_clock);
	setupTimer();
	sample = 0;
	*LETIMER0_CMD = 0x5; /* CLear and start clock */
	 
}
/* LETIMER0 interrupt handler */
void __attribute__ ((interrupt)) LETIMER0_IRQHandler() 
{  
	//*PRS_SWPULSE = 1;
	*LETIMER0_IFC = 0x1f;
	unsigned char sound = sound_array[sample];
	sample++;
	if (sample < nofSamples) {
		*DAC0_CH0DATA = sound << 2;
  		*DAC0_CH1DATA = sound << 2;
	}
	else {
		disableTimer();
		disableDAC();
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

