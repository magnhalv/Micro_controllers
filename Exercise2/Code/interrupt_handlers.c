#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "efm32gg.h"
#include <math.h>

#define PI 3.14159265

void gpio_interrupt () {
	*GPIO_IFC = 0xff;
	uint32_t input_status = *GPIO_PC_DIN;
	input_status = input_status << 8;
	*GPIO_PA_DOUT = input_status + 0xff;

	*LETIMER0_REP0 = 0xff;
	*LETIMER0_CMD = 0x5;

}
/* LETIMER0 interrupt handler */
void __attribute__ ((interrupt)) LETIMER0_IRQHandler() 
{  
	// LEDS
	*LETIMER0_REP0 = 0xff;
	static float t = 0;
	static uint32_t sound = 0;
	t += 0.016;
	sound += 67;
	if (t > 1.0/440) {
		t -= 1.0/440;
		sound = 0;
	}
	
	//uint32_t sound = 2048 + 2048*sin(440*2*PI*t);
	
	*LETIMER0_IFC = 0x1f;
  	*GPIO_PA_DOUT = *GPIO_PA_DOUT << 1;

  	// SOUND
  	*DAC0_CH0DATA = sound & 0xfff;
  	*DAC0_CH1DATA = sound & 0xfff;
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

