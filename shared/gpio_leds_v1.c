#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/buffer_head.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>

#define DRIVER_AUTHOR "Ovazza - Kossyfidis"
#define DRIVER_DESC "Led Module"
#define DRIVER_LICENSE "GPL"
#define DRIVER_VERSION "1.0"

#define SIZE 4

static struct timer_list timer;
static unsigned long interval;
static unsigned long base = 0x01;
static unsigned long pattern = 0;
static unsigned long direction = 0;
static int freq = 1;
struct ensea_leds_dev *last_dev = NULL; 

char message[1024];

module_param(freq, int, 0); 

// Directories
static struct proc_dir_entry *ensea_proc_dir;
static struct proc_dir_entry *speed_proc_entry;
static struct proc_dir_entry *dir_proc_entry;

// Prototypes
static int leds_probe(struct platform_device *pdev);
static int leds_remove(struct platform_device *pdev);
static ssize_t leds_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t leds_write(struct file *file, const char *buffer, size_t len, loff_t *offset);
static ssize_t read_dir(struct file *file, char *buffer, size_t len, loff_t *ppos);
static ssize_t write_dir(struct file *file, const char *buffer, size_t len, loff_t *ppos);
static ssize_t read_speed(struct file *file, char *buffer, size_t len, loff_t *ppos);
static ssize_t write_speed(struct file *file, const char *buffer, size_t len, loff_t *ppos);

// An instance of this structure will be created for every ensea_led IP in the system
struct ensea_leds_dev
{
    struct miscdevice miscdev;
    void __iomem *regs;
    u8 leds_value;
};

// Specify which device tree devices this driver supports
static struct of_device_id ensea_leds_dt_ids[] = {
    {.compatible = "dev,ensea"},
    {/* end of table */}};

// Inform the kernel about the devices this driver supports
MODULE_DEVICE_TABLE(of, ensea_leds_dt_ids);

// Data structure that links the probe and remove functions with our driver
static struct platform_driver leds_platform = {
    .probe = leds_probe,
    .remove = leds_remove,
    .driver = {
        .name = "Ensea LEDs Driver",
        .owner = THIS_MODULE,
        .of_match_table = ensea_leds_dt_ids}};

static const struct file_operations dir_proc_fops = {
    .owner = THIS_MODULE,
    .read = read_dir,
    .write = write_dir};

ssize_t read_dir(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    int copy;
    if (count > SIZE)
        count = SIZE;

    if (access_ok(VERIFY_READ, buffer, count))
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

    return count - copy;
}

ssize_t write_dir(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    int len = count;
    if (len > SIZE)
        len = SIZE;

    if (access_ok(VERIFY_WRITE, buffer, count))
    {
        if (copy_from_user(message, buffer, count))
        {
            printk("Error copy to user\n");
            return -EFAULT;
        }
        message[count] = '\0';
        printk("Copy from user ok\n");
        printk("Message : %s\n", message);

        memcpy(&direction, message, SIZE);
        direction &= 0x01;
        printk("Direction : %ld\n", direction);
    }
    else
    {
        printk("Error Acces_Ok Write\n");
        return -EFAULT;
    }

    return count;
}

static const struct file_operations speed_proc_fops = {
    .owner = THIS_MODULE,
    .read = read_speed,
    .write = write_speed};

ssize_t read_speed(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
    int copy;
    if (count > SIZE)
        count = SIZE;

    if (access_ok(VERIFY_READ, buffer, count))
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

    return count - copy;
}

ssize_t write_speed(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
    int len = count;
    if (len > SIZE)
        len = SIZE;

    if (access_ok(VERIFY_WRITE, buffer, count))
    {
        if (copy_from_user(message, buffer, count))
        {
            printk("Error copy to user\n");
            return -EFAULT;
        }
        message[count] = '\0';
        printk("Copy from user ok\n");
        printk("Message : %s\n", message);

        memcpy(&freq, message, SIZE);
        interval = HZ / freq;
    }
    else
    {
        printk("Error Acces_Ok Write\n");
        return -EFAULT;
    }

    return count;
}

// The file operations that can be performed on the ensea_leds character file
static const struct file_operations ensea_leds_fops = {
    .owner = THIS_MODULE,
    .read = leds_read,
    .write = leds_write};

