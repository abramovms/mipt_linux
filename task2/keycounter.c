#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/irq.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Misha Stryukov");
MODULE_VERSION("0.1");

static const unsigned int PS2_IRQ = 1;
static const unsigned long minute = 60000;

static const unsigned int KEY_PRESSED = 0x80;
static const unsigned int PORT = 0x60;

static atomic64_t counter;
static struct timer_list timer;

void timer_callback(struct timer_list* timer) {
    pr_info("%lld symbols per minute\n", atomic64_xchg(&counter, 0));
    mod_timer(timer, jiffies + msecs_to_jiffies(minute));
}

irqreturn_t int_call(int irq, void* dev_id) {
    if ((PORT & KEY_PRESSED) == 0) {
            atomic64_add(1, &counter);
    }
    return IRQ_HANDLED;
}

static int __init kc_init(void) {
    atomic64_set(&counter, 0);
    timer_setup(&timer, (void*)timer_callback, 0);
    mod_timer(&timer, jiffies + msecs_to_jiffies(minute));

    int error = request_irq(PS2_IRQ,
                          (void*)int_call,
                          IRQF_SHARED,
                          "key_counter",
                          (void*)int_call);
    if (error) {
            return 1;
    }

    return 0;
}
static void __exit kc_exit(void) {
    free_irq(PS2_IRQ, (void*)int_call);
    timer_shutdown_sync(&timer);
}

module_init(kc_init);
module_exit(kc_exit);
