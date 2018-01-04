/*
 * interrupt 1: debug
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * Test interrupt 1 - debug
 * Single-Step Interrupt Single-stepping is a useful debugging tool to
 * observe the behavior of a program instruction by instruction. To start
 * single-stepping, the trap flag (TF) bit in the flags register should 
 * be set (i.e., TF = 1). When TF is set, the CPU automatically generates
 * a type 1 interrupt after executing each instruction. Some exceptions
 * do exist, but we do not worry about them here.
 * The interrupt handler for the type 1 interrupt can be used to display 
 * relevant information about the state of the program. For example, 
 * the contents of all registers could be displayed.
 * To end single stepping, the TF should be cleared. The instruction set, 
 * however, does not have instructions to directly manipulate the TF bit. 
 * Instead, we have to resort to an indirect means. You have to push flags
 * register using pushf and manipulate the TF bit and use popf to store this
 * value back in the flags register. Here is an example code fragment that
 * sets the trap flag.
 */

/* trigger interrupt 1 by set eflags TF bit */
//#define INT1_EFLAGS_TF       0x01

/* trigger interrupt 1 by 'int 1' */
#define INT1_SOFTINT      0x01

/*
 * Trigger interrupt 1: set eflags TF bit
 * To start single-stepping, the trap flag (TF) bit in the flags register
 * should be set (i.e., TF = 1). When TF is set, the CPU automatically 
 * generates a type 1 interrupt after executing each instruction. 
 * Some exceptions do exist, but we do not worry about them here.
 */
#ifdef INT1_EFLAGS_TF
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
#endif

/*
 * trigger interrupt 1 by 'int 0x1'
 * The interrupt 1 will be general when invoke soft-interrupt 'int $0x1'
 * Note! whatever interrupt is enable or disable, 'int $0x1' must trigger
 * interrupt 1.
 */
#ifdef INT1_SOFTINT
void trigger_interrupt1(void)
{
    printk("Test interrupt 1: debug ['int $0x1']\n");
    __asm__("int $0x1");
}
#endif
