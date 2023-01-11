#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#define INTERVAL HZ/2

#define DRIVER_AUTHOR "Ovazza - Kossyfidis"
#define DRIVER_DESC "Hello world Module"
#define DRIVER_LICENSE "GPL"

static struct timer_list timer;

static void mytimer(unsigned long data)
{
    static unsigned long pattern = 0x00000001;
    printk("0x%08lx\n", pattern);
    if (pattern > 0)
    {
        pattern = pattern << 4;
    }
    else
    {
        pattern = 0x00000001;
    }
    mod_timer(&timer, jiffies + INTERVAL);
}

int hello_init(void)
{
	printk(KERN_INFO "Hello world!\n");
    init_timer(&timer);
    timer.function=mytimer;
    timer.data=0;
    mod_timer(&timer, jiffies + INTERVAL);
	return 0;
}

void hello_exit(void)
{
    del_timer(&timer);
	printk(KERN_ALERT "Bye bye...\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
