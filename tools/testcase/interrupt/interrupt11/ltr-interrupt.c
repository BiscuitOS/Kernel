/*
 * Interrupt 11: Segment Not Present (#NP) on LTR instruction
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
 * Trigger interrupt 11: LTR - Load Task Register
 *   Loads the source operand into the segment selector field of the 
 *   task register. The source operand (a general-purpose regiseter or
 *   a memory location) contains a segment selector that points to a
 *   task state segment (TSS). After the segment selector is loaded in
 *   the task register, the processor uses the segment selector to 
 *   locate the segment descriptor for the TSS in the global descriptor
 *   table (GDT). It then loads the segment limit and base address for
 *   the TSS from the segment descriptor into the task register. The 
 *   task pointed to by the task register is marked busy, but a switch
 *   to the task does not occur.
 */
void trigger_interrupt11(void)
{
    unsigned short nr_tss = 1;
    unsigned char  dpl = 0x00;
    struct desc_struct *desc;

    /* Obtain specific segment descriptor for TSS */
    desc = gdt + (nr_tss << 1) + FIRST_TSS_ENTRY;

    /* Clear protect privilege level */
    desc->b &= 0xFFFF9FFF;
    /* Specify privilege for TSS */
    desc->b |= dpl << 13;
    /* Reload TR register */
    ltr(nr_tss);
}
