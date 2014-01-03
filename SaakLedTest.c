#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for KERN_INFO */
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#define DRIVER_AUTHOR "Saak Dertadian <s.dertadian@gmail.com>"
#define DRIVER_DESC   "First Led Test Driver"
MODULE_LICENSE("GPL");

#define TIMER_PERIOD	250	// * 1 ms
unsigned int LED_GPIO_1 = 60;	// GPIO1_28 = (1x32) + 28 = 60
unsigned int LED_GPIO_2 = 48;	// GPIO1_16 = (1x32) + 26 = 48
unsigned int BP_GPIO = 49;	// GPIO1_17 = (1x32) + 17 = 49

static struct timer_list my_timer;
int tt,flagBp;
int irq_line;

irqreturn_t ButtonInterrupt(int irq,void *dev_id,struct pt_regs *regs)
{
	flagBp=1;
	return IRQ_HANDLED;
}


void my_timer_callback( unsigned long data )
{
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMER_PERIOD));
	tt++;
//	printk(KERN_EMERG "Hello: tt=%d\n",tt);
	gpio_set_value(LED_GPIO_1,tt&0x01);
	if(flagBp)
	{
		flagBp=0;
		gpio_set_value(LED_GPIO_2,1);
	}
	else
		gpio_set_value(LED_GPIO_2,0);
}

int init_module(void)
{
	int ret;
	int irq_req_res;

	tt=0;
	flagBp=0;

	printk(KERN_EMERG "Init SaakLedTest >%d<.\n",tt);

	ret=gpio_request(LED_GPIO_1,"Led 1");
	if(ret)	printk(KERN_EMERG "Error: gpio_request: %d\n",LED_GPIO_1);
	ret=gpio_request(LED_GPIO_2,"Led 2");
	if(ret)	printk(KERN_EMERG "Error: gpio_request: %d\n",LED_GPIO_2);
	ret=gpio_request(BP_GPIO,"Interrupt BP");
	if(ret)	printk(KERN_EMERG "Error: gpio_request: %d\n",BP_GPIO);

	gpio_direction_output(LED_GPIO_1,1);
	gpio_direction_output(LED_GPIO_2,1);
	gpio_set_value(LED_GPIO_1,0);
	gpio_set_value(LED_GPIO_2,0);

	ret=gpio_direction_input(BP_GPIO);
	if(ret)	printk(KERN_EMERG "Error: gpio_direction_input: %d\n",BP_GPIO);
	if((irq_line=gpio_to_irq(BP_GPIO))<0)
	{
		printk(KERN_EMERG "Gpio %d cannot be used as interrupt",BP_GPIO);
		return(0);
	}
	irq_req_res=request_irq(irq_line,(void*)ButtonInterrupt,IRQF_TRIGGER_FALLING,"BP Int",NULL);
	if(irq_req_res)
	{
		if(irq_req_res==-EBUSY)
			ret=irq_req_res;
		else
			ret=-EINVAL;
	}

	/* setup your timer to call my_timer_callback */
	setup_timer(&my_timer, my_timer_callback, 0);
	/* setup timer interval to 1000 msecs */
	ret=mod_timer(&my_timer, jiffies + msecs_to_jiffies(TIMER_PERIOD));
	if(ret)	printk("Error in mod_timer\n");

	return 0;
}

void cleanup_module(void)
{
	/* remove kernel timer when unloading module */
	del_timer(&my_timer);

	free_irq(irq_line,NULL);
	gpio_free(BP_GPIO);
	gpio_free(LED_GPIO_1);
	gpio_free(LED_GPIO_2);

	printk(KERN_EMERG "Goodbye world.\n");
}
