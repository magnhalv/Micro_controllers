/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/interrupt.h>

int timer_open(struct inode *inode, struct file *filep);
int timer_close(struct inode *inode, struct file *filep);
ssize_t timer_read (struct file *filp, char __user *buff, size_t count, loff_t *offp);
ssize_t timer_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp);
void my_timer_callback(unsigned long data);
int timer_fasync(int fd, struct file *filp, int mode);

uint16_t msecs;

static const char *device_name = "driver-timer";
static dev_t devno;
static struct cdev timer_cdev;
static struct class *cl;
static struct fasync_struct *timer_fasync_t;
static struct file_operations timer_fops = {
	.owner = THIS_MODULE,
	.read = timer_read,
	.write = timer_write,
	.open = timer_open,
	.release = timer_close,
	.fasync = timer_fasync,
};

static struct timer_list my_timer;

void my_timer_callback( unsigned long data ) {
	msecs = 0;
	kill_fasync(&timer_fasync_t, SIGIO, POLL_IN);
}

static int __init template_init(void) {

	alloc_chrdev_region(&devno, 0, 1, device_name);

	cdev_init(&timer_cdev, &timer_fops);
	cdev_add(&timer_cdev, devno, 1);	

	cl = class_create (THIS_MODULE, device_name);
	device_create(cl, NULL, devno, NULL, device_name);
	printk("Timer module installing\n");
	return 0;
}

static void __exit template_cleanup(void){
	int ret;

	ret = del_timer( &my_timer );
	if (ret) printk("The timer is still in use...\n");
	unregister_chrdev_region(devno, 1);
	printk("Timer module uninstalling\n");

}

int timer_open(struct inode *inode, struct file *filp) {
	setup_timer( &my_timer, my_timer_callback, 0 );
	return 0;
}

int timer_close(struct inode *inode, struct file *filp) {
	msecs = 0;
	timer_fasync(-1, filp, 0);
	return 0;
}

ssize_t timer_read (struct file *filp, char __user *buff, size_t count, loff_t *offp) {
	if (msecs == 0) *buff = 1;
	else *buff = 0;
	return 0;
}
ssize_t timer_write (struct file *filp, const char __user *buff, size_t count, loff_t *offp) {
	
	msecs = ((uint16_t)(buff[1] << 8)) + buff[0];
	
	int ret;
	ret = mod_timer( &my_timer, jiffies + msecs_to_jiffies(msecs) );
	if (ret) printk("Error in mod_timer\n");
	return 0;
}

int timer_fasync(int fd, struct file *filp, int mode) {
	int ret_val = fasync_helper(fd, filp, mode, &timer_fasync_t);
	if (ret_val < 0) return ret_val;
	return 0;
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");

