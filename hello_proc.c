#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

#define DRIVER_AUTHOR "Quentin Souvignet et Antoine Coutant"
#define DRIVER_DESC "Hello proc Module"
#define DRIVER_LICENSE "GPL"

static struct proc_dir_entry *proc_entry;

static ssize_t hello_proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_INFO "Hello!\n");
    return 0;
}

static const struct file_operations hello_proc_fops = {
    .owner = THIS_MODULE,
    .read = hello_proc_read,
};

int hello_init(void)
{
    proc_entry = proc_create("hello_proc", 0, NULL, &hello_proc_fops);
    if (proc_entry == NULL) {
        printk(KERN_ERR "Error creating proc entry\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "Hello world!\n");
    return 0;
}

void hello_exit(void)
{
    remove_proc_entry("hello_proc", NULL);
    printk(KERN_ALERT "Bye bye...\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
