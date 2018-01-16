/*
 * Linear Address Machanism on MMU
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/sched.h>

#include <test/debug.h>

static char *var = "BiscuitOS";
/*
 * Convent logic address to linear address
 */
static void logic_to_linear(void)
{
    struct logic_addr la;
    unsigned long dpl, cpl;
    unsigned long cs, ds;
    struct desc_struct *desc;
    unsigned long base, limit;
    unsigned long linear;

    __asm__ ("movl %%ds, %0\n\r"
             "movl %%cs, %1" 
             : "=r" (ds), "=r" (cs));
    /* Uses the offset in the segment selector to locate the segment
     * descriptor for the segment in the GDT or LDT and reads it into
     * the processor. (This step is needed only when a new segment
     * selector is loaded into a segment register) */
    la.offset = (unsigned long)&var;
    la.sel = ds;
    if ((la.sel >> 2) & 0x1) {
        /* Segment descriptor locate in LDT */
        desc  = &current->ldt[la.sel >> 3];
        base  = get_base(*desc);
        limit = get_limit(*desc);
    } else {
        /* Segment descriptor locate in GDT */
        desc  = &gdt[la.sel >> 3]; 
        base  = get_base(*desc);
        limit = get_limit(*desc);
    }

    /* Examines the segment descriptor to check the access rights and
     * range of the segment to insure that the segment is accessible 
     * and that the offset is within the limits of the segment. */
    limit *= 4096;
    if (la.offset > limit)
        panic("Out of range for segment\n");
    cpl = cs & 0x3;
    dpl = desc->b >> 13;
    if (cpl > dpl)
        panic("trigger #GP");

    /* Adds the base address of the segment from the segment descriptor
     * to the offset to form a linear address. */
    linear = la.offset + base;
    printk("Logical Address: %#x:%#x\n", la.sel, la.offset);
    printk("Linear  Address: %#x\n", linear);
}

/* common linear address entry */
int debug_linear_address_common(void)
{
    if (1) {
        logic_to_linear();
    } else {
        logic_to_linear();
    }
    return 0;
}
