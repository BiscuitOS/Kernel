/*
 * Interrupt 11: Segment Not Present (#NP) on LLDT instruction
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/head.h>

/*
 * Trigger interrupt 11: LLDT - Load Local Descriptor Table Register
 *   Loads the source operand into the segment selector field of the local
 *   descriptor table register (LDTR). The source operand (a general-
 *   purpose register or memory location) contains a segment selector that
 *   points to a local descriptor table (LDT). After the segment selector
 *   is loaded in the LDTR, the processor uses the segment selector to
 *   locate the segment descriptor for the LDT in the global descriptor
 *   table (GDT). It then loads the segment limit and base address for 
 *   the LDT from the segment descriptor into the LDTR. The segment
 *   register DS, ES, SS, FS, GS and CS are not affected by this instruction
 *   by this instruction, nor is the LDTR field in the task state 
 *   segment (TSS) for the current task.
 *
 *   If bit 2-15 of the source operand are 0, LDTR is marked invalid and 
 *   the LLDT instruction completes silently. However, all subsequent 
 *   references to descriptors in the LDT (except by the LAR, VERR, VERW
 *   or LSL instructions) cause a general protection exception (#GP).
 */
void trigger_interrupt11(void)
{
    unsigned char nr_ldt = 0;
    struct desc_struct *desc;

    /* Obtain specific segment descriptor for LDT */
    desc = gdt + (nr_ldt << 1) + FIRST_LDT_ENTRY;

    /* Clear preset bit on segment descriptor */
    desc->b &= 0xFFFF7FFF;

    /* load new LDT */
    lldt(nr_ldt);
}
