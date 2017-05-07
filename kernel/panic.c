

/*
 * This function is used through-out the kernel (include mm and fs)
 * to indicate a major problem.
 */
#define PANIC

#include <linux/kernel.h>
#include <linux/sched.h>

void sys_sync(void)
{
}

void panic(const char *s)
{
	printk("Kernel panic: %s\n\r", s);
	if (current == task[0])
		printk("In swapper task - not syncing\n\r");
	else
		sys_sync();
	for(;;);
}
