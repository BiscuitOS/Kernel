/*
 * Interrupt 11: Segment Not Present (#NP) when moving ES
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
#include <linux/mm.h>
#include <asm/system.h>

/*
 * Trigger interrupt 11: MOV a selector that not marked present to ES
 *   Copies the second operand (source operand) to the first operand
 *   (destination operand). The source operand can be an immediate
 *   value. general-purpos register, segment register, or memory location.
 *   
 *   IF ES is loadded with non-NULL selector
 *   THEN
 *     IF segment not marked present
 *       THEN #NP (selector);
 *   FI
 */
void trigger_interrupt11(void)
{
    unsigned char es_sel = 0x2;
    unsigned char nr_ldt = 0x2;
    unsigned char dpl = 0x00;
    struct desc_struct *desc;
    struct desc_struct *ldt;
    struct desc_struct *ds_desc;

    /* Establish A dummy Data segment on LDT */
    ldt = (struct desc_struct *) get_free_page();
    set_ldt_desc(gdt + (nr_ldt << 1) + FIRST_LDT_ENTRY, ldt);

    /* Establish a dummy segment selector for dummy data segment that
     * points to LDT
     */
    es_sel = (es_sel << 3) | dpl | (1 << 2);

    /* Obtain specific Data segment descriptor on LDT */    
    desc = ldt + (es_sel >> 3);

    /* Initialze Dummy Data segment descriptor as kernel Data segment */
    ds_desc = gdt + 2;
    desc->a = ds_desc->a;
    desc->b = ds_desc->b;

    /* Clear preset bit on segment descriptor */
    desc->b &= 0xFFFF7FFF;

    __asm__ ("movl %%eax, %%es"
             :: "a" (es_sel));
}
