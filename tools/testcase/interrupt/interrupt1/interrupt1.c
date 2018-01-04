/*
 * interrupt 1: debug #DB
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

/*
 * interrupt 1 - debug #DB
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

void common_interrupt1(void)
{
    trigger_interrupt1();
}
