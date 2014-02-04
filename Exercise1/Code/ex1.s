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
	      .long   letimer0_handler
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
	//Enable GPIO clock
	ldr r1, cmu_base_address
	ldr r2, [r1, #CMU_HFPERCLKEN0]
	mov r3, #1
	lsl r3, r3, #CMU_HFPERCLKEN0_GPIO
	orr r2, r2, r3
	str r2 , [ r1 , #CMU_HFPERCLKEN0]

	//Enable LE clock
	ldr r1, cmu_base_address
	ldr r2, [r1, #CMU_HFCORECLKEN0]
	mov r3, #1
	lsl r3, r3, #CMU_HFCORECLKEN0_LE
	orr r2, r2, r3
	str r2 , [ r1 , #CMU_HFCORECLKEN0]

	//Enable LETIMER0 clock
	ldr r2, [r1, #CMU_LFACLKEN0]
	mov r3, #1
	lsl r3, r3, #CMU_LFACLKEN0_LETIMER0
	orr r2, r2, r3
	str r2, [r1, #CMU_LFACLKEN0]



	//Set high drive strength
	ldr r1, gpio_pa_base_address
	mov r2, #0x2
	str r2, [r1, #GPIO_CTRL]
	
	//Set pins 8-15 to output
	ldr r2, set_pins_output
	str r2, [r1, #GPIO_MODEH]


	//Turn off leds
	ldr r1, gpio_pa_base_address
	mov r2, #0xff
	lsl r2, r2, #8
	str r2, [r1, #GPIO_DOUT]


	//Enable input
	ldr r1, gpio_pc_base_address
	ldr r2, set_pins_input
	str r2, [r1, #GPIO_MODEL]
	mov r2, #0xff
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
 	ldr r2, enable_interrupts
 	str r2, [r1, #0]



	ldr r1, emu_base_address
	mov r2, #0x3
	str r2, [r1, #EMU_CTRL]
	
	b .


	/////////////////////////////////////////////////////////////////////////////
	//
	// GPIO handler
	// The CPU will jump here when there is a GPIO interrupt
	//
	/////////////////////////////////////////////////////////////////////////////

        .thumb_func
gpio_handler:
	//Get button status. 
	ldr r1, gpio_pc_base_address
	ldr r1, [r1, #GPIO_DIN]

	//Invert the button status byte
	push {lr, r1}
	push {r1}
	bl invert_byte
	pop {r1, r2, lr}

	//Combine the two bytes, thus setting the complementary LEDs low.
	and r1, r1, r2
	lsl r1, r1, #8

	//Set leds
	ldr r2, gpio_pa_base_address
	str r1, [r2, #GPIO_DOUT]


	//TIMER TEST

	
	
	//set comp
	ldr r1, letimer0_base_address
	mov r2, #5
	str r2, [r1, #LETIMER0_COMP0]
	mov r2, #0
	str r2, [r1, #LETIMER0_COMP1]

	//Enable interrupt
	mov r2, #2
	str r2, [r1, #LETIMER0_IEN]
	
	//start timer
	mov r2, #1
	str r2, [r1, #LETIMER0_CMD]
	
	
	//reset interrupt
	ldr r1, gpio_base_address
	mov r2, #0xff
	str r2, [r1, #GPIO_IFC]
	bx lr 
	
	/////////////////////////////////////////////////////////////////////////////
	

	.thumb_func
letimer0_handler:
	ldr r1, gpio_pa_base_address
	mov r2, #0xff
	lsl r2, r2, #8
	str r2, [r1, #GPIO_DOUT]
	b .
	.thumb_func
	
invert_byte:
	pop {r1}
	mov r3, #0
	mov r4, #24
invert_byte_loop:
	ror r2, r1, #1
	lsr r1, r1, #1
	lsr r2, r2, r4
	add r3, r2, r3
	add r4, r4, #1
	cmp r4, #32
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

enable_interrupt:
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

letimer0_base_address:
	.long LETIMER0_BASE

enable_interrupts:
	.long 0x2000802









