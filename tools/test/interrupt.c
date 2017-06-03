#include <linux/kernel.h>

#ifdef CONFIG_TESTCODE

/*
 * Test interrupt 0 - divide zero
 */
void test_interrupt0_divide_error(void)
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

/*
 * Test interrupt 1 - debug
 */
void test_interrupt1_debug(void)
{
	/* case 0 */
	/*
	 * Set TF on EFLAGS will invoke interrupt 1 (debug).
	 */
	__asm__("pushl %%eax\n\t"
			"pushf\n\t"
			"movl %%esp, %%eax\n\t"
			"orl $0x0100, (%%eax)\n\t" // set TF bit.
			"popf\n\t"
			"popl %%eax"
			::);
}

/*
 * Test Interrupt 3 - int3 
 */
void test_interrupt3_int3(void)
{
	/* general interrupt entry */
	__asm__("pushl %%eax\n\t"
			"pushf\n\t"
			"movl %%esp, %%eax\n\t"
			"orl $0x0100, (%%eax)\n\t"
			"popf\n\t"
			"popl %%eax"
			::);	
}

/*
 * Test Interrupt 4 - overflow error
 * The type 4 interrupt is dedicated to handle overflow conditions. 
 * There are two ways by which a type 4 interrupt can be generated: 
 * either by 'int4' or by 'into' . Like the breakpoint interrupt, 
 * 'into' requires only one byte to encode, as it does not require 
 * the specification of the interrupt type number as part of the 
 * instruction. Unlike 'int4', which unconditionally generates a 
 * type 4 interrupt, 'into' generates a type 4 interrupt only if the
 * overflow flag is set. We do not normally use 'into' , as the 
 * overflow condition is usually detected and processed by using 
 * the conditional jump instructions 'jo' and 'jno'.
 */
void test_interrupt4_overflow(void)
{
#ifndef CONFIG_SOFT_INTERRUPT
	/* case 0 */
	/* 
	 * 'OF' set and call 'into'
	 */
	__asm__("pushl %%ebx\n\t"
			"movb $0x7f, %%bl\n\t"
			"addb $10, %%bl\n\t"
			"into\n\t"
			"popl %%ebx"
			::);
#else
	/* case 1 */
	__asm__("int $4");
#endif
}

#endif
