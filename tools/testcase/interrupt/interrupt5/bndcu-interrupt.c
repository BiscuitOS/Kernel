/*
 * Interrupt 5 (#BR): Trigger #BR on BNDCU/BNDCN instruction
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/* 
 * Trigger interrupt 5 (#BR): BNDCU -- Check Upper Bound
 *   Compare the address in the second operand with the upper bound in
 *   bnd. The second operand can be either a register or a memory 
 *   operand. If the address is higher than the upper bound in bnd.UB,
 *   it will set BNDSTATUS to 01H and signal a #BR exception.
 *
 *   BNDCU perform 1's complement operation on the upper bound of bnd
 *   first before proceeding with address comparesion. BNDCN perform
 *   address comparison directly using the upper bound in bnd that is
 *   already reverted out of 1's complement form.
 *
 *   This instruction does not cause any memory access, and does not
 *   read or write any flags. Effective address computation of m32/64
 *   has identical behavior to LEA.
 */
void trigger_interrupt5(void)
{
    int BND[2] = { 0, 6 };
    int index = 7; /* safe value: 0, 1, 2, 3, 4, 5, 6 */

    printk("Trigger interrupt 5 (#BR): BNDCU/BNDCN.\n");
    /*
     * Upper = BND[1]
     * lower = BND[0]
     * BNDCU BND, reg 
     * IF reg > NOT(BND.UB) Then
     *     BNDSTATUS <- 0x01H
     *     #BR
     * FI
     *
     * or
     *
     * BNDCL BND, mem
     * TEMP <- LEA(mem)
     * IF TEMP > NOT(BND.UB) Then
     *     BNDSTATUS <- 0x01H
     *     #BR
     * FI
     *
     * or
     * 
     * BNDCN BND, reg
     * IF reg > BND.UB Then
     *     BNDSTATUS <- 0x01H
     *     #BR
     * FI
     *
     * or
     *
     * BNDCL BND, mem
     * TEMP <- LEA(mem)
     * IF TEMP > BND.UB Then
     *     BNDSTATUS <- 0x01H
     *     #BR
     * FI
     *
     * or
     */
    __asm__ ("lea %0, %%edx\n\t"
             "movl %1, %%eax\n\t"
             "bndcu %%eax, (%%edx)"
             :: "m" (BND), "m" (index));
}
