#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>

#define DRIVER_AUTHOR "Ovazza - Kossyfidis"
#define DRIVER_DESC "Hello world Module"
#define DRIVER_LICENSE "GPL"

#define SIZE 4

static struct timer_list timer;
static unsigned long basePattern;
static unsigned long interval;
static unsigned long pattern = 0;

char message[1024];

int freq = 1;
module_param(freq, int, 0); 


static struct proc_dir_entry *ensea_proc_dir;
static struct proc_dir_entry *chenille_proc_entry;

ssize_t read_func(struct file* file, char __user* buffer, size_t count, loff_t* ppos) {
    int copy;
    if (count > SIZE) count = SIZE;

    if(access_ok(VERIFY_READ, buffer, count))
    {
        if ((copy = copy_to_user(buffer, message, count)))
        {
            printk("Error copy to user\n");
            return -EFAULT;
        }

        printk("Copy to user ok\n");
    }
    else
    {
        printk("Error Acces_Ok Read\n");
        return -EFAULT;
    }

    return count-copy;
}

ssize_t write_func(struct file * file, const char __user * buffer, size_t count, loff_t * ppos)
{
    int len = count;
    if (len > SIZE) len = SIZE;
    
    
    if(access_ok(VERIFY_WRITE, buffer, count))
    {
        if (copy_from_user(message, buffer, count))
        {
            printk("Error copy to user\n");
            return -EFAULT;
        }
        message[count] = '\0';
        printk("Copy from user ok\n");
        printk("Message : %s\n", message);
        
    
        memcpy(&basePattern, message, SIZE);
    }
    else
    {
        printk("Error Acces_Ok Write\n");
        return -EFAULT;
    }
    

    return count;
}

static const struct file_operations chenille_proc_fops = {
    .owner = THIS_MODULE,
    .read = read_func,
    .write = write_func
};


static void mytimer(unsigned long data)
{
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

int __init hello_init(void)
{
    ensea_proc_dir = proc_mkdir("ensea", NULL);
    chenille_proc_entry = proc_create("chenille", 666, ensea_proc_dir, &chenille_proc_fops);


	printk(KERN_INFO "Hello world!\n");
    interval = HZ / freq;

    init_timer(&timer);
    timer.function=mytimer;
    timer.data=0;
    mod_timer(&timer, jiffies + interval);
	return 0;
}
    
void __exit hello_exit(void)
{
    remove_proc_entry("chenille", ensea_proc_dir);
    remove_proc_entry("ensea", NULL);

    del_timer(&timer);
	printk(KERN_ALERT "Bye bye...\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
