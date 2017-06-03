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

void test_set_multi_register(void)
{
	unsigned int _eax = 0xFF;
	unsigned int _ebx = 0xEE;
	unsigned int _ecx = 0xAA;

	__asm__("movl %0, %%eax\n\t"
			"movl %1, %%ebx\n\t"
			"movl %2, %%ecx"
			:: "m" (_eax), "m" (_ebx), "m" (_ecx));
}

void test_set_and_get_register(void)
{
	unsigned int _eax = 0xFF;
	unsigned int _res;

	__asm__("movl %1, %%eax\n\t"
			"movl %%eax, %0"
			: "=m" (_res) : "m" (_eax));
	printk("%s get EAX %#x\n", __func__, _res);
}
#endif
