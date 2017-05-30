#include <linux/kernel.h>

#ifdef CONFIG_TESTCODE

void simple_inline_assembly(void)
{
	unsigned int __res;
	__asm__("movl %%eax, %0"
			: "=m" (__res) :);
	printk("%s EAX %#x\n",__func__, __res);
}

void test_get_eax(void)
{
	unsigned int __res;
	__asm__("movl %%eax, %0"
			: "=m" (__res) :);
	printk("%s EAX %#x\n", __func__, __res);
}

void test_get_ebx(void)
{
	unsigned int __res;
	__asm__("movl %%ebx, %0"
			: "=m" (__res) :);
	printk("%s EBX %#x\n", __func__, __res);
}

void test_multi_output(void)
{
	unsigned int _eax, _ebx, _ecx;
	__asm__("movl %%eax, %0\n\t"
			"movl %%ebx, %1\n\t"
			"movl %%ecx, %2"
			: "=m" (_eax), "=m" (_ebx), "=m" (_ecx) :);
	printk("%s>\nEAX %#x\nEBX %#x\nECX %#x\n",
		__func__, _eax, _ebx, _ecx);
}

void test_set_eax(void)
{
	unsigned int _eax = 0xFF;
	__asm__("movl %0, %%eax"
			:: "m" (_eax));
	printk("%s write %#x into EAX\n", __func__, _eax);
}

#endif
