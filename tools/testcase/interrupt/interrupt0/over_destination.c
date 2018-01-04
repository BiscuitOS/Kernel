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
 * Tarigger Interrupt 0: Overflow EAX/AX
 * EAX or AX can't store a rightful result-value when 
 * execute a divide operation, eg:
 * _rax / 0x01 = 0xFFF, and 'AL' can't store result '0xFFF'
 */
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
