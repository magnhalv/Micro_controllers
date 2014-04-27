/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include "efm32gg.h"
#include "swtheme.h"
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/interrupt.h>
#include "battle003.h"

/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */

#define TIMER_TOP_VALUE 0x0001

int sound_open(struct inode *inode, struct file *filep);
int sound_close(struct inode *inode, struct file *filep);
ssize_t sound_read (struct file *filp, char __user *buff, size_t count, loff_t *offp);
ssize_t sound_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp);
int sound_fasync(int fd, struct file *filp, int mode);
static irqreturn_t interrupt_handler (int irq, void *dev_id, struct pt_regs *regs);

void setupTIMER3(void);
void disableTIMER3(void);
void setupDAC(void);
void disableDAC(void);
void setupDMA(void);
void disableDMA(void);

typedef struct {
	uint32_t src_pointer;
	uint32_t dest_pointer;
	uint32_t control;
} DMA_descriptor;



char *sound_array;
int nofSamples;
int nofSamplesTransfered;
DMA_descriptor *descriptor;
void *dac0_ch0data_vaddr;
void *my_memory;

static const char *device_name = "driver-sound";
static dev_t devno;
static struct cdev sound_cdev;
static struct class *cl;
static struct fasync_struct *sound_fasync_t;
static struct file_operations sound_fops = {
	.owner = THIS_MODULE,
	.read = sound_read,
	.write = sound_write,
	.open = sound_open,
	.release = sound_close,
	.fasync = sound_fasync,
};

static int __init template_init(void)
{
	alloc_chrdev_region(&devno, 0, 1, device_name);

	cdev_init(&sound_cdev, &sound_fops);
	cdev_add(&sound_cdev, devno, 1);	

	cl = class_create (THIS_MODULE, device_name);
	device_create(cl, NULL, devno, NULL, device_name);

	if (request_mem_region((unsigned long)DAC0_BASE2, DAC0_MEM_LENGTH, device_name) == NULL) {
		printk(KERN_ALERT "Failed to obtain DAC memory region for:%s\n", device_name);
		return -1;
	}
	if (request_mem_region((unsigned long)TIMER3_BASE, TIMER3_MEM_LENGTH, device_name) == NULL) {
		release_mem_region((unsigned long)DAC0_BASE2, DAC0_MEM_LENGTH);
		printk(KERN_ALERT "Failed to obtain TIMER3 memory region for:%s\n", device_name);
		return -1;
	}
	if (request_mem_region((unsigned long)DMA_BASE, DMA_MEM_LENGTH, device_name) == NULL) {
		release_mem_region((unsigned long)DAC0_BASE2, DAC0_MEM_LENGTH);
		release_mem_region((unsigned long)TIMER3_BASE, TIMER3_MEM_LENGTH);
		printk(KERN_ALERT "Failed to obtain DMA memory region for:%s\n", device_name);
		return -1;
	}
	if(request_irq(20, interrupt_handler, IRQF_DISABLED, device_name, NULL) < 0) {
		printk(KERN_ALERT "Failed to enable interrupt for:%s\n", device_name);
		return -1;
	}
	my_memory = kmalloc(sizeof(DMA_descriptor)*10, GFP_DMA | GFP_KERNEL);
	int offset = (int)my_memory%10;
	int descriptor_addr = my_memory + (10 - offset);
	descriptor = descriptor_addr;
	dac0_ch0data_vaddr = ioremap_nocache((unsigned long)DAC0_CH0DATA, REG_SIZE);
	sound_array = swtheme;
	nofSamples = sizeof(swtheme);
	descriptor->control = 0xc0003ff1;
	descriptor->dest_pointer = DAC0_CH1DATA;
	descriptor->src_pointer = sound_array+0xff*sizeof(char);
	setupDAC();
	setupTIMER3();
	setupDMA();
	printk("Sound driver installed.\n");
	return 0;
}

static void __exit template_cleanup(void)
{
	 printk("Short life for a small module...\n");
}

