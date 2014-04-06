/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>

/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */


const char *device_name = "driver-gamerpad";

static int __init template_init(void)
{
	printk("Hello World, here is your module speaking\n I have changed!");
	if (request_mem_region(GPIO_PA_BASE, GPIO_Px_MEM_LENGTH, DEVICE_NAME) == NULL) {
		printk(KERN_ALERT "Failed to obtain GPIO_PA memory address for:%s", DEVICE_NAME);
		return -1;
	}
	if (request_mem_region(GPIO_PC_BASE, GPIO_Px_MEM_LENGTH, DEVICE_NAME) == NULL) {
		release_mem_region(GPIO_PA_BASE, GPIO_Px_MEM_LENGTH);
		printk(KERN_ALERT "Failed to obtain GPIO_PB memory address for:%s", DEVICE_NAME);
		return -1;
	}
	if (request_mem_region(GPIO_EXTIPSELL, GPIO_I_MEM_LENGTH, DEVICE_NAME) == NULL) {
		release_mem_region(GPIO_PA_BASE, GPIO_Px_MEM_LENGTH);
		release_mem_region(GPIO_PC_BASE, GPIO_Px_MEM_LENGTH);
		printk(KERN_ALERT "Failed to obtain GPIO interrupt memory address for:%s", DEVICE_NAME);
		return -1;
	}
	if (request_mem_region(CMU_BASE2, CMU_MEM_LENGTH, DEVICE_NAME)) {
		release_mem_region(GPIO_PA_BASE, GPIO_Px_MEM_LENGTH);
		release_mem_region(GPIO_PC_BASE, GPIO_Px_MEM_LENGTH);
		release_mem_region(GPIO_EXTIPSELL, GPIO_I_MEM_LENGTH);
		printk(KERN_ALERT "Failed to obtain GPIO interrupt memory address for:%s", DEVICE_NAME);
	}

	void *cmu_hfperclken0_vaddr = ioremap_nocache(CMU_HFPERCLKEN0, 0x4);
	iowrite(ioread32(cmu_hfperclken0_vaddr) | CMU2_HFPERCLKEN0_GPIO, cmu_hfperclken0_vaddr);

	





	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO; /* enable GPIO clock*/
     /* Enable output */
  	*GPIO_PA_CTRL = 0x2;  /* set high drive strength */
  	*GPIO_PA_MODEH = 0x55555555; /* set pins A8-15 as output */
  	*GPIO_PA_DOUT = 0xff00; /* turn off LEDs D4-D8 (LEDs are active low) */
     /* Enable input */
     *GPIO_PC_MODEL = 0x33333333;
     *GPIO_PC_DOUT = 0xff;

     /* Enable GPIO interrupt */
     *GPIO_EXTIPSELL = 0x22222222;
     *GPIO_EXTIFALL = 0xff;
     *GPIO_IEN = 0xff;

	return 0;
}

/*
 * template_cleanup - function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit template_cleanup(void)
{
	 printk("Short life for a small module...\n");
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");

