/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include "efm32gg.h"

/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */

static int my_open(struct inode *inode, struct file *filp);
static int my_close(struct inode *inode, struct file *filp);
static ssize_t my_read (struct file *filp, char __user *buff, size_t count, loff_t *offp);
static ssize_t my_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp);


static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
	.open = my_open,
	.release = my_close,
};
const char *device_name = "driver-gamepad";
static dev_t devno;
static struct cdev my_cdev;
static struct class *cl;

static int __init template_init(void)
{

	// void *gpio_pa_ctrl_vaddr, 
	// 	 *gpio_pa_modeh_vaddr, 
	// 	 *gpio_pa_dout_vaddr;
		 
	alloc_chrdev_region(&devno, 0, 1, device_name);

	cdev_init(&my_cdev, &my_fops);
	cdev_add(&my_cdev, devno, 1);	

	cl = class_create (THIS_MODULE, device_name);
	device_create(cl, NULL, devno, NULL, device_name);
	
	// if (request_mem_region((unsigned long)GPIO_PA_BASE, GPIO_Px_MEM_LENGTH, device_name) == NULL) {
	// 	printk(KERN_ALERT "Failed to obtain GPIO_PA memory address for:%s", device_name);
	// 	return -1;
	// }
	// if (request_mem_region((unsigned long)GPIO_PC_BASE, GPIO_Px_MEM_LENGTH, device_name) == NULL) {
	// 	release_mem_region((unsigned long)GPIO_PA_BASE, GPIO_Px_MEM_LENGTH);
	// 	printk(KERN_ALERT "Failed to obtain GPIO_PB memory address for:%s", device_name);
	// 	return -1;
	// }
	// if (request_mem_region((unsigned long)GPIO_EXTIPSELL, GPIO_I_MEM_LENGTH, device_name) == NULL) {
	// 	release_mem_region((unsigned long)GPIO_PA_BASE, GPIO_Px_MEM_LENGTH);
	// 	release_mem_region((unsigned long)GPIO_PC_BASE, GPIO_Px_MEM_LENGTH);
	// 	printk(KERN_ALERT "Failed to obtain GPIO interrupt memory address for:%s", device_name);
	// 	return -1;
	// }
	


	// /* Enable output for LED12, LED13, LED14 */
	// gpio_pa_ctrl_vaddr = ioremap_nocache((unsigned long)GPIO_PA_CTRL, REG_SIZE);
	// iowrite32(0x2, gpio_pa_ctrl_vaddr); /*Set high drive strength */
	
	// gpio_pa_modeh_vaddr = ioremap_nocache((unsigned long)GPIO_PA_MODEH, REG_SIZE);
	// iowrite32(ioread32(gpio_pa_modeh_vaddr) | 0x05550000, gpio_pa_modeh_vaddr);

	// gpio_pa_dout_vaddr = ioremap_nocache((unsigned long)GPIO_PA_DOUT, REG_SIZE);
	// iowrite32(0x7000, gpio_pa_dout_vaddr);

	/* Enable input */

	





	// *CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO; /* enable GPIO clock*/
 //     /* Enable output */
 //  	*GPIO_PA_CTRL = 0x2;  /* set high drive strength */
 //  	*GPIO_PA_MODEH = 0x55555555; /* set pins A8-15 as output */
 //  	*GPIO_PA_DOUT = 0xff00;  turn off LEDs D4-D8 (LEDs are active low) 
 //     /* Enable input */
 //     *GPIO_PC_MODEL = 0x33333333;
 //     *GPIO_PC_DOUT = 0xff;

 //     /* Enable GPIO interrupt */
 //     *GPIO_EXTIPSELL = 0x22222222;
 //     *GPIO_EXTIFALL = 0xff;
 //     *GPIO_IEN = 0xff;


	printk("Successfully initalized: %s.", device_name);
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
	release_mem_region((unsigned long)GPIO_PA_BASE, GPIO_Px_MEM_LENGTH);
	release_mem_region((unsigned long)GPIO_PC_BASE, GPIO_Px_MEM_LENGTH);
	release_mem_region((unsigned long)GPIO_EXTIPSELL, GPIO_I_MEM_LENGTH);

	unregister_chrdev_region(&devno, 1);
	printk("Short life for a small module...\n");
}

static int my_open(struct inode *inode, struct file *filp) {
	printk("Open was called.\n");
	
	return 0;
}

static int my_close(struct inode *inode, struct file *filp) {
	return 0;
}

static ssize_t my_read (struct file *filp, char __user *buff, size_t count, loff_t *offp) {
	return 0;
}
static ssize_t my_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp) {
	return 0;
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");

