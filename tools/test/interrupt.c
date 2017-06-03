#include <linux/kernel.h>

#ifdef CONFIG_TESTCODE

void test_divide_error(void)
{
#ifdef CONFIG_DIVIDE_ERROR0
	int a;
	int b = 0;
#else
	unsigned short _rax = 0xFFF;
	unsigned short _res;
#endif
	
#ifdef CONFIG_DIVIDE_ERROR0
	/* case 0 */
	/* divide error -> interrupt0 */
	a = 3 / b;	
	b = a;
#else
	/* case 1 */
	/* 
	 * EAX or AX can't store a rightful result-value 
	 * eg, _rax / 0x01 = 0xFFF, and "AL" can't store result '0xFFF'. 
	 */

	__asm__("mov %1, %%ax\n\t"
			"movb $0x1, %%bl\n\t"
			"div %%bl\n\t"
			"movb %%al, %0"
			: "=m" (_res) : "m" (_rax));
	printk("The result %#x\n", _res);
#endif
}

#endif
