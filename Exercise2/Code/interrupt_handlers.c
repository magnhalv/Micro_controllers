#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "efm32gg.h"

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
	static uint32_t sound = 0;
	sound += 200;
	if (sound > 4095) sound = 0;  
	
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

