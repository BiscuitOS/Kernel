/*
 * Interrupt 0: divide zero
 *
 * (C) 2018.1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

/*
 * Trigger interrupt 0 - divide zero
 * The processor generates a type 0 interrupt whenever executing a divide
 * instruction—either 'div' (divide) or 'idiv' (integer divide)—results in 
 * a quotient that is larger than the destination specified. The default 
 * interrupt handler on Linux displays a Floating point exception message
 * and terminates the program.
 */
void trigger_interrupt0(void)
{
    int a;
    int b = 0;

    printk("Trigger interrupt 0: DIV or IDIV is zero\n");

    /* divide error --> interrupt0 */
    a = 3 / b;
    b = a;
}
