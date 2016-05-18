  

#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <mach/platform.h>

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

//------------------------------------------------------------------------------
//                              MODULE INIT & EXIT 
//------------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Franck from http://sysprogs.com/VisualKernel/tutorials/raspberry/leddriver/)");
MODULE_DESCRIPTION("leds on-off");
 
struct file_operations fops_driver =
{
    .open 	= open_ledbtn,
    .read 	= read_ledbtn,
    .write 	= write_ledbtn,
    .release	= release_ledbtn 
};


static int __init LedBlinkModule_init(void)
{
    int result;

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
}

module_init(LedBlinkModule_init);
module_exit(LedBlinkModule_exit);