// This function gets called whenever a read operation occurs on one of the character files
static ssize_t leds_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    int success = 0;

    /*
     * Get the ensea_leds_dev structure out of the miscdevice structure.
     *
     * Remember, the Misc subsystem has a default "open" function that will set
     * "file"s private data to the appropriate miscdevice structure. We then use the
     * container_of macro to get the structure that miscdevice is stored inside of (which
     * is our ensea_leds_dev structure that has the current led value).
     *
     * For more info on how container_of works, check out:
     * http://linuxwell.com/2012/11/10/magical-container_of-macro/
     */
    struct ensea_leds_dev *dev = container_of(file->private_data, struct ensea_leds_dev, miscdev);

    // Give the user the current led value
    success = copy_to_user(buffer, &base, sizeof(base));

    // If we failed to copy the value to userspace, display an error message
    if (success != 0)
    {
        pr_info("Failed to return current led value to userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    }

    return 0; // "0" indicates End of File, aka, it tells the user process to stop reading
}

// This function gets called whenever a write operation occurs on one of the character files
static ssize_t leds_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    int success = 0;

    /*
     * Get the ensea_leds_dev structure out of the miscdevice structure.
     *
     * Remember, the Misc subsystem has a default "open" function that will set
     * "file"s private data to the appropriate miscdevice structure. We then use the
     * container_of macro to get the structure that miscdevice is stored inside of (which
     * is our ensea_leds_dev structure that has the current led value).
     *
     * For more info on how container_of works, check out:
     * http://linuxwell.com/2012/11/10/magical-container_of-macro/
     */
    struct ensea_leds_dev *dev = container_of(file->private_data, struct ensea_leds_dev, miscdev);

    // Get the new led value (this is just the first byte of the given data)
    success = copy_from_user(&dev->leds_value, buffer, sizeof(dev->leds_value));

    // If we failed to copy the value from userspace, display an error message
    if (success != 0)
    {
        pr_info("Failed to read led value from userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    }
    else
    {
        // We read the data correctly, so update the LEDs pattern
        pattern = dev->leds_value;
        base = dev->leds_value;
        // iowrite32(pattern, dev->regs);
    }

    // Tell the user process that we wrote every byte they sent
    // (even if we only wrote the first value, this will ensure they don't try to re-write their data)
    return len;
}

// Called whenever the kernel finds a new device that our driver can handle
// (In our case, this should only get called for the one instantiation of the Ensea LEDs module)
static int leds_probe(struct platform_device *pdev)
{
    int ret_val = -EBUSY;
    struct ensea_leds_dev *dev;
    struct resource *r = 0;

    pr_info("leds_probe enter\n");

    // Get the memory resources for this LED device
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (r == NULL)
    {
        pr_err("IORESOURCE_MEM (register space) does not exist\n");
        goto bad_exit_return;
    }

    // Create structure to hold device-specific information (like the registers)
    dev = devm_kzalloc(&pdev->dev, sizeof(struct ensea_leds_dev), GFP_KERNEL);

    // Both request and ioremap a memory region
    // This makes sure nobody else can grab this memory region
    // as well as moving it into our address space so we can actually use it
    dev->regs = devm_ioremap_resource(&pdev->dev, r);
    if (IS_ERR(dev->regs))
        goto bad_ioremap;

    // Turn the LEDs on (access the 0th register in the ensea LEDs module)
    dev->leds_value = 0x01;
    iowrite32(dev->leds_value, dev->regs);

    // Initialize the misc device (this is used to create a character file in userspace)
    dev->miscdev.minor = MISC_DYNAMIC_MINOR; // Dynamically choose a minor number
    dev->miscdev.name = "ensea_leds";
    dev->miscdev.fops = &ensea_leds_fops;

    ret_val = misc_register(&dev->miscdev);
    if (ret_val != 0)
    {
        pr_info("Couldn't register misc device :(");
        goto bad_exit_return;
    }
    last_dev = dev;
    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void *)dev);

    pr_info("leds_probe exit\n");

    return 0;

bad_ioremap:
    ret_val = PTR_ERR(dev->regs);
bad_exit_return:
    pr_info("leds_probe bad exit :(\n");
    return ret_val;
}

// Gets called whenever a device this driver handles is removed.
// This will also get called for each device being handled when
// our driver gets removed from the system (using the rmmod command).
static int leds_remove(struct platform_device *pdev)
{
    // Grab the instance-specific information out of the platform device
    struct ensea_leds_dev *dev = (struct ensea_leds_dev *)platform_get_drvdata(pdev);

    pr_info("leds_remove enter\n");

    // Turn the LEDs off
    iowrite32(0x00, dev->regs);

    // Unregister the character file (remove it from /dev)
    misc_deregister(&dev->miscdev);

    pr_info("leds_remove exit\n");

    return 0;
}

static void mytimer(unsigned long data)
{
    //printk("0x%08lx\n", pattern);
    if (direction)
    {
        if (pattern < 512)
        {
            pattern = pattern << 1;
        }
        else
        {
            pattern = last_dev->leds_value;
        }
    }
    else
    {
        if (pattern > 0)
        {
            pattern = pattern >> 1;
        }
        else
        {
            pattern = last_dev->leds_value << 4;
        }
    }

    iowrite32(pattern, last_dev->regs);
    mod_timer(&timer, jiffies + interval);
}

// Called when the driver is installed
static int leds_init(void)
{
    int ret_val = 0;
    pr_info("Initializing the Ensea LEDs module\n");

    // Register our driver with the "Platform Driver" bus
    ret_val = platform_driver_register(&leds_platform);
    if (ret_val != 0)
    {
        pr_err("platform_driver_register returned %d\n", ret_val);
        return ret_val;
    }

    pr_info("Ensea LEDs module successfully initialized!\n");

    ensea_proc_dir = proc_mkdir("ensea", NULL);
    speed_proc_entry = proc_create("speed", 666, ensea_proc_dir, &speed_proc_fops);
    dir_proc_entry = proc_create("dir", 666, ensea_proc_dir, &dir_proc_fops);

    printk(KERN_INFO "Proc directories ok !\n");
    interval = HZ / freq;

    init_timer(&timer);
    timer.function = mytimer;
    timer.data = 0;
    mod_timer(&timer, jiffies + interval);

    return 0;
}

// Called when the driver is removed
static void leds_exit(void)
{
    pr_info("Ensea LEDs module exit\n");

    // Unregister our driver from the "Platform Driver" bus
    // This will cause "leds_remove" to be called for each connected device
    platform_driver_unregister(&leds_platform);

    pr_info("Ensea LEDs module successfully unregistered\n");

    remove_proc_entry("speed", ensea_proc_dir);
    remove_proc_entry("dir", ensea_proc_dir);
    remove_proc_entry("ensea", NULL);

    del_timer(&timer);
    pr_info("Proc directories + timer successfully deleted\n");
}

// Tell the kernel which functions are the initialization and exit functions
module_init(leds_init);
module_exit(leds_exit);

// Define information about this kernel module
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
