       .syntax unified
	
	      .include "efm32gg.s"

	/////////////////////////////////////////////////////////////////////////////
	//
  // Exception vector table
  // This table contains addresses for all exception handlers
	//
	/////////////////////////////////////////////////////////////////////////////
	
        .section .vectors
	
	      .long   stack_top               /* Top of Stack                 */
	      .long   _reset                  /* Reset Handler                */
	      .long   dummy_handler           /* NMI Handler                  */
	      .long   dummy_handler           /* Hard Fault Handler           */
	      .long   dummy_handler           /* MPU Fault Handler            */
	      .long   dummy_handler           /* Bus Fault Handler            */
	      .long   dummy_handler           /* Usage Fault Handler          */
	      .long   dummy_handler           /* Reserved                     */
	      .long   dummy_handler           /* Reserved                     */
	      .long   dummy_handler           /* Reserved                     */
	      .long   dummy_handler           /* Reserved                     */
	      .long   dummy_handler           /* SVCall Handler               */
	      .long   dummy_handler           /* Debug Monitor Handler        */
	      .long   dummy_handler           /* Reserved                     */
	      .long   dummy_handler           /* PendSV Handler               */
	      .long   dummy_handler           /* SysTick Handler              */

	      /* External Interrupts */
	      .long   dummy_handler
	      .long   gpio_handler            /* GPIO even handler */
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   gpio_handler            /* GPIO odd handler */
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler
	      .long   dummy_handler

	      .section .text

	/////////////////////////////////////////////////////////////////////////////
	//

	// Reset handler
  // The CPU will start executing here after a reset
	//
	/////////////////////////////////////////////////////////////////////////////

	      .globl  _reset
	      .type   _reset, %function
        .thumb_func
_reset:
	//Enable GPIO
	ldr r1, cmu_base_address
	ldr r2, [r1, #CMU_HFPERCLKEN0]
	mov r3, #1
	lsl r3, r3, #CMU_HFPERCLKEN0_GPIO
	orr r2, r2, r3
	str r2 , [ r1 , #CMU_HFPERCLKEN0]

	



	//Set high drive strength
	ldr r1, gpio_pa_base_address
	ldr r2, #two_value
	str r2, [r1, #GPIO_CTRL]
	
	//Set pins 8-15 to output
	ldr r2, set_pins_output_value
	str r2, [r1, #GPIO_MODEH]


	//Enable input
	ldr r1, gpio_pc_base_address
	ldr r2, set_pins_input_value
	str r2, [r1, #GPIO_MODEL]
	ldr r2, a_interrupt_trans
	str r2, [r1, #GPIO_DOUT]
	
	
	//Enable interrupt generation
	ldr r1, gpio_base_address
	ldr r2, gpio_extipsell_value1
	str r2, [r1, #GPIO_EXTIPSELL]
	ldr r2, #a_interrupt_trans
	str r2, [r1, #GPIO_EXTIFALL]
	str r2, [r1, #GPIO_EXTIRISE]
	str r2, [r1, #GPIO_IEN]


	mov r1, #1000
loop:
	sub r1, #1
	cmp r1, #0
	bne loop
	//Enable interrupts
	ldr r1, iser0_address
	ldr r2, e_interrupt
	str r2, [r1, #0]
	
	b .
	

	


	
	
	
	
	
	
	/////////////////////////////////////////////////////////////////////////////
	//
  // GPIO handler
  // The CPU will jump here when there is a GPIO interrupt
	//
	/////////////////////////////////////////////////////////////////////////////
	
        .thumb_func
gpio_handler:  
	//Set led 10 low
	ldr r1, gpio_pa_base_address
	ldr r3, [r1, #GPIO_DOUT]
	mov r2, #1
	lsl r2, r2, #10
	orr r3, r3, r2
	str r3, [r1, #GPIO_DOUT]
	//reset interrupt
	ldr r1, gpio_base_address
	ldr r2, a_interrupt_trans
	str r2, [r1, #GPIO_IFC]
	bx lr  // return
	
	/////////////////////////////////////////////////////////////////////////////
	
        .thumb_func
dummy_handler:

        b .  // do nothing



cmu_base_address:
	.long CMU_BASE

gpio_base_address:
	.long GPIO_BASE

gpio_extipsell_value1:
	.long GPIO_EXTIPSELL_VALUE

iser0_address:
	.long ISER0

a_interrupt_trans:
	.long ACTIVATE_INTERRUPT_TRANS

e_interrupt:	
	.long ENABLE_INTERRUPT

gpio_pa_base_address:
	.long GPIO_PA_BASE

gpio_pc_base_address:
	.long GPIO_PC_BASE

set_pins_output_value:
	.long SET_PINS_OUTPUT

set_pins_input_value:
	.long SET_PINS_INPUT

set_leds_value:
	.long SET_LEDS

two_value:
	.long TWO







