/*
 * Interrupt 3: Trigger Breakpoint by setting TF on Eflags
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* trigger interrupt 3: set TF bit 
 * When TF on EFLAGS is set, the system will trigger interrupt 3,
 * this interrupt is named 'break point'.
 */
void trigger_interrupt3(void)
{
    printk("Test interrupt 3: Set TF bit on EFLAGS.\n");
    /* general interrupt entry */
    __asm__ ("pushl %%eax\n\t"
             "pushf\n\t"
             "movl %%esp, %%eax\n\t"
             "orl $0x0100, (%%eax)\n\t"
             "popf\n\t"
             "popl %%eax"
             ::);
}
