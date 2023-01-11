#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>
#include <linux/init.h>
#include <linux/timer.h>
#define INTERVAL HZ/2

#define DRIVER_AUTHOR "Ovazza - Kossyfidis"
#define DRIVER_DESC "Hello world Module"
#define DRIVER_LICENSE "GPL"

static struct timer_list timer;
static unsigned long basePattern;
static unsigned long interval;

int freq = 1;
MODULE_PARM(freq, "i");

struct file *file_open(const char *path, int flags, int rights) 
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file) 
{
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size) 
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
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
    mod_timer(&timer, jiffies + INTERVAL);
}

int hello_init(void)
{
	printk(KERN_INFO "Hello world!\n");
    interval = HZ / freq;

    struct file* file = file_open("/proc/ensea/chenille", 0, 0);
    unsigned char* data = kmalloc(9 * sizeof(unsigned char));
    file_read(file, 0, data, 9);
    for (size_t i = 0; i < 8; i++)
    {
        basePattern |= (data[i] - '0') << (28 - 4 * i);
    }
    file_close(file);

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
