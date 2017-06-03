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

#endif