int sound_open(struct inode *inode, struct file *filep){
	
	return 0;
}

int sound_close(struct inode *inode, struct file *filep) {
	return 0;
}

ssize_t sound_read (struct file *filp, char __user *buff, size_t count, loff_t *offp) {
	return 0;
}

ssize_t sound_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp) {
	return 0;
}

int sound_fasync(int fd, struct file *filp, int mode) {
	// int ret_val = fasync_helper(fd, filp, mode, &timer_fasync_t);
	// if (ret_val < 0) return ret_val;
	return 0;
}

/* DMA interrupt handler */
static irqreturn_t interrupt_handler (int irq, void *dev_id, struct pt_regs *regs) {
	int samplesToTransfer;
	void 	*dma_ifc_vaddr;
	//disableTIMER3();
	dma_ifc_vaddr = ioremap_nocache((unsigned long)DMA_IFC, REG_SIZE);
	printk("Last sample written: %d\n", ioread32(dac0_ch0data_vaddr));
	iowrite32(0xff, dma_ifc_vaddr);

	nofSamplesTransfered += 1024;
	if (nofSamples - nofSamplesTransfered < 0xff) samplesToTransfer = nofSamples - nofSamplesTransfered;
	else samplesToTransfer = 1024;
	if (nofSamplesTransfered < nofSamples) {
		descriptor->control = 0xc0003ff1;
		descriptor->dest_pointer = DAC0_CH1DATA;
		descriptor->src_pointer = (sound_array+nofSamplesTransfered)+samplesToTransfer*sizeof(char);
		setupDMA();
		//setupTIMER3();
	}
	else {
		printk("Sound done playing.\n");
		disableDAC();
		disableTIMER3();
	}
	return IRQ_HANDLED;
}

/*******************************/
/*   Setup/disable functions   */
/*******************************/

void setupTIMER3() {
	void	*timer3_ctrl_vaddr,
			*timer3_cmd_vaddr,
			*timer3_ien_vaddr,
			*timer3_top_vaddr,
			*timer3_cnt_vaddr,
			*prs_ch0_ctrl_vaddr;

	timer3_ctrl_vaddr = ioremap_nocache((unsigned long)TIMER3_CTRL, REG_SIZE);
	timer3_cmd_vaddr = ioremap_nocache((unsigned long)TIMER3_CMD, REG_SIZE);
	timer3_ien_vaddr = ioremap_nocache((unsigned long)TIMER3_IEN, REG_SIZE);
	timer3_top_vaddr = ioremap_nocache((unsigned long)TIMER3_TOP, REG_SIZE);
	timer3_cnt_vaddr = ioremap_nocache((unsigned long)TIMER3_CNT, REG_SIZE);
	prs_ch0_ctrl_vaddr = ioremap_nocache((unsigned long)PRS_CH0_CTRL, REG_SIZE);

	iowrite32((0 << 24) | 0x1, timer3_ctrl_vaddr);
	//iowrite32(TIMER3_UF_INTERRUPT, timer3_ien_vaddr);
	iowrite32(TIMER_TOP_VALUE, timer3_top_vaddr);
	iowrite32(SOURCESEL, prs_ch0_ctrl_vaddr);
	iowrite32(TIMER3_CMD_START, timer3_cmd_vaddr);

}

void disableTIMER3() {
	void	*timer3_ctrl_vaddr,
			*timer3_cmd_vaddr,
			*timer3_ien_vaddr,
			*timer3_top_vaddr,
			*timer3_cnt_vaddr;

	timer3_cmd_vaddr = ioremap_nocache((unsigned long)TIMER3_CMD, REG_SIZE);
	//timer3_ctrl_vaddr = ioremap_nocache((unsigned long)TIMER3_CTRL, REG_SIZE);
	// timer3_ien_vaddr = ioremap_nocache((unsigned long)TIMER3_IEN, REG_SIZE);
	// timer3_top_vaddr = ioremap_nocache((unsigned long)TIMER3_TOP, REG_SIZE);
	// timer3_cnt_vaddr = ioremap_nocache((unsigned long)TIMER3_CNT, REG_SIZE);

	// iowrite32(0x1, timer3_ctrl_vaddr);
	// iowrite32(TIMER3_UF_INTERRUPT, timer3_ien_vaddr);
	// iowrite32(TIMER_TOP_VALUE, timer3_top_vaddr);
	iowrite32(TIMER3_CMD_STOP, timer3_cmd_vaddr);

}

