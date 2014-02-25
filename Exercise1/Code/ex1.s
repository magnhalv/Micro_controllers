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
	///////CLOCK CONFIGURATIONS/////////
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

	//Set the ULFRCO (1 kHz) as clock source for LETIMER0, to save energy.
	ldr r2, [r1, #CMU_LFCLKSEL]
	lsr r2, #2
	lsl r2, #2
	mov r3, #1
	lsl r3, r3, CMU_LFCLKSEL_LFAE
	orr r3, r2, r3
	str r3, [r1, #CMU_LFCLKSEL]
	

	/////// I/O CONFIGURATIONS ///////

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


	//Enable gpio_pc input
	ldr r1, gpio_pc_base_address
	ldr r2, set_pins_input
	str r2, [r1, #GPIO_MODEL]
	mov r2, #0xff
	str r2, [r1, #GPIO_DOUT]
	

	//////// GPIO INTERRUPT CONFIGURATIONS ///////
	//Enable interrupt generation on falling 
	ldr r1, gpio_base_address
	ldr r2, gpio_extipsell_value1
	str r2, [r1, #GPIO_EXTIPSELL]
	mov r2, #0xff
	str r2, [r1, #GPIO_EXTIFALL]
//	str r2, [r1, #GPIO_EXTIRISE]
	str r2, [r1, #GPIO_IEN]

	//Enable gpio interrupt
	ldr r1, iser0_address
 	ldr r2, enable_interrupts
 	str r2, [r1, #0]

	/////// LETIMER0 CONFIGURATIONS ///////
	//set comp0 top value and buffered mode
	ldr r1, letimer0_base_address
	mov r2, #1
	lsl r2, r2, #9
	ldr r3, [r1, #LETIMER0_CTRL]
	orr r3, r2, r3
	add r3, r3, #2
	str r3, [r1, #LETIMER0_CTRL]

	//Set buffered values
	mov r2, #0x01
	str r2, [r1, #LETIMER0_REP0]
	str r2, [r1, #LETIMER0_REP1]

	
	//Set top value and compare
	ldr r2, =0x00FF
	str r2, [r1, #LETIMER0_COMP0]

	//Enable interrupt
	mov r2, #0x08
	str r2, [r1, #LETIMER0_IEN]

	

forever_loop:	
	WFI
	b forever_loop


	/////////////////////////////////////////////////////////////////////////////
	//
	// GPIO handler
	// The CPU will jump here when there is a GPIO interrupt
	//
	/////////////////////////////////////////////////////////////////////////////

        .thumb_func
gpio_handler:

	//Clear gpio interrupt
	ldr r1, gpio_base_address
	mov r2, #0xff
	str r2, [r1, #GPIO_IFC]

	//Set initial LEDS //

	//Get button status. 
	ldr r1, gpio_pc_base_address
	ldr r1, [r1, #GPIO_DIN]

	//Invert the button status byte
	push {lr, r1} //save registers
	push {r1} //push arguments
	bl revert_byte
	pop {r1, r2, lr} //pop return value and restore registers

	//Combine the two bytes, thus setting the complementary LEDs low.
	and r1, r1, r2
	lsl r1, r1, #8

	//Set leds
	ldr r2, gpio_pa_base_address
	str r1, [r2, #GPIO_DOUT]
	mov r6, r1 //Store value for later. Used in letimer0_handler
	add r6, r6, #0xff //Add for easier shifting of leds


	//Start timer. Should cause 8 interrupts, which will be used to shift the leds to
	//the right on the board.

	mov r7, #0x8 //Counter for nof timer interrupts


	//Stop timer, in case it's running. 
	ldr r1, letimer0_base_address
	mov r2, #2
	str r2, [r1, #LETIMER0_CMD]
	mov r2, #4
	str r2, [r1, #LETIMER0_CMD]
	//Start and clear letimer0
	mov r2, #1
	str r2, [r1, #LETIMER0_CMD]

	
	bx lr 
	
	/////////////////////////////////////////////////////////////////////////////
	

	.thumb_func
letimer0_handler:
	sub r7, r7, #1 //decrement number of interrupts
	cmp r7, #1
	ble dont_update_REP1
	//Write to REP1, so another interrupt will happen.
	ldr r8, letimer0_base_address
	mov r4, #0x01
	str r4, [r8, #LETIMER0_REP1]
dont_update_REP1:
	//Shift the LEDS to the right on the board.
	lsl r6, r6, #1
	ldr r4, gpio_pa_base_address
	str r6, [r4, #GPIO_DOUT]
	add r6, r6, #1

	//Clear letimer0 interrupt.
	ldr r8, letimer0_base_address
	mov r7, #0x08
	str r7, [r8, #LETIMER0_IFC]
	bx lr

	////////////////////////////////////////////////////////////////////////////
	
	.thumb_func	
revert_byte:
	pop {r1} //pop parameter
	mov r3, #0
	mov r4, #24
revert_byte_loop:
	ror r2, r1, #1
	lsr r1, r1, #1
	lsr r2, r2, r4
	add r3, r2, r3
	add r4, r4, #1
	cmp r4, #32
	bne revert_byte_loop
	push {r3} //push return value
	bx lr
	
	
	/////////////////////////////////////////////////////////////////////////////
	
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
	.long 0x4000802









