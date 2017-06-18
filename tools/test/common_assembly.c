#include <linux/kernel.h>

#ifdef CONFIG_TESTCODE

/*
 * Call in C function on assmebly.
 * Push all arguments into satack from right to left,
 * eg. push 'b' first and 'a' next. when return from c function,
 * add vlaue of esp (nr_argument * bit_length).
 */
static void C_function(int a, int b)
{
	printk("A is %d B is %d\n", a, b);	
}

void test_call_c_in_assembly(void)
{
	unsigned int a = 20;
	unsigned int b = 30;

	__asm__("pushl %%esp\n\t"
			"pushl %%ebx\n\t"
			"pushl %%eax\n\t"
			"call C_function\n\t"
			"addl $8, %%esp\n\t"
			"popl %%esp"
			:: "a" (a), "b" (b));	
	/* Avoid warning */
	C_function(b, a);
}

/*
 * Get return address from call-function.
 */
void test_get_c_return_address(void)
{
	unsigned int __res;

	__asm__("pushl %%esp\n\t"
			"pushl %%eax\n\t"
			"lea 36(%%esp), %%eax\n\t"
			"movl %%eax, %0\n\t"
			"popl %%eax\n\t"
			"popl %%esp"
			: "=m" (__res) :);
	printk("Call-return address %#x\n", __res);
}


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

/*
 * Common setup EFLAGS
 */
void test_set_EFLAGS(void)
{
	unsigned int _eax = 0;
	unsigned int _ebx = 0;

	__asm__ ("pushfl\n\r"
             "movl (%%esp), %%eax\n\r"
             "andl $0xFFFFFF00, (%%esp)\n\r"
             "movl (%%esp), %%ebx\n\r"
             "popfl\n\r"
			 "movl %%eax, %0\n\r"
             "movl %%ebx, %1"
             :: "m" (_eax), "m" (_ebx));
	printk("Before EFALGS %#x\nAfter EFLAGS %#x\n",
			_eax, _ebx);
}
#endif
