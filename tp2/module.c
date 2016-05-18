#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <mach/platform.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charlie, 2015");
MODULE_DESCRIPTION("Module, aussitot insere, aussitot efface");
static int LED;
static int major;

module_param(LED, int, 0);
MODULE_PARM_DESC(LED, "Nombre de led");

static const int LED0 = 2;


//------------------------------------------------------------------------------
//                                GPIO ACCES
//------------------------------------------------------------------------------

struct gpio_s
{
    uint32_t gpfsel[7];
    uint32_t gpset[3];
    uint32_t gpclr[3];
    uint32_t gplev[3];
    uint32_t gpeds[3];
    uint32_t gpren[3];
    uint32_t gpfen[3];
    uint32_t gphen[3];
    uint32_t gplen[3];
    uint32_t gparen[3];
    uint32_t gpafen[3];
    uint32_t gppud[1];
    uint32_t gppudclk[3];
    uint32_t test[1];
}
*gpio_regs = (struct gpio_s *)__io_address(GPIO_BASE);;

enum {FUN_INPUT, FUN_OUTPUT};

static void gpio_fsel(int pin, int fun)
{
    uint32_t reg = pin / 10;
    uint32_t bit = (pin % 10) * 3;
    uint32_t mask = 0b111 << bit;
    gpio_regs->gpfsel[reg] = (gpio_regs->gpfsel[reg] & ~mask) | ((fun << bit) & mask);
}

static void gpio_write(int pin, bool val)
{
    if (val)
        gpio_regs->gpset[pin / 32] = (1 << (pin % 32));
    else
        gpio_regs->gpclr[pin / 32] = (1 << (pin % 32));
}

static int 
open_ledbtn(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "gallepe open()\n");
    return 0;
}

static ssize_t 
read_ledbtn(struct file *file, char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "gallepe read()\n");
    return count;
}

/*static ssize_t 
write_ledbtn(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    printk(KERN_DEBUG "write()\n");
    return count;
}
*/

static int 
release_ledbtn(struct inode *inode, struct file *file) {
    printk(KERN_DEBUG "gallepe close()\n");
    return 0;
}

struct file_operations fops_driver =
{
    .open 	= open_ledbtn,
    .read 	= read_ledbtn,
    .write 	= gpio_write,
    .release	= release_ledbtn 
};

//------------------------------------------------------------------------------
//                             TIMER PROGRAMMING 
//------------------------------------------------------------------------------

static struct timer_list led_blink_timer;
static int led_blink_period = 1000;

static void led_blink_handler(unsigned long unused)
{
    static bool on = false;
    on = !on;
    gpio_write(LED0, on);
    mod_timer(&led_blink_timer, jiffies + msecs_to_jiffies(led_blink_period));
}


static int __init LedBlinkModule_init(void)
{
    int result;
    printk(KERN_DEBUG "gallepe init()\n");

	register_chrdev(0, "ledbp" &fops_ledbp);

    gpio_fsel(LED0, FUN_OUTPUT); 
    gpio_write(LED0, 1);
    setup_timer(&led_blink_timer, led_blink_handler, 0);
    result = mod_timer(&led_blink_timer, jiffies + msecs_to_jiffies(led_blink_period));
    BUG_ON(result < 0);
    return 0;
}

static void __exit LedBlinkModule_exit(void)
{
    gpio_fsel(LED0, FUN_INPUT); 
    del_timer(&led_blink_timer);

	unregister_chrdev(major, "ledbp");
}

module_init(LedBlinkModule_init);
module_exit(LedBlinkModule_exit);
