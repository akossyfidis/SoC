#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/malloc.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/init.h>
#include <linux/timer.h>

#define DRIVER_AUTHOR "Ovazza - Kossyfidis"
#define DRIVER_DESC "Hello world Module"
#define DRIVER_LICENSE "GPL"

static char* pattern = "00000001";
static struct timer_list timer;
static unsigned long basePattern;
static unsigned long interval;

int freq = 1;
MODULE_PARM(freq, "i");

struct file_operations chenille_proc_fops
{
    .owner = THIS_MODULE,
    .read = read_func,
    .write = write_func,
}

ssize_t read_func(struct file* file, char __user* buffer, size_t count, loff_t* ppos) {
    int copy;
    if (count > 8) count = 8;
    if (copy = copy_to_user(buffer, message, count)) return -EFAULT;
    return count-copy;
}

ssize_t fops_write(struct file * file, const char __user * buffer,
size_t count, loff_t * ppos)
{
    int len = count;
    if (len > TAILLE) len = TAILLE;
    
    if (copy_from_user(message, buffer, count)) return -EFAULT;
    message[count] = '\0';
    
    memcpy(pattern, buffer, 8);

    return count;
}

static void mytimer(unsigned long data)
{
    static unsigned long pattern = basePattern;
    printk("0x%08lx\n", pattern);
    if (pattern > 0)
    {
        pattern = pattern << 4;
    }
    else
    {
        pattern = basePattern;
    }
    mod_timer(&timer, jiffies + interval);
}

int hello_init(void)
{
    ensea_proc_dir = proc_mkdir("ensea", NULL);
    chenille_proc_entry = proc_create("chenille", 0, ensea_proc_dir, &chenille_proc_fops);


	printk(KERN_INFO "Hello world!\n");
    interval = HZ / freq;

    for (size_t i = 0; i < 8; i++)
    {
        basePattern |= (data[i] - '0') << (28 - 4 * i);
    }

    init_timer(&timer);
    timer.function=mytimer;
    timer.data=0;
    mod_timer(&timer, jiffies + interval);
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
