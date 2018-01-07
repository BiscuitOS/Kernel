/*
 * Interrupt 5 (#BR): Trigger #BR on BNDCL instruction
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* 
 * Trigger interrupt 5 (#BR): BNDCL -- Check lower Bound
 *   Compare the address in the second operand with the lower bound in
 *   bnd. The second operand can be either a register or memory operand.
 *   If the address is lower than the lower bound in bnd.LD, it will
 *   set BNDSTATUS to 01H and signal a #BR exception.
 *
 *   This instruction does not cause any memory access, and does not
 *   read or write any flags.
 */
void trigger_interrupt5(void)
{
    int BND[2] = { 0, 6 };
    int index = -2; /* safe value: 0, 1, 2, 3, 4, 5, 6 */

    printk("Trigger interrupt 5 (#BR): BNDCL.\n");
    /*
     * Upper = BND[1]
     * lower = BND[0]
     * BNDCL BND, reg 
     * IF reg < BND.LB Then
     *     BNDSTATUS <- 0x01H
     *     #BR
     * FI
     *
     * or
     *
     * BNDCL BND, mem
     * TEMP <- LEA(mem)
     * IF TEMP < BND.LB Then
     *     BNDSTATUS <- 0x01H
     *     #BR
     * FI
     */
    __asm__ ("lea %0, %%edx\n\t"
             "movl %1, %%eax\n\t"
             "bndcl %%eax, (%%edx)"
             :: "m" (BND), "m" (index));
}
