#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

void gpio_interrupt () {
	*GPIO_IFC = 0xff;
	uint32_t input_status = *GPIO_PC_DIN;
	input_status = input_status << 8;
	*GPIO_PA_DOUT = input_status;
}
/* TIMER1 interrupt handler */
void __attribute__ ((interrupt)) TIMER1_IRQHandler() 
{  
  /*
    TODO feed new samples to the DAC
    remember to clear the pending interrupt by writing 1 to TIMER1_IFC
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

