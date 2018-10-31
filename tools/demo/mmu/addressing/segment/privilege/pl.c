/*
 * Segmentation mechanism
 *
 * (C) 2018.10.24 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>

#include <demo/debug.h>

/* Segmetn node */
struct desc_node {
    struct desc_struct *desc;
    unsigned short Sel;
};

/* Globl desc_node */
static struct desc_node __unused desc;

/*
 * Obtain a segment descriptor
 */
static int __unused segment_descriptor_entence(void)
{
    /* Obtain specify segment selector from Kconfig */
#ifdef CONFIG_SEG_CHECK_KERNEL_CS
    __asm__ ("mov %%cs, %0" : "=m" (desc.Sel));
#elif defined CONFIG_SEG_CHECK_KERNEL_DS
    __asm__ ("mov %%ds, %0" : "=m" (desc.Sel));
#elif defined CONFIG_SEG_CHECK_KERNEL_SS
    __asm__ ("mov %%ss, %0" : "=m" (desc.Sel));
#elif defined CONFIG_SEG_CHECK_USER_CS
    __asm__ ("mov %%cs, %0" : "=m" (desc.Sel));
#elif defined CONFIG_SEG_CHECK_USER_FS
    __asm__ ("mov %%fs, %0" : "=m" (desc.Sel));
#elif defined CONFIG_SEG_CHECK_USER_SS
    __asm__ ("mov %%ss, %0" : "=m" (desc.Sel));
#elif defined CONFIG_SEG_CHECK_LDT
    __asm__ ("sldt %0" : "=m" (desc.Sel));
#elif defined CONFIG_SEG_CHECK_TSS
    __asm__ ("str %0" : "=m" (desc.Sel));
#else 
    desc.Sel = 0x0000;
#endif

    /* Obtain segment descriptor from GDT or LDT */
    if (desc.Sel & 0x4) {
        /* Segment descriptor locate on LDT */
        desc.desc = current->ldt + (desc.Sel >> 3);
    } else {
        /* Segment descriptor locate on GDT */
        desc.desc = gdt + (desc.Sel >> 3);
    }
    printk("Segment Sel: %#x, base %#x, limit %#x\n", desc.Sel,
                        (unsigned int)get_base(*(desc.desc)), 
                        (unsigned int)get_limit(desc.Sel));

    return 0;
}

/*
 * CPL (Current Privilege levels)
 *
 * The CPL is the privilege level of the currently executing program or task.
 * It is stored in bits 0 and 1 of the CS and SS segment register. Normally,
 * the CPL is equal to the privilege level of the code segment from which
 * instructions are being fetched. The processor changes the CPL when program
 * control is transferred to a code segment with a different privilege level.
 * The CPL is treated slightly differently when accessing conforming code
 * segments. Conforming code segments can be accessed from any privilege level
 * that is equal to or numerically greater (less privileged) than the DPL of
 * the conforming code segment. Also, the CPL is not changed when the processor
 * accesses a conforming code segment that has a different privilege level than
 * CPL.
 */
static int __unused CPL_entence(void)
{
    unsigned short __unused CPL;
    unsigned short __unused Sel;
    
    /* Obtain CS segment selector */
    __asm__ ("mov %%cs, %0" : "=m" (Sel));

    /* Normally, the CPL is equal to the privilege level of the code segment
     * from which instructions are being fetched
     */
    CPL = Sel & 0x3;
    printk("Segment CPL: %#x\n", CPL);

    return 0;
}

/*
 * DPL (Descriptor Privilege levels)
 *
 * The DPL is the privilege level of a segment or gate. It is stored in the DPL
 * field of the segment or gate descriptor for the segment or gate. When the
 * currently executing code segment attempts to access a segment or gate, the
 * DPL of the segment or gate is compared to the CPL and RPL of the segment or
 * gate selector (as descriptor later in this section). The DPL is interpreted
 * differently, depending on the type of segment or gate being accessed:
 */
static int __unused DPL_entence(void)
{
    if ((desc.desc->b & 0x1800) == 0x1000) {
        /*
         * Data segment
         *
         * The DPL indicates the numerically highest privilege level that a
         * program or task can have to be allowed to access the segment. For
         * example, if the DPL of a data segment is 1, only programs running
         * at a CPL of 0 or 1 can access the segment.
         */
        printk("Data segment DPL: %#x\n", 
                                 (unsigned int)(desc.desc->b >> 13) & 0x3);
    } else if ((desc.desc->b & 0x1C00) == 0x1800) {
        /*
         * Nonconforming code segment (without using a call gate)
         *
         * The DPL indicates the privilege level that a program or task must
         * be at to access the segment. For example, if the DPL of a
         * nonconforming code segment is 0, only programs running at a CPL of
         * 0 can access the segment.
         */
         printk("Nonconforming code segment DPL: %#x\n", 
                             (unsigned int)(desc.desc->b >> 13) & 0x3);
    } else
         printk("Unknow segment PLD: %#x\n", 
                             (unsigned int)(desc.desc->b >> 13) & 0x3);
    return 0;
}

/*
 * RPL (Requested Privilege levels)
 *
 * The PRL is an override privilege level that is assigned to segment selector.
 * It is stored in bits 0 and 1 of the segment selector. The processor checks
 * the RPL along with the CPL to determine if access to a segment is allowed.
 * Even if the program or task requesting access to a segment has sufficient
 * privilege to access the segment, access is denied if the RPL is not of
 * sufficient privilege level. That is, if the RPL of a segment selector is
 * numerically greater than the CPL, the PRL overrides the CPL, and vice versa.
 * The PRL can be used to insure that privileged code does not access a segment
 * on behalf of an application program unless the program itself has access
 * privileges for that segment.
 *
 */
static int __unused RPL_entence(void)
{
    printk("Segment RPL: %#x\n", desc.Sel & 0x3);
    return 0;
}

static int segment_check_entence(void)
{
    /* Obtain Segment selector and Segment descriptor */
    segment_descriptor_entence();

#ifdef CONFIG_DEBUG_MMU_PL_CPL
    /* CPL (Current Privilege levels) */
    CPL_entence();
#endif

#ifdef CONFIG_DEBUG_MMU_PL_DPL
    /* DPL (Descriptor Privilege levels) */
    DPL_entence();
#endif

#ifdef CONFIG_DEBUG_MMU_PL_RPL
    /* PRL (Requested Privilege levels) */
    RPL_entence();
#endif

    return 0;
}
late_debugcall(segment_check_entence);
