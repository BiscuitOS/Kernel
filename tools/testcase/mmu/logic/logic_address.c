/*
 * Logical Address Machanism on MMU
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

static char var[50] = "BiscuitOS";
/*
 * Obtain vairable loagic address that from stack segment, 
 * code segment, data segment.
 */
static void obtain_logic_address(void)
{
    unsigned int a_var;
    unsigned int cs, ds, ss;
    struct logic_addr stack_logic, data_logic, code_logic;

    __asm__ ("movl %%cs, %0\n\r"
             "movl %%ds, %1\n\r"
             "movl %%ss, %2"
             : "=r" (cs), "=r" (ds), "=r" (ss));
    /* A logical address consists of a 16-bit segment selector and 
     * a 32-bit offset */
    /* The logic address on stack segment */
    stack_logic.offset = (unsigned long)&a_var;
    stack_logic.sel    = ss;
    /* The logic address on data segment */
    data_logic.offset = (unsigned long)&var;
    data_logic.sel    = ds;
    /* The logic address on code segment */
    code_logic.offset = (unsigned long)obtain_logic_address;
    code_logic.sel    = cs;

    printk("var  -> logic address: %#8x:%#x\n", 
                   data_logic.sel, data_logic.offset);
    printk("a_var-> logic address: %#8x:%#x\n",
                   stack_logic.sel, stack_logic.offset);
    printk("func -> logic address: %#8x:%#x\n",
                   code_logic.sel, code_logic.offset);
}

/*
 * Logical address convent to linear address
 */
static void logic_to_linear(void)
{
    unsigned long linear;
    unsigned long ds, cs;
    unsigned long cpl, dpl;
    unsigned long base, limit;
    struct logic_addr la;
    struct desc_struct *desc;

    /* Establlise a varible on Data segment */
    __asm__ ("movl %%ds, %0" : "=r" (ds));

    /* Logical address for "var" */
    la.offset = (unsigned long)&var;
    la.sel    = (unsigned long)ds;

    /* Uses the offset in the segment selector to locate the segment
     * descriptor for the segment in the GDT or LDT and reads it into
     * the processor. (This step is needed only when a new segment selector
     * is loaded into a segment register.) */
    if ((ds >> 2) & 0x1) {
        /* Segment descriptor locate in LDT */
        desc = &current->ldt[ds >> 3];
        base = get_base(*desc);
        limit = get_limit(*desc);
    } else {
        /* Segment descriptor locate in GDT */
        desc = &gdt[ds >> 3];
        base = get_base(*desc);
        limit = get_limit(*desc);
    }
    /* Examines the segment descriptor to check the access rights and range
     * of the segment to insure that the segment is accessible and that
     * offset is within the limit of the segment */
    limit = limit * 4096;
    if (la.offset > limit) {
        panic("Out of range for segment\n");
    }
    /* Obtain CPL from code selector */
    __asm__ ("movl %%cs, %0" : "=r" (cs));
    cpl = cs & 0x3;
    dpl = desc->b >> 13 & 0x3;
    if (cpl > dpl) {
        panic("Trigger #GP\n");
    }

    /* Adds the base address of segment from the segment descriptor to the 
     * offset to from a linear address. */
    linear = base + la.offset;

    printk("Logic  Address: %#8x:%#x\n", la.sel, la.offset);
    printk("Linear Address: %#8x\n", linear);
}

/*
 * Logical address convent to physical address
 */
static void logic_to_physic(void)
{
    struct logic_addr la;
    unsigned long virtual, linear, physic;
    unsigned long base, limit;
    unsigned long cs, ds, cr3;
    unsigned char cpl, dpl;
    struct desc_struct *desc;
    unsigned long *cr3_pgdir, *page_table, *page;

    /* Obtain specific segment selector */
    __asm__ ("movl %%cs, %0\n\r"
             "movl %%ds, %1"
             : "=r" (cs), "=r" (ds));

    /* Establish a logical address */
    la.offset = (unsigned long)&var;
    la.sel    = ds;

    /* Obtain virtual address */
    virtual = la.offset;

    /* violation COW */
    var[48] = 'A';

    /* Uses the offset in the segment selector to locate the segment
     * descriptor for the segment in the GDT or LDT and reads it into
     * the processor. (This step is needed only when a new segment selector
     * is loaded into a segment register.) */
    if ((la.sel >> 2) & 0x1)
        desc = &current->ldt[la.sel >> 3];
    else
        desc = &gdt[la.sel >> 3];
    /* Examines the segment descriptor to check the access rights and range
     * of the segment to insure that the segment is accessible and that
     * offset is within the limit of the segment */
    base  = get_base(*desc);
    limit = get_limit(*desc) * 4096;
    if (la.offset > limit)
        panic("Out of segment\n");
    cpl = cs & 0x3;
    dpl = desc->b >> 13 & 0x3;
    if (cpl > dpl)
        panic("Trigger #GP");

    /* Obtain linear address */
    linear = base + la.offset;
    
    /* Obtain physical address of pg-dir */
    __asm__ ("movl %%cr3, %0" : "=r" (cr3));
    cr3_pgdir = (unsigned long *)cr3;

    /* Obtain Page-Table */
    page_table = (unsigned long *)cr3_pgdir[(linear >> 22) & 0x3FF];
    page_table = (unsigned long)page_table & 0xFFFFF000;
    /* Access right check */

    /* Obtain page frame */
    page = (unsigned long *)page_table[(linear >> 12) & 0x3FF];
    /* Page 4Kb alignment */
    page = (unsigned long)page & 0xFFFFF000;
    /* Obtain specify physical address */
    physic = (unsigned char *)page + (linear & 0xFFFF);

    printk("Logical Address: %#x:%#x\n", la.sel, la.offset);
    printk("Virtual Address: %#x\n", virtual);
    printk("Linear  Address: %#x\n", linear);
    printk("Physic  Address: %#x\n", physic);
}

/* common linear address entry */
void debug_logic_address_common(void)
{
    if (1) {
        logic_to_physic();
    } else {
        obtain_logic_address();
        logic_to_linear();
        logic_to_physic();
    }
}
