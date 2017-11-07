/*
 * Interrupt 0: divide zero
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <asm/system.h>

#include <test/testcase.h>

/* divide zero */
//#define INT_DIVIDE_ERROR       0x1

/* overflow for EAX/AX when execute divide */
//#define INT_OVERFLOW_EAX     0x1

/* software interrupt 0 */
#define INT0_SOFTINT     0x01

/*
 * Trigger interrupt 0 - divide zero
 * The processor generates a type 0 interrupt whenever executing a divide
 * instruction—either 'div' (divide) or 'idiv' (integer divide)—results in 
 * a quotient that is larger than the destination specified. The default 
 * interrupt handler on Linux displays a Floating point exception message
 * and terminates the program.
 */
#ifdef INT_DIVIDE_ERROR
void trigger_interrupt0(void)
{
    int a;
    int b = 0;

    printk("Test interrupt 0: divide error\n");

    /* divide error --> interrupt0 */
    a = 3 / b;
    b = a;
}
#endif

/*
 * Tarigger Interrupt 0: Overflow EAX/AX
 * EAX or AX can't store a rightful result-value when 
 * execute a divide operation, eg:
 * _rax / 0x01 = 0xFFF, and 'AL' can't store result '0xFFF'
 */
#ifdef INT_OVERFLOW_EAX
void trigger_interrupt0(void)
{
    unsigned short _rax = 0xfff;
    unsigned short _res;

    printk("Test interrupt 0: over EAX/AX after divide.\n");
    __asm__ ("mov %1, %%ax\n\t"
             "movb $0x01, %%bl\n\t"
             "div %%bl\n\t"
             "movb %%al, %0"
             : "=m" (_res) : "m" (_rax));
    printk("The result %#x\n", _res);
}
#endif

/*
 * Trigger interrupt 0: invoke 'int $0'
 * The system will trigger interrupt 0 when execute 'int 0'. 
 * Note! for this case, whatever interrupt is enable or disable,
 * 'int $0x0' will trigger interrupter 0.
 */
#ifdef INT0_SOFTINT
void trigger_interrupt0(void)
{
    printk("Test interrupt 0: divide zero [invoke 'int $0x0']\n");
    __asm__ ("int $0");
}
#endif
