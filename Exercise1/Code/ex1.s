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
	mov r2, #0x2
	str r2, [r1, #GPIO_CTRL]
	
	//Set pins 8-15 to output
	ldr r2, set_pins_output
	str r2, [r1, #GPIO_MODEH]


	


	//Enable input
	ldr r1, gpio_pc_base_address
	ldr r2, set_pins_input
	str r2, [r1, #GPIO_MODEL]
	ldr r2, a_interrupt_trans
	str r2, [r1, #GPIO_DOUT]
	
	
	//Enable interrupt generation
	ldr r1, gpio_base_address
	ldr r2, gpio_extipsell_value1
	str r2, [r1, #GPIO_EXTIPSELL]
	mov r2, #0xff
	str r2, [r1, #GPIO_EXTIFALL]
	str r2, [r1, #GPIO_EXTIRISE]
	str r2, [r1, #GPIO_IEN]



	
	//Enable interrupt
	ldr r1, iser0_address
 	ldr r2, e_interrupt
 	str r2, [r1, #0]

	//Turn off leds
	ldr r1, gpio_pa_base_address
	mov r2, #0xff
	lsl r2, r2, #8
	str r2, [r1, #GPIO_DOUT]

	b .


	/////////////////////////////////////////////////////////////////////////////
	//
	// GPIO handler
	// The CPU will jump here when there is a GPIO interrupt
	//
	/////////////////////////////////////////////////////////////////////////////

        .thumb_func
gpio_handler:
	ldr r1, gpio_pc_base_address
	ldr r1, [r1, #GPIO_DIN]
	
//	ror r3, r2, #16
//	lsr r3, r3, #24
//	and r3, r2, r3
	mov r4, #0xef
	push {r4}
	bl invert_byte
	pop {r4}
	ror r5, r4, #8
	lsr r5, r5, #24
	ror r3, r2, #8
	lsr r3, r3, #24
	and r2, r2, r3
	lsl r2, r2, #8
	
	ldr r1, gpio_pa_base_address
	str r5, [r1, #GPIO_DOUT]
	
	
	//reset interrupt
	ldr r1, gpio_base_address
	ldr r2, a_interrupt_trans
	str r2, [r1, #GPIO_IFC]
	bx lr  // return
	
	/////////////////////////////////////////////////////////////////////////////
	
       
	.thumb_func
	
invert_byte:
	pop {r1}
	pop {r1}
	mov r3, #0
	mov r4, #0
	mov r5, #31
invert_byte_loop:
	ror r2, r1, #1
	lsr r1, r1, #1
	lsr r2, r2, r5
	add r3, r2, r3
	add r4, r4, #1
	sub r5, r5, r4
	cmp r4, #8
	bne invert_byte_loop
	push {r3}
	bx lr
	
	
	
	
	.thumb_func
dummy_handler:

        b .  // do nothing



cmu_base_address:
	.long CMU_BASE

gpio_base_address:
	.long GPIO_BASE

gpio_extipsell_value1:
	.long 0x22222222

iser0_address:
	.long ISER0

a_interrupt_trans:
	.long ACTIVATE_INTERRUPT_TRANS

e_interrupt:	
	.long 0x802

gpio_pa_base_address:
	.long GPIO_PA_BASE

gpio_pc_base_address:
	.long GPIO_PC_BASE

set_pins_output:
	.long 0x55555555

set_pins_input:
	.long 0x33333333

emu_base_address:
	.long EMU_BASE

big_number:
	.long 0x0000000f