void setupDAC() {

	void 	*dac0_ctrl_vaddr,
			*dac0_ch0ctrl_vaddr,
			*dac0_ch1ctrl_vaddr;



	//*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0; /* Enable DAC clk gate */
	dac0_ctrl_vaddr = ioremap_nocache((unsigned long)DAC0_CTRL, REG_SIZE);
	iowrite32(0x50010, dac0_ctrl_vaddr);

	/* Enable channels and make them listen to PRS channel 0 */
	dac0_ch0ctrl_vaddr = ioremap_nocache((unsigned long)DAC0_CH0CTRL, REG_SIZE);
	iowrite32(DAC_ENABLE_CH, dac0_ch0ctrl_vaddr);

	dac0_ch1ctrl_vaddr = ioremap_nocache((unsigned long)DAC0_CH1CTRL, REG_SIZE);
	iowrite32(DAC_ENABLE_CH, dac0_ch1ctrl_vaddr);

}

void disableDAC() {
	void 	*dac0_ctrl_vaddr,
			*dac0_ch0ctrl_vaddr,
			*dac0_ch1ctrl_vaddr;

	//*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_DAC0; /* Enable DAC clk gate */
	dac0_ctrl_vaddr = ioremap_nocache((unsigned long)DAC0_CTRL, REG_SIZE);
	iowrite32(0, dac0_ctrl_vaddr);

	dac0_ch0ctrl_vaddr = ioremap_nocache((unsigned long)DAC0_CH0CTRL, REG_SIZE);
	iowrite32(0, dac0_ch0ctrl_vaddr);

	dac0_ch1ctrl_vaddr = ioremap_nocache((unsigned long)DAC0_CH1CTRL, REG_SIZE);
	iowrite32(0, dac0_ch1ctrl_vaddr);
}



void setupDMA() {

	void 	*dma_config_vaddr,
			*dma_ch0_ctrl_vaddr,
			*dma_ctrlbase_vaddr,
			*dma_ien_vaddr,
			*dma_chens_vaddr,
			*dma_chreqmaskc_vaddr,
			*dma_chaltc_vaddr;

	dma_config_vaddr = ioremap_nocache((unsigned long)DMA_CONFIG, REG_SIZE);
	dma_ch0_ctrl_vaddr = ioremap_nocache((unsigned long)DMA_CH0_CTRL, REG_SIZE);
	dma_ctrlbase_vaddr = ioremap_nocache((unsigned long)DMA_CTRLBASE, REG_SIZE);
	dma_ien_vaddr = ioremap_nocache((unsigned long)DMA_IEN, REG_SIZE);
	dma_chens_vaddr = ioremap_nocache((unsigned long)DMA_CHENS, REG_SIZE);
	dma_chreqmaskc_vaddr = ioremap_nocache((unsigned long)DMA_CHREQMASKC, REG_SIZE);
	dma_chaltc_vaddr = ioremap_nocache((unsigned long)DMA_CHALTC, REG_SIZE);

	iowrite32(0x1, dma_ien_vaddr);
	/* LETIMER0_CH0 set as default trigger on PRS CH0. */
	//*PRS_CH0_CTRL |= 1 << 28;
 	iowrite32(SOURCESEL, dma_ch0_ctrl_vaddr);
	iowrite32(descriptor, dma_ctrlbase_vaddr);
	iowrite32(0x1, dma_config_vaddr);
	iowrite32(0x1, dma_chaltc_vaddr);
	iowrite32(0x1, dma_chreqmaskc_vaddr);
	iowrite32(0x1, dma_chens_vaddr);
}


module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");

