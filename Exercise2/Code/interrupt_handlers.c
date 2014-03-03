#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "efm32gg.h"
#include "game_over.h"
#include "swtheme.h"


#define TOP_AMP 256
#define BUTTOM_AMP 128
#define BASE_PERIOD 128
#define LENGTH_OF_SOUND 20
#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))

uint32_t nofTimes;
uint32_t period_offset;
uint32_t step;


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

	*LETIMER0_CMD = 0x5;

	nofTimes = LENGTH_OF_SOUND;
	step = 512/((BASE_PERIOD - period_offset)/2);
	sample = 0;
	*CMU_OSCENCMD |= CMU2_OSCENCMD_LFRCOEN_E; 
}
/* LETIMER0 interrupt handler */
void __attribute__ ((interrupt)) LETIMER0_IRQHandler() 
{  
	*LETIMER0_IFC = 0x1f;
	unsigned char sound = swtheme[sample];
	sample++;
	if (sound == 0) {
		*LETIMER0_CMD = 1 << 1; //Stop timer. 
		*CMU_OSCENCMD |= 0; //Disable LFRCO when sound is done playing. 
	}
	else {
		*DAC0_CH0DATA = sound;
  		*DAC0_CH1DATA = sound;
  	}
	/*
	if (nofTimes != 0 && *LETIMER0_REP0 < 2) {
		*LETIMER0_REP1 = 0xff;
		nofTimes--;
	}
	// LEDS
	static float t = 0;
	t++;

	
	if (t <= (BASE_PERIOD- period_offset)/2) {
		sound += step;
	}
	else sound -= step;
	if (t >= BASE_PERIOD - period_offset) t = 0;
	
	//uint32_t sound = 2048 + 2048*sin((440*2*PI)/t);
	
	*LETIMER0_IFC = 0x1f;
  	//*GPIO_PA_DOUT = *GPIO_PA_DOUT << 1;

  	// SOUND
  	*DAC0_CH0DATA = sound;
  	*DAC0_CH1DATA = sound;
  	*/
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

