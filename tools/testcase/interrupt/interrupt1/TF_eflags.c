/*
 * Interrupt 1: trigger by setting TF flag on EFLAGS
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
 * Trigger interrupt 1: set eflags TF bit
 * To start single-stepping, the trap flag (TF) bit in the flags register
 * should be set (i.e., TF = 1). When TF is set, the CPU automatically 
 * generates a type 1 interrupt after executing each instruction. 
 * Some exceptions do exist, but we do not worry about them here.
 */
void trigger_interrupt1(void)
{
    printk("Test interrupt 1: Set TF on Eflags.\n");

    /* Set TF on EFLAGS will invoke interrupt1 (debug) */
    __asm__ ("pushl %%eax\n\t"
             "pushf\n\t"
             "movl %%esp, %%eax\n\t"
             "orl $0x0100, (%%eax)\n\t"   // set TF bit.
             "popf\n\t"
             "popl %%eax"
             ::);
}
