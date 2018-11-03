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
#include <linux/unistd.h>

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

/*
 * Privilege level checking when accessing data segments
 * 
 * To access operands in a data segment, the segment selector for the data
 * segment must be loaded into to the data segment register (DS, ES, FS, or
 * GS) or into the stack segment register (SS). (Segment registers can be
 * loaded with the MOV, POP, LDS, LES, LFS, LGS, and LSS instructions.) Before
 * the processor loads a segment selector into a segent register, it performs
 * a privilege check (seeFigure) by comparing the privilege levels of the
 * currently running program or task (the CPL), the RPL of the segment
 * selector, and the DPL of the segment's segment descriptor. The processor
 * loads the segment selector into the segment register if the DPL is
 * numerically greater than or equal to both the CPL and RPL. Otherwise, a
 * general-protection fault is generated and the segment register is not
 * loaded.
 *
 *
 *  CS Regsiter
 *  +-------------------------+-----+
 *  |                         | CPL |---------o
 *  +-------------------------+-----+         |
 *                                            |      +-----------------+
 *                                            |      |                 |
 *  Segment Selector for Data Segment         o----->|                 |
 *  +-------------------------+-----+                |                 |
 *  |                         | RPL |--------------->| Privilege Check |
 *  +-------------------------+-----+                |                 |
 *                                            o----->|                 |
 *                                            |      |                 |
 *  Data-Segment Descriptor                   |      +-----------------+
 *  +-------------+-----+-----------+         |
 *  |             | DPL |           |---------o
 *  +-------------+-----+-----------+
 *  +-------------------------------+
 *  |                               |
 *  +-------------------------------+
 *
 */
static int __unused privilege_check_data_segment(void)
{
    unsigned short __unused CS;
    unsigned short __unused DS;
    unsigned short __unused SS;
    unsigned short __unused ES;
    unsigned short __unused FS;
    unsigned short __unused Sel;
    unsigned short __unused CPL;
    unsigned short __unused RPL;
    unsigned short __unused DPL;
    unsigned int __unused offset;
    unsigned int __unused seg[2];
    struct desc_struct __unused *desc;
    const char __unused *hello = "Hello BiscuitOS";

    /* Obtain CPL from CS segment selector */
    __asm__ ("mov %%cs, %0" : "=m" (Sel));
    CPL = Sel & 0x3;

#ifdef CONFIG_DEBUG_PL_DATA_C0

#ifdef CONFIG_DEBUG_PL_DATA_C0_P0
    /*
     * CPL == RPL == DPL == 0
     *
     * The procedure in code segment A0 is able to access data segment E0
     * using segment selector E0, because the CPL of code segment A0 and the
     * RPL of segment selector E0 are equal to the DPL of data segment E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment A0 |       | Segment Sel. E0 |       | Data Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+ 
     *
     */
    /* Utilize segment selector: 0x0200
     * Segment Descriptor: 0xc0c392000000ffff
     * RPL: 00
     * CPL: 00
     * DPL: 00
     */
    Sel = 0x0200;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C0_P3
    /*
     * CPL == RPL == DPL == 3
     *
     * The procedure in code segment A3 is able to access data segment E3
     * using segment selector E3, because the CPL of code segment A3 and the
     * RPL of segment selector E3 are equal to the DPL of data segment E3.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment A3 |       | Segment Sel. E3 |       | Data Segment E3 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /* Utilize segment selector: 0x03f3
     * Segment Descriptor: 0x00cbf2000000ffff
     * RPL: 03
     * CPL: 03
     * DPL: 03
     */
    Sel  = 0x03f3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);
    
#endif

#elif defined CONFIG_DEBUG_PL_DATA_C1 /* CONFIG_DEBUG_PL_DATA_C0 */

#ifdef CONFIG_DEBUG_PL_DATA_C1_P0
    /*
     * CPL:0 == RPL:0 > DPL:1
     *
     * The procedure in code segment B0 is able to access data segment E0
     * using segment selector E0, because the CPL of code segment B0 and the
     * RPL of segment selector E0 are both numerically lower than (more
     * privileged) the DPL of data segment E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B0 |       | Segment Sel. E0 |       | Data Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /* Utilize segment selector: 0x0210
     * Segment Descriptor: 0xc0c3b2000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 01
     */
    Sel  = 0x0210;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C1_P1
    /*
     * CPL:0 == RPL:0 > DPL:2
     *
     * The procedure in code segment B1 is able to access data segment E1
     * using segment selector E1, because the CPL of code segment B1 and the
     * RPL of segment selector E1 are both numerically lower than (more
     * privileged) the DPL of data segment E1.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B1 |       | Segment Sel. E1 |       | Data Segment E1 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0220
     * Segment Descriptor: 0xc0c3d2000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 02
     */
    Sel  = 0x0220;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C1_P2
    /*
     * CPL:0 == RPL:0 > DPL:3
     *
     * The procedure in code segment B3 is able to access data segment E3
     * using segment selector E3, because the CPL of code segment B3 and the
     * RPL of segment selector E3 are both numerically lower than (more
     * privileged) the DPL of data segment E3.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B3 |       | Segment Sel. E3 |       | Data Segment E3 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0230
     * Segment Descriptor: 0xc0c3f2000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 03
     */
    Sel  = 0x0230;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_DATA_C2 /* CONFIG_DEBUG_PL_DATA_C1 */

#ifdef CONFIG_DEBUG_PL_DATA_C2_P0
    /*
     * CPL:0 > RPL:1 == DPL:1
     *
     * The procedure in code segment C0 is able to access data segment E0
     * using segment selector E0, because the CPL of code segment C0 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E0, and the RPL of segment selector E0 is equal to DPL of data segment
     * E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C0 |       | Segment Sel. E0 |       | Data Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0251
     * Segment Descriptor: 0xc0c3b2000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 01
     */
    Sel  = 0x0251;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C2_P1
    /*
     * CPL:0 > RPL:2 == DPL:2
     *
     * The procedure in code segment C1 is able to access data segment E1
     * using segment selector E1, because the CPL of code segment C1 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E1, and the RPL of segment selector E1 is equal to DPL of data segment
     * E1.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C1 |       | Segment Sel. E1 |       | Data Segment E1 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x02a2
     * Segment Descriptor: 0xc0c3d2000000fffff
     * CPL: 00
     * RPL: 02
     * DPL: 02
     */
    Sel  = 0x02a2;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C2_P2
    /*
     * CPL:0 > RPL:3 == DPL:3
     *
     * The procedure in code segment C2 is able to access data segment E2
     * using segment selector E2, because the CPL of code segment C2 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E2, and the RPL of segment selector E2 is equal to DPL of data segment
     * E2.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C2 |       | Segment Sel. E2 |       | Data Segment E2 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 3         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x02f3
     * Segment Descriptor: 0xc0c3f2000000ffff
     * CPL: 00
     * PRL: 03
     * DPL: 03 
     */
    Sel  = 0x02f3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_DATA_C3 /* CONFIG_DEBUG_PL_DATA_C2 */

#ifdef CONFIG_DEBUG_PL_DATA_C3_P2
    /*
     * CPL3 == RPL:3 < DPL:0
     *
     * The procedure in code segment D2 is not able to access data segment E2
     * using segment selector E2, because the CPL of code segment D2 and the
     * RPL of segment selector E2 are both numerically greater than (less
     * privilege) the DPL of data segment E2. (The CPL of code segmetn D2 is
     * equal to RPL of segment selecotr E2).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D2 |       | Segment Sel. E2 |       | Data Segment E2 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x03C3
     * Segment Descriptor: 0x00cb92000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 00
     */
    Sel  = 0x03C3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C3_P4
    /*
     * CPL:3 == RPL:3 < DPL:1
     *
     * The procedure in code segment D4 is not able to access data segment E4
     * using segment selector E4, because the CPL of code segment D4 and the
     * RPL of segment selector E4 are both numerically greater than (less
     * privilege) the DPL of data segment E4. (The CPL of code segmetn D4 is
     * equal to RPL of segment selecotr E4).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D4 |       | Segment Sel. E4 |       | Data Segment E4 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x03d3
     * Segment Descriptor: 0x00cbb2000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 01
     */
    Sel  = 0x03d3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C3_P5
    /*
     * CPL:3 == RPL:3 < DPL:2
     *
     * The procedure in code segment D5 is not able to access data segment E5
     * using segment selector E5, because the CPL of code segment D5 and the
     * RPL of segment selector E5 are both numerically greater than (less
     * privilege) the DPL of data segment E5. (The CPL of code segmetn D5 is
     * equal to RPL of segment selecotr E5).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D5 |       | Segment Sel. E5 |       | Data Segment E5 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x03e3
     * Segment Descriptor: 0x00cbd2000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 02
     */
    Sel  = 0x03e3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_DATA_C4 /* CONFIG_DEBUG_PL_DATA_C3 */

#ifdef CONFIG_DEBUG_PL_DATA_C4_P2
    /*
     * CPL:3 < RPL:0 == DPL:0
     *
     * The procedure in code segment E2 is not able to access data segment E2
     * using segment selector E2, because the CPL of code segment E2 both are
     * numerically greater than (less privilege) the RPL of segment selector
     * E2 and the DPL of the data segment E2. (The RPL of segment selector E2
     * is equal to DPL of the data segment E2).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E2 |       | Segment Sel. E2 |       | Data Segment E2 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 0         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0300
     * Segment Desscriptor: 0x00cb92000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 00
     */
    Sel  = 0x0300;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C4_P4
    /*
     * CPL:3 < RPL:1 == DPL:1
     *
     * The procedure in code segment E4 is not able to access data segment E4
     * using segment selector E4, because the CPL of code segment E4 both are
     * numerically greater than (less privilege) the RPL of segment selector
     * E4 and the DPL of the data segment E4. (The RPL of segment selector E4
     * is equal to DPL of the data segment E4).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E4 |       | Segment Sel. E4 |       | Data Segment E4 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 1         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0351
     * Segment Descriptor: 0x00cbb2000000ffff
     * CPL: 03
     * RPL: 01
     * DPL: 01
     */
    Sel  = 0x0351;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selecotr */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C4_P5
    /*
     * CPL:3 < RPL:2 == DPL:2
     *
     * The procedure in code segment E5 is not able to access data segment E5
     * using segment selector E5, because the CPL of code segment E5 both are
     * numerically greater than (less privilege) the RPL of segment selector
     * E5 and the DPL of the data segment E5. (The RPL of segment selector E5
     * is equal to DPL of the data segment E5).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E5 |       | Segment Sel. E5 |       | Data Segment E5 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 2         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x03a2
     * Segment Descriptor: 0x00cbd2000000ffff
     * CPL: 03
     * RPL: 02
     * DPL: 02
     */
    Sel  = 0x03a2;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_DATA_C5 /* CONFIG_DEBUG_PL_DATA_C4 */

#ifdef CONFIG_DEBUG_PL_DATA_C5_P1
    /*
     * CPL:3 < RPL:0 && RPL:0 > DPL:1
     * CPL:3 < DPL:1 < RPL:0
     *
     * The procedure in code segment F1 is not able to access data segment E1
     * using segment selector E1. If the CPL of code segment F1 is numerically
     * greate than (less privilege) the DPL of data segment E1, even the RPL
     * of segment selector E1 is numerically less than (more privilege) the
     * DPL of data segment E1, the code segment F1 is able to access data
     * segment E1.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment F1 |       | Segment Sel. E1 |       | Data Segment E1 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 0         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilze segment selector: 0x0310
     * Segment Descriptor: 0x00cbb2000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 01
     */
    Sel  = 0x0310;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS)) ;
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C5_P2
    /*
     * CPL:3 < RPL:0 && RPL:0 > DPL:2
     * CPL:3 < DPL:2 < RPL:0
     *
     * The procedure in code segment F3 is not able to access data segment E3
     * using segment selector E3. If the CPL of code segment F3 is numerically
     * greate than (less privilege) the DPL of data segment E3, even the RPL
     * of segment selector E3 is numerically less than (more privilege) the DPL
     * of data segment E3, the code segment F3 is able to access data segment
     * E3.
     *
     *
     * +-----------------+       +-----------------+        +-----------------+
     * | Code Segment F3 |       | Segment Sel. E3 |        | Data Segment E3 |
     * |                -|------>|                -|---X--->|                 |
     * | CPL = 3         |       | RPL = 0         |        | DPL = 2         |
     * +-----------------+       +-----------------+        +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0320
     * Segment Descriptor: 0x00cbd2000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 02
     */
    Sel  = 0x0320;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS regsiter and trigger privilege 
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C5_P3
    /*
     * CPL:3 < RPL:1 && RPL:1 > DPL:2
     * CPL:3 < DPL:2 < RPL:1
     *
     * The procedure in code segment F2 is not able to access data segment E2
     * using segment selector E2. If the CPL of code segment F2 is numerically
     * greate than (less privilege) the DPL of data segment E2, even the RPL
     * of segment selector E2 is numerically less than (more privilege) the
     * DPL of data segment E2, the code segment F2 is able to access data
     * segment E2.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment F2 |       | Segment Sel. E2 |       | Data Segment E2 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 1         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0361
     * Segment Descriptor: 0x00cbd2000000ffff
     * CPL: 03
     * RPL: 01
     * DPL: 02
     */ 
    Sel  = 0x0361;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_DATA_C6 /* CONFIG_DEBUG_PL_DATA_C5 */

#ifdef CONFIG_DEBUG_PL_DATA_C6_P0
    /*
     * CPL:0 > RPL:2 && RPL:2 < DPL:1
     * CPL:0 > DPL:1 > RPL:2
     *
     * The procedure in code segment G0 should be able to access data segment
     * E0 because code segment D's CPL is numerically less than the DPL of
     * data segment E0. However, the RPL of segment selector E0 (which the
     * code segment G0 procedure is using to access data segment E0) is
     * numerically greater than the DPL of data segment E0, so access is not
     * allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment G0 |       | Segment Sel. E0 |       | Data Segment E0 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /* 
     * Utilize segment selector: 0x0292
     * Segment Descriptor: 0xc0c3b2000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 01
     */
    Sel  = 0x0292;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                  (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C6_P1
    /*
     * CPL:0 > RPL:3 && RPL:3 < DPL:1
     * CPL:0 > DPL:1 > RPL:3
     *
     * The procedure in code segment G1 should be able to access data segment
     * E1 because code segment D's CPL is numerically less than the DPL of
     * data segment E1. However, the RPL of segment selector E1 (which the
     * code segment G1 procedure is using to access data segment E1) is
     * numerically greater than the DPL of data segment E1, so access is not
     * allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment G1 |       | Segment Sel. E1 |       | Data Segment E1 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 3         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x02d3
     * Segment Descriptor: 0xc0c3b2000000ffff
     * CPL: 00
     * RPL: 03
     * DPL: 01
     */
    Sel  = 0x02d3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                        (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * Checking */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_DATA_C7 /* CONFIG_DEBUG_PL_DATA_C6 */

#ifdef CONFIG_DEBUG_PL_DATA_C7_P0
    /*
     * CPL:0 > RPL:1 > DPL:2
     *
     * The procedure in code segment H0 should be able to access data segment
     * E0 because code segment D's CPL is numerically less than the DPL of
     * data segment E0. However, the RPL of segment selector E0 (which the
     * code segment G0 procedure is using to access data segment E0) is
     * numerically less than the DPL of data segment E0, and the CPL of code
     * segment G0 is numerically less than the RPL of segment selector E0, so
     * access is allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H0 |       | Segment Sel. E0 |       | Data Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0261
     * Segment Descriptor: 0xc0c3d2000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 02
     */
    Sel  = 0x0261;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                                (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_DATA_C7_P1
    /*
     * CPL:0 > RPL:1 > DPL:3
     *
     * The procedure in code segment H3 should be able to access data segment
     * E3 because code segment D's CPL is numerically less than the DPL of
     * data segment E3. However, the RPL of segment selector E3 (which the
     * code segment G3 procedure is using to access data segment E3) is
     * numerically less than the DPL of data segment E3, and the CPL of code
     * segment G3 is numerically less than the RPL of segment selector E3, so
     * access is allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H3 |       | Segment Sel. E3 |       | Data Segment E3 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0271
     * Segment Descriptor: 0xc0c3f2000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 03
     */
    Sel  = 0x0271;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                   (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);
 
#elif defined CONFIG_DEBUG_PL_DATA_C7_P2
    /*
     * CPL:0 > RPL:2 > DPL:3
     *
     * The procedure in code segment H1 should be able to access data segment
     * E1 because code segment D's CPL is numerically less than the DPL of
     * data segment E1. However, the RPL of segment selector E1 (which the
     * code segment G1 procedure is using to access data segment E1) is
     * numerically less than the DPL of data segment E1, and the CPL of code
     * segment G1 is numerically less than the RPL of segment selector E1, so
     * access is allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H1 |       | Segment Sel. E1 |       | Data Segment E1 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x02b2
     * Segment Descriptor: 0xc0c3f2000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 03
     */
    Sel  = 0x02b2;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                         (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load a new data segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#endif /* CONFIG_DEBUG_PL_DATA_C7 */

    return 0;
}

#if defined CONFIG_DEBUG_PL_DATA_C0_P0 | defined CONFIG_DEBUG_PL_DATA_C1_P0 | \
    defined CONFIG_DEBUG_PL_DATA_C1_P1 | defined CONFIG_DEBUG_PL_DATA_C1_P2 | \
    defined CONFIG_DEBUG_PL_DATA_C2_P0 | defined CONFIG_DEBUG_PL_DATA_C2_P1 | \
    defined CONFIG_DEBUG_PL_DATA_C2_P2 | defined CONFIG_DEBUG_PL_DATA_C6_P0 | \
    defined CONFIG_DEBUG_PL_DATA_C6_P1 | defined CONFIG_DEBUG_PL_DATA_C7_P0 | \
    defined CONFIG_DEBUG_PL_DATA_C7_P1 | defined CONFIG_DEBUG_PL_DATA_C7_P2
late_debugcall(privilege_check_data_segment);
#elif defined CONFIG_DEBUG_PL_DATA_C0_P3 | \
      defined CONFIG_DEBUG_PL_DATA_C3_P2 | \
      defined CONFIG_DEBUG_PL_DATA_C3_P4 | \
      defined CONFIG_DEBUG_PL_DATA_C3_P5 | \
      defined CONFIG_DEBUG_PL_DATA_C4_P2 | \
      defined CONFIG_DEBUG_PL_DATA_C4_P4 | \
      defined CONFIG_DEBUG_PL_DATA_C4_P5 | \
      defined CONFIG_DEBUG_PL_DATA_C5_P1 | \
      defined CONFIG_DEBUG_PL_DATA_C5_P2 | \
      defined CONFIG_DEBUG_PL_DATA_C5_P3
user1_debugcall_sync(privilege_check_data_segment);
#endif

#ifdef CONFIG_DEBUG_PL_DATA_ESTABLISH
/*
 * Establish a lot of debug data segment selectors and segment descriptors.
 *
 * Debug segment selector and segment descriptor list
 *
 * +-------------+--------------------+-------------+-----+-----+----+
 * | Segment Sel | Segment Descriptor | Kernel/User | CPL | RPL | DPL|
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0200      | 0xc0c392000000ffff | kernel      | 00  | 00  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0210      | 0xc0c3b2000000ffff | kernel      | 00  | 00  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0220      | 0xc0c3d2000000ffff | kernel      | 00  | 00  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0230      | 0xc0c3f2000000ffff | kernel      | 00  | 00  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0241      | 0xc0c392000000ffff | kernel      | 00  | 01  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0251      | 0xc0c3b2000000ffff | kernel      | 00  | 01  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0261      | 0xc0c3d2000000ffff | kernel      | 00  | 01  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0271      | 0xc0c3f2000000ffff | kernel      | 00  | 01  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0282      | 0xc0c392000000ffff | kernel      | 00  | 02  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0292      | 0xc0c3b2000000ffff | kernel      | 00  | 02  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02a2      | 0xc0c3d2000000ffff | kernel      | 00  | 02  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02b2      | 0xc0c3f2000000ffff | kernel      | 00  | 02  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02c3      | 0xc0c392000000ffff | kernel      | 00  | 03  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02d3      | 0xc0c3b2000000ffff | kernel      | 00  | 03  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02e3      | 0xc0c3d2000000ffff | kernel      | 00  | 03  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02f3      | 0xc0c3f2000000ffff | kernel      | 00  | 03  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0300      | 0x00cb92000000ffff | User        | 03  | 00  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0310      | 0x00cbb2000000ffff | User        | 03  | 00  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0320      | 0x00cbd2000000ffff | User        | 03  | 00  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0330      | 0x00cbf2000000ffff | User        | 03  | 00  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0341      | 0x00cb92000000ffff | User        | 03  | 01  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0351      | 0x00cbb2000000ffff | User        | 03  | 01  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0361      | 0x00cbd2000000ffff | User        | 03  | 01  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0371      | 0x00cbf2000000ffff | User        | 03  | 01  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0382      | 0x00cb92000000ffff | User        | 03  | 02  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0392      | 0x00cbb2000000ffff | User        | 03  | 02  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03a2      | 0x00cbd2000000ffff | User        | 03  | 02  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03b2      | 0x00cbf2000000ffff | User        | 03  | 02  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03c3      | 0x00cb92000000ffff | User        | 03  | 03  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03d3      | 0x00cbb2000000ffff | User        | 03  | 03  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03e3      | 0x00cbd2000000ffff | User        | 03  | 03  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03f3      | 0x00cbf2000000ffff | User        | 03  | 03  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 */
static struct desc_struct __unused debug_desc[] = {
  { .a = 0x0000ffff, .b = 0xc0c39200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3b200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3d200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3f200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c39200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3b200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3d200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3f200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c39200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3b200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3d200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3f200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c39200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3b200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3d200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3f200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0x00cb9200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbb200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbd200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbf200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cb9200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbb200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbd200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbf200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cb9200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbb200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbd200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbf200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cb9200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbb200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbd200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbf200}, /* user   3GB data at 0x00000000 */
  { }
};

static int __unused establish_debug_data_segment(void)
{
    unsigned short __unused start_Sel = 0x0200;
    unsigned short __unused end_Sel   = 0x03f3;
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;
    unsigned short __unused i = 0;

    for (Sel = start_Sel; Sel <= end_Sel; Sel += 0x10, i++) {
        desc = gdt + (Sel >> 0x3);
        desc->a = debug_desc[i].a;
        desc->b = debug_desc[i].b;
    }
 
    return 0;
}
device_debugcall(establish_debug_data_segment);
#endif

#ifdef CONFIG_DEBUG_PL_CD_ESTABLISH
/*
 * Establish a lot of debug code segment selectors and segment descriptors.
 *
 * Debug segment selector and segment descriptor list
 *
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | Seg.Sel | Segment Descriptor | Ker/UR | Conforming/N | CPL | RPL | DPL |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0210  | 0xc0c39a000000ffff | Kernel | Non-Conform  | 00  | 00  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0220  | 0xc0c3ba000000ffff | Kernel | Non-Conform  | 00  | 00  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0230  | 0xc0c3da000000ffff | Kernel | Non-Conform  | 00  | 00  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0240  | 0xc0c3fa000000ffff | Kernel | Non-Conform  | 00  | 00  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0251  | 0xc0c39a000000ffff | Kernel | Non-Conform  | 00  | 01  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0261  | 0xc0c3ba000000ffff | Kernel | Non-Conform  | 00  | 01  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0271  | 0xc0c3da000000ffff | Kernel | Non-Conform  | 00  | 01  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0281  | 0xc0c3fa000000ffff | Kernel | Non-Conform  | 00  | 01  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0292  | 0xc0c39a000000ffff | Kernel | Non-Conform  | 00  | 02  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x02a2  | 0xc0c3ba000000ffff | Kernel | Non-Conform  | 00  | 02  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x02b2  | 0xc0c3da000000ffff | Kernel | Non-Conform  | 00  | 02  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x02c2  | 0xc0c3fa000000ffff | Kernel | Non-Conform  | 00  | 02  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x02d3  | 0xc0c39a000000ffff | Kernel | Non-Conform  | 00  | 03  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x02e3  | 0xc0c3ba000000ffff | Kernel | Non-Conform  | 00  | 03  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x02f3  | 0xc0c3da000000ffff | Kernel | Non-Conform  | 00  | 03  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0303  | 0xc0c3fa000000ffff | Kernel | Non-Conform  | 00  | 03  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0310  | 0x00cb9a000000ffff | User   | Non-Conform  | 03  | 00  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0320  | 0x00cbba000000ffff | User   | Non-Conform  | 03  | 00  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0330  | 0x00cbda000000ffff | User   | Non-Conform  | 03  | 00  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0340  | 0x00cbfa000000ffff | User   | Non-Conform  | 03  | 00  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0351  | 0x00cb9a000000ffff | User   | Non-Conform  | 03  | 01  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0361  | 0x00cbba000000ffff | User   | Non-Conform  | 03  | 01  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0371  | 0x00cbda000000ffff | User   | Non-Conform  | 03  | 01  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0381  | 0x00cbfa000000ffff | User   | Non-Conform  | 03  | 01  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0392  | 0x00cb9a000000ffff | User   | Non-Conform  | 03  | 02  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x03a2  | 0x00cbba000000ffff | User   | Non-Conform  | 03  | 02  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x03b2  | 0x00cbda000000ffff | User   | Non-Conform  | 03  | 02  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x03c2  | 0x00cbfa000000ffff | User   | Non-Conform  | 03  | 02  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x03d3  | 0x00cb9a000000ffff | User   | Non-Conform  | 03  | 03  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x03e3  | 0x00cbba000000ffff | User   | Non-Conform  | 03  | 03  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x03f3  | 0x00cbda000000ffff | User   | Non-Conform  | 03  | 03  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0403  | 0x00cbfa000000ffff | User   | Non-Conform  | 03  | 03  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0410  | 0xc0c39a000000ffff | Kernel |  Conforming  | 00  | 00  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0420  | 0xc0c3be000000ffff | Kernel |  Conforming  | 00  | 00  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0430  | 0xc0c3de000000ffff | Kernel |  Conforming  | 00  | 00  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0440  | 0xc0c3fe000000ffff | Kernel |  Conforming  | 00  | 00  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0451  | 0xc0c39e000000ffff | Kernel |  Conforming  | 00  | 01  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0461  | 0xc0c3be000000ffff | Kernel |  Conforming  | 00  | 01  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0471  | 0xc0c3de000000ffff | Kernel |  Conforming  | 00  | 01  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0481  | 0xc0c3fe000000ffff | Kernel |  Conforming  | 00  | 01  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0492  | 0xc0c39e000000ffff | Kernel |  Conforming  | 00  | 02  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x04a2  | 0xc0c3be000000ffff | Kernel |  Conforming  | 00  | 02  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x04b2  | 0xc0c3de000000ffff | Kernel |  Conforming  | 00  | 02  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x04c2  | 0xc0c3fe000000ffff | Kernel |  Conforming  | 00  | 02  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x04e3  | 0xc0c39e000000ffff | Kernel |  Conforming  | 00  | 03  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x04e3  | 0xc0c3be000000ffff | Kernel |  Conforming  | 00  | 03  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x04f3  | 0xc0c3de000000ffff | Kernel |  Conforming  | 00  | 03  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0503  | 0xc0c3fe000000ffff | Kernel |  Conforming  | 00  | 03  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0510  | 0x00cb9e000000ffff | User   |  Conforming  | 03  | 00  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0520  | 0x00cbbe000000ffff | User   |  Conforming  | 03  | 00  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0530  | 0x00cbde000000ffff | User   |  Conforming  | 03  | 00  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0540  | 0x00cbfe000000ffff | User   |  Conforming  | 03  | 00  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0551  | 0x00cb9e000000ffff | User   |  Conforming  | 03  | 01  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0561  | 0x00cbbe000000ffff | User   |  Conforming  | 03  | 01  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0571  | 0x00cbde000000ffff | User   |  Conforming  | 03  | 01  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0581  | 0x00cbfe000000ffff | User   |  Conforming  | 03  | 01  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0592  | 0x00cb9e000000ffff | User   |  Conforming  | 03  | 02  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x05a2  | 0x00cbbe000000ffff | User   |  Conforming  | 03  | 02  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x05b2  | 0x00cbde000000ffff | User   |  Conforming  | 03  | 02  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x05c2  | 0x00cbfe000000ffff | User   |  Conforming  | 03  | 02  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x05d3  | 0x00cb9e000000ffff | User   |  Conforming  | 03  | 03  | 00  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x05e3  | 0x00cbbe000000ffff | User   |  Conforming  | 03  | 03  | 01  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x05f3  | 0x00cbde000000ffff | User   |  Conforming  | 03  | 03  | 02  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 * | 0x0603  | 0x00cbfe000000ffff | User   |  Conforming  | 03  | 03  | 03  |
 * +---------+--------------------+--------+--------------+-----+-----+-----+
 */
static struct desc_struct __unused debug_desc1[] = {
    { .a = 0x0000ffff, .b = 0xc0c39a00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3ba00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3da00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3fa00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c39a00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3ba00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3da00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3fa00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c39a00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3ba00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3da00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3fa00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c39a00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3ba00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3da00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3fa00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0x00cb9a00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbba00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbda00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbfa00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cb9a00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbba00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbda00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbfa00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cb9a00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbba00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbda00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbfa00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cb9a00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbba00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbda00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbfa00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0xc0c39e00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3be00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3de00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3fe00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c39e00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3be00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3de00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3fe00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c39e00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3be00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3de00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3fe00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c39e00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3be00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3de00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0xc0c3fe00 }, /* kernel 1GB code at 0xC0000000 */
    { .a = 0x0000ffff, .b = 0x00cb9e00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbbe00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbde00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbfe00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cb9e00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbbe00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbde00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbfe00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cb9e00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbbe00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbde00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbfe00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cb9e00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbbe00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbde00 }, /* user   3GB code at 0x00000000 */
    { .a = 0x0000ffff, .b = 0x00cbfe00 }, /* user   3GB code at 0x00000000 */
    { }
};

static int __unused establish_debug_code_segment(void)
{
    unsigned short __unused start_Sel = 0x0210;
    unsigned short __unused end_Sel   = 0x0603;
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;
    unsigned short __unused i = 0;

    for (Sel = start_Sel; Sel <= end_Sel; Sel += 0x10, i++) {
        desc = gdt + (Sel >> 0x3);
        desc->a = debug_desc1[i].a;
        desc->b = debug_desc1[i].b;
    }

    return 0;
}
device_debugcall(establish_debug_code_segment);
#endif

/*
 * Privilege Checking when accessing data in Code segment.
 *
 * In some instances it may be desirable to access data structes that are
 * contained in a code segment. The following methods of accessing data in
 * code segments are possible:
 *
 * * Load a data-segment register with a segment selector for a nonconforming,
 *   readable, code segment.
 *
 * * Load a data-segment register with a segment selector for a conforming,
 *   readable, code segment.
 *
 * * User a code-segment override prefix(CS) to read a readable, code segment
 *   whose selector is already loaded in the CS register.
 *
 * The same rules for accessing data segments apply to method 1. Method 2 is
 * always valid because the privilege level of a conforming code segment is
 * effectively the same as the CPL, regardless of its DPL. Method 3 is always
 * valid because the DPL of the code segment selected by the CS register is
 * the same as the CPL.
 *
 */
static int __unused privilege_check_data_segment_from_CS(void)
{
    unsigned short __unused CS;
    unsigned short __unused DS;
    unsigned short __unused Sel;
    unsigned short __unused CPL;
    unsigned short __unused RPL;
    unsigned short __unused DPL;
    struct desc_struct __unused *desc;
    char __unused *hello = "hello BiscuitOS";

    /* Obtain CPL from Code segment selector */
    __asm__ ("mov %%cs, %0" : "=m" (CS));
    CPL = CS & 0x3;

#ifdef CONFIG_DEBUG_PL_CD_C0

#ifdef CONFIG_DEBUG_PL_CD_C0_P0
    /*
     * CPL == RPL == DPL == 0
     *
     * The procedure in code segment A0 is able to access code segment E0
     * using segment selector E0, because the CPL of code segment A0 and the
     * RPL of segment selector E0 are equal to the DPL of code segment E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment A0 |       | Segment Sel. E0 |       | Code Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+ 
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0410
     * Segment Descriptor: 0xc0c39a000000ffff
     * RPL: 00
     * CPL: 00
     * DPL: 00
     */
    Sel = 0x0410;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0210
     * Segment Descriptor: 0xc0c39a000000ffff
     * RPL: 00
     * CPL: 00
     * DPL: 00
     */
    Sel = 0x0210;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C0_P3
    /*
     * CPL == RPL == DPL == 3
     *
     * The procedure in code segment A3 is able to access Code segment E3
     * using segment selector E3, because the CPL of code segment A3 and the
     * RPL of segment selector E3 are equal to the DPL of Code segment E3.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment A3 |       | Segment Sel. E3 |       | Code Segment E3 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0603
     * Segment Descriptor: 0x00cbfe000000ffff
     * RPL: 03
     * CPL: 03
     * DPL: 03
     */
    Sel = 0x0603;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0403
     * Segment Descriptor: 0x00cbfa000000ffff
     * RPL: 03
     * CPL: 03
     * DPL: 03
     */
    Sel = 0x0403;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);
    
#endif

#elif defined CONFIG_DEBUG_PL_CD_C1 /* CONFIG_DEBUG_PL_CD_C0 */

#ifdef CONFIG_DEBUG_PL_CD_C1_P0
    /*
     * CPL:0 == RPL:0 > DPL:1
     *
     * The procedure in code segment B0 is able to access data in code segment 
     * E0 using segment selector E0, because the CPL of code segment B0 and the
     * RPL of segment selector E0 are both numerically lower than (more
     * privileged) the DPL of code segment E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B0 |       | Segment Sel. E0 |       | Code Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0420
     * Segment Descriptor: 0xc0c3be000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 01
     */
    Sel = 0x0420;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0220
     * Segment Descriptor: 0xc0c3ba000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 01
     */
    Sel = 0x0220;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C1_P1
    /*
     * CPL:0 == RPL:0 > DPL:2
     *
     * The procedure in code segment B1 is able to access data on code segment
     * E1 using segment selector E1, because the CPL of code segment B1 and the
     * RPL of segment selector E1 are both numerically lower than (more
     * privileged) the DPL of code segment E1.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B1 |       | Segment Sel. E1 |       | Code Segment E1 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0430
     * Segment Descriptor: 0xc0c3de000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 02
     */
    Sel = 0x0430;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0230
     * Segment Descriptor: 0xc0c3da000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 02
     */
    Sel = 0x0230;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C1_P2
    /*
     * CPL:0 == RPL:0 > DPL:3
     *
     * The procedure in code segment B3 is able to access data on code segment
     * E3 using segment selector E3, because the CPL of code segment B3 and the
     * RPL of segment selector E3 are both numerically lower than (more
     * privileged) the DPL of code segment E3.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B3 |       | Segment Sel. E3 |       | Code Segment E3 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0440
     * Segment Descriptor: 0xc0c3fe000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 03
     */
    Sel = 0x0440;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0240
     * Segment Descriptor: 0xc0c3fa000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 03
     */
    Sel = 0x0240;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_CD_C2 /* CONFIG_DEBUG_PL_CD_C1 */

#ifdef CONFIG_DEBUG_PL_CD_C2_P0
    /*
     * CPL:0 > RPL:1 == DPL:1
     *
     * The procedure in code segment C0 is able to access data on code segment
     * E0 using segment selector E0, because the CPL of code segment C0 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E0, and the RPL of segment selector E0 is equal to DPL of code segment
     * E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C0 |       | Segment Sel. E0 |       | Code Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0461
     * Segment Descriptor: 0xc0c3be000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 01
     */
    Sel = 0x0461;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0261
     * Segment Descriptor: 0xc0c3ba000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 01
     */
    Sel = 0x0261;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C2_P1
    /*
     * CPL:0 > RPL:2 == DPL:2
     *
     * The procedure in code segment C1 is able to access data on code segment
     * E1 using segment selector E1, because the CPL of code segment C1 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E1, and the RPL of segment selector E1 is equal to DPL of code segment
     * E1.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C1 |       | Segment Sel. E1 |       | Code Segment E1 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x04b2
     * Segment Descriptor: 0xc0c3de000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 02
     */
    Sel = 0x04b2;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x02b2
     * Segment Descriptor: 0xc0c3da000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 02
     */
    Sel = 0x02b2;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C2_P2
    /*
     * CPL:0 > RPL:3 == DPL:3
     *
     * The procedure in code segment C2 is able to access data on code segment
     * E2 using segment selector E2, because the CPL of code segment C2 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E2, and the RPL of segment selector E2 is equal to DPL of code segment
     * E2.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C2 |       | Segment Sel. E2 |       | Code Segment E2 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 3         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0503
     * Segment Descriptor: 0xc0c3fe000000ffff
     * CPL: 00
     * RPL: 03
     * DPL: 03
     */
    Sel = 0x0503;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0303
     * Segment Descriptor: 0xc0c3fa000000ffff
     * CPL: 00
     * RPL: 03
     * DPL: 03
     */
    Sel = 0x0303;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_CD_C3 /* CONFIG_DEBUG_PL_CD_C2 */

#ifdef CONFIG_DEBUG_PL_CD_C3_P2
    /*
     * CPL3 == RPL:3 < DPL:0
     *
     * The procedure in code segment D2 is not able to access data on code 
     * segment E2 (E2 is a non-conforming code segment) using segment selector
     * E2, because the CPL of code segment D2 and the RPL of segment selector
     * E2 are both numerically greater than (less privilege) the DPL of code
     * segment E2. (The CPL of code segmetn D2 is equal to RPL of segment
     * selecotr E2).
     *
     * E2: A non-conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D2 |       | Segment Sel. E2 |       | Code Segment E2 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if the E2 is a conforming code segment, the procedure in code 
     * segment D2 is able to access data on code segment E2.
     *
     * E2: A conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D2 |       | Segment Sel. E2 |       | Code Segment E2 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x05d3
     * Segment Descriptor: 0x00cb9e000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 00
     */
    Sel = 0x05d3;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x03d3
     * Segment Descriptor: 0x00cb9a000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 00
     */
    Sel = 0x03d3;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C3_P4
    /*
     * CPL:3 == RPL:3 < DPL:1
     *
     * The procedure in code segment D4 is not able to access data on 
     * non-conforming code segment E4 using segment selector E4, because the
     * CPL of code segment D4 and the  RPL of segment selector E4 are both
     *  numerically greater than (less privilege) the DPL of code segment E4.
     * (The CPL of code segmetn D4 is equal to RPL of segment selecotr E4).
     *
     * E4: A non-conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D4 |       | Segment Sel. E4 |       | Code Segment E4 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if the E4 is a comforming code segment, the segment D4 is able to
     * access data on code segment E4.
     *
     * E4: A conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D4 |       | Segment Sel. E4 |       | Code Segment E4 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x05e3
     * Segment Descriptor: 0x00cbbe000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 01
     */
    Sel = 0x05e3;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x03e3
     * Segment Descriptor: 0x00cbba000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 01
     */
    Sel = 0x03e3;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C3_P5
    /*
     * CPL:3 == RPL:3 < DPL:2
     *
     * The procedure in code segment D5 is not able to access data on 
     * non-conforming code segment E5 using segment selector E5, because the
     * CPL of code segment D5 and the RPL of segment selector E5 are both
     * numerically greater than (less privilege) the DPL of code segment E5.
     * (The CPL of code segmetn D5 is equal to RPL of segment selecotr E5).
     *
     * E5: A non-conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D5 |       | Segment Sel. E5 |       | Code Segment E5 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if E5 is a conforming code segment, and D5 is able to access data
     * on code segment E5.
     *
     * E5: A conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D5 |       | Segment Sel. E5 |       | Code Segment E5 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x05f3
     * Segment Descriptor: 0x00cbde000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 02
     */
    Sel = 0x05f3;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x03f3
     * Segment Descriptor: 0x00cbda000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 02
     */
    Sel = 0x03f3;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_CD_C4 /* CONFIG_DEBUG_PL_CD_C3 */

#ifdef CONFIG_DEBUG_PL_CD_C4_P2
    /*
     * CPL:3 < RPL:0 == DPL:0
     *
     * The procedure in code segment E2 is not able to access data on 
     * non-conforming segment E2 using segment selector E2, because the CPL
     * of code segment E2 both are numerically greater than (less privilege)
     * the RPL of segment selector E2 and the DPL of the code segment E2. (The
     * RPL of segment selector E2 is equal to DPL of the code segment E2).
     *
     * E2: A non-conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E2 |       | Segment Sel. E2 |       | Code Segment E2 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 0         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if E2 is a conforming code segment, and code segment E2 is able to
     * access data on code segment E2.
     *
     * E2: A conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E2 |       | Segment Sel. E2 |       | Code Segment E2 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 0         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0510
     * Segment Descriptor: 0x00cb9e000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 00
     */
    Sel = 0x0510;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0310
     * Segment Descriptor: 0x00cb9a000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 00
     */
    Sel = 0x0310;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C4_P4
    /*
     * CPL:3 < RPL:1 == DPL:1
     *
     * The procedure in code segment E4 is not able to access data on 
     * non-conforming code segment E4 using segment selector E4, because the
     * CPL of code segment E4 both are numerically greater than (less
     * privilege) the RPL of segment selector E4 and the DPL of the code
     * segment E4. (The RPL of segment selector E4 is equal to DPL of the code
     * segment E4).
     *
     * E4: A non-conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E4 |       | Segment Sel. E4 |       | Code Segment E4 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 1         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if E4 is a conforming code segment, and E4 is able to access data
     * on code segment E4.
     *
     * E4: A conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E4 |       | Segment Sel. E4 |       | Code Segment E4 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 1         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0561
     * Segment Descriptor: 0x00cbbe000000ffff
     * CPL: 03
     * RPL: 01
     * DPL: 01
     */
    Sel = 0x0561;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0361
     * Segment Descriptor: 0x00cbba000000ffff
     * CPL: 03
     * RPL: 01
     * DPL: 01
     */
    Sel = 0x0361;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selecotr */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C4_P5
    /*
     * CPL:3 < RPL:2 == DPL:2
     *
     * The procedure in code segment E5 is not able to access data on 
     * non-conforming code segment E5 using segment selector E5, because the
     * CPL of code segment E5 both are numerically greater than (less 
     * privilege) the RPL of segment selector E5 and the DPL of the code
     * segment E5. (The RPL of segment selector E5 is equal to DPL of the code
     * segment E5).
     *
     * E5: A non-conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E5 |       | Segment Sel. E5 |       | Code Segment E5 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 2         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if E5 is a conforming code segment, and E5 is able to access data
     * on code segment E5.
     *
     * E5: A conforming code segment
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E5 |       | Segment Sel. E5 |       | Code Segment E5 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 2         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x05b2
     * Segment Descriptor: 0x00cbde000000ffff
     * CPL: 03
     * RPL: 02
     * DPL: 02
     */
    Sel = 0x05b2;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x03b2
     * Segment Descriptor: 0x00cbda000000ffff
     * CPL: 03
     * RPL: 02
     * DPL: 02
     */
    Sel = 0x03b2;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_CD_C5 /* CONFIG_DEBUG_PL_CD_C4 */

#ifdef CONFIG_DEBUG_PL_CD_C5_P1
    /*
     * CPL:3 < RPL:0 && RPL:0 > DPL:1
     * CPL:3 < DPL:1 < RPL:0
     *
     * The procedure in code segment F1 is not able to access data on 
     * non-conforming code segment E1 using segment selector E1. If the CPL
     * of code segment F1 is numerically greate than (less privilege) the DPL
     * of code segment E1, even the RPL of segment selector E1 is numerically
     * less than (more privilege) the DPL of code segment E1, the code segment
     * F1 is able to access data segment E1.
     *
     * E1: A non-conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment F1 |       | Segment Sel. E1 |       | Code Segment E1 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 0         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if E1 is a conforming code segment, and F1 is able to access data
     * on code segment E1.
     *
     * E1: A conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment F1 |       | Segment Sel. E1 |       | Code Segment E1 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 0         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0520
     * Segment Descriptor: 0x00cbbe000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 01
     */
    Sel = 0x0520;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0320
     * Segment Descriptor: 0x00cbba000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 01
     */
    Sel = 0x0320;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS)) ;
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C5_P2
    /*
     * CPL:3 < RPL:0 && RPL:0 > DPL:2
     * CPL:3 < DPL:2 < RPL:0
     *
     * The procedure in code segment F3 is not able to access data on 
     * non-conforming code segment E3 using segment selector E3. If the CPL
     * of code segment F3 is numerically greate than (less privilege) the DPL
     * of code segment E3, even the RPL of segment selector E3 is numerically
     * less than (more privilege) the DPL of code segment E3, the code segment
     * F3 is able to access data segment E3.
     *
     * E3: A non-conforming code segment.
     *
     * +-----------------+       +-----------------+        +-----------------+
     * | Code Segment F3 |       | Segment Sel. E3 |        | Code Segment E3 |
     * |                -|------>|                -|---X--->|                 |
     * | CPL = 3         |       | RPL = 0         |        | DPL = 2         |
     * +-----------------+       +-----------------+        +-----------------+
     *
     * But if E3 is a conforming code segment, and F3 is able to access data
     * on code segment E3.
     *
     * E3: A conforming code segment.
     *
     * +-----------------+       +-----------------+        +-----------------+
     * | Code Segment F3 |       | Segment Sel. E3 |        | Code Segment E3 |
     * |                -|------>|                -|------->|                 |
     * | CPL = 3         |       | RPL = 0         |        | DPL = 2         |
     * +-----------------+       +-----------------+        +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0530
     * Segment Descriptor: 0x00cbde000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 02
     */
    Sel = 0x0530;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0330
     * Segment Descriptor: 0x00cbda000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 02
     */
    Sel = 0x0330;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS regsiter and trigger privilege 
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C5_P3
    /*
     * CPL:3 < RPL:1 && RPL:1 > DPL:2
     * CPL:3 < DPL:2 < RPL:1
     *
     * The procedure in code segment F2 is not able to access data on
     * non-conforming code segment E2 using segment selector E2. If the CPL of
     * code segment F2 is numerically greate than (less privilege) the DPL of
     * code segment E2, even the RPL of segment selector E2 is numerically
     * less than (more privilege) the DPL of code segment E2, the code segment
     * F2 is able to access code segment E2.
     *
     * E2: A non-conforming data segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment F2 |       | Segment Sel. E2 |       | Code Segment E2 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 1         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if E2 is a conforming code segment, F2 is able to access data on
     * code segment E2.
     *
     * E2: A conforming data segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment F2 |       | Segment Sel. E2 |       | Code Segment E2 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 1         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0571
     * Segment Descriptor: 0x00cbde000000ffff
     * CPL: 03
     * RPL: 01
     * DPL: 02
     */
    Sel = 0x0571;
    printf("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0371
     * Segment Descriptor: 0x00cbda000000ffff
     * CPL: 03
     * RPL: 01
     * DPL: 02
     */
    Sel = 0x0371;
    printf("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printf("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_CD_C6 /* CONFIG_DEBUG_PL_CD_C5 */

#ifdef CONFIG_DEBUG_PL_CD_C6_P0
    /*
     * CPL:0 > RPL:2 && RPL:2 < DPL:1
     * CPL:0 > DPL:1 > RPL:2
     *
     * The procedure in code segment G0 should be able to access data on 
     * non-conforming code segment E0 because code segment D's CPL is
     * numerically less than the DPL of code segment E0. However, the RPL of
     * segment selector E0 (which the code segment G0 procedure is using to
     * access code segment E0) is numerically greater than the DPL of code
     * segment E0, so access is not allowed.
     *
     * E0: a non-conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment G0 |       | Segment Sel. E0 |       | Code Segment E0 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if E0 is a conforming code segment, and G0 is able to access data
     * on code segment E0.
     *
     * E0: a conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment G0 |       | Segment Sel. E0 |       | Code Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x04a2
     * Segment Descriptor: 0xc0c3be000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 01
     */
    Sel = 0x04a2;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x02a2
     * Segment Descriptor: 0xc0c3ba000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 01
     */
    Sel = 0x02a2;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                  (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C6_P1
    /*
     * CPL:0 > RPL:3 && RPL:3 < DPL:1
     * CPL:0 > DPL:1 > RPL:3
     *
     * The procedure in code segment G1 should be able to access data on 
     * non-conforming code segment E1 because code segment D's CPL is
     * numerically less than the DPL of code segment E1. However, the RPL of
     * segment selector E1 (which the code segment G1 procedure is using to
     * access data segment E1) is numerically greater than the DPL of code
     * segment E1, so access is not allowed.
     *
     * E1: A non-conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment G1 |       | Segment Sel. E1 |       | Code Segment E1 |
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 3         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     * But if E1 is a conforming code segment, and G1 is able to access data
     * on code segment E1.
     *
     * E1: A conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment G1 |       | Segment Sel. E1 |       | Code Segment E1 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 3         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x04e3
     * Segment Descriptor: 0xc0c3be000000ffff
     * CPL: 00
     * RPL: 03
     * DPL: 01
     */
    Sel = 0x04e3;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x02e3
     * Segment Descriptor: 0xc0c3ba000000ffff
     * CPL: 00
     * RPL: 03
     * DPL: 01
     */
    Sel = 0x02e3;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                        (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * Checking */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_CD_C7 /* CONFIG_DEBUG_PL_CD_C6 */

#ifdef CONFIG_DEBUG_PL_CD_C7_P0
    /*
     * CPL:0 > RPL:1 > DPL:2
     *
     * The procedure in code segment H0 should be able to access data on
     * non-conforming code segment E0 because code segment D's CPL is
     * numerically less than the DPL of code segment E0. However, the RPL of
     * segment selector E0 (which the code segment G0 procedure is using to
     * access data segment E0) is numerically less than the DPL of code
     * segment E0, and the CPL of code segment G0 is numerically less than
     * the RPL of segment selector E0, so access is allowed.
     *
     * E0: A non-conforming code segment.
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H0 |       | Segment Sel. E0 |       | Code Segment E0 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0471
     * Segment Descriptor: 0xc0c3de000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 02
     */
    Sel = 0x0471;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0271
     * Segment Descriptor: 0xc0c3da000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 02
     */
    Sel = 0x0271;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                                (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#elif defined CONFIG_DEBUG_PL_CD_C7_P1
    /*
     * CPL:0 > RPL:1 > DPL:3
     *
     * The procedure in code segment H3 should be able to access data on code
     * segment E3 because code segment D's CPL is numerically less than the
     * DPL of code segment E3. However, the RPL of segment selector E3 (which
     * the code segment G3 procedure is using to access data segment E3) is
     * numerically less than the DPL of code segment E3, and the CPL of code
     * segment G3 is numerically less than the RPL of segment selector E3, so
     * access is allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H3 |       | Segment Sel. E3 |       | Code Segment E3 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x0481
     * Segment Descriptor: 0xc0c3fe000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 03
     */
    Sel = 0x0481;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x0281
     * Segment Descriptor: 0xc0c3fa000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 03
     */
    Sel = 0x0281;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                   (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);
 
#elif defined CONFIG_DEBUG_PL_CD_C7_P2
    /*
     * CPL:0 > RPL:2 > DPL:3
     *
     * The procedure in code segment H1 should be able to access data on code
     * segment E1 because code segment D's CPL is numerically less than the
     * DPL of code segment E1. However, the RPL of segment selector E1 (which
     * the code segment G1 procedure is using to access code segment E1) is
     * numerically less than the DPL of code segment E1, and the CPL of code
     * segment G1 is numerically less than the RPL of segment selector E1, so
     * access is allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H1 |       | Segment Sel. E1 |       | Code Segment E1 |
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
#ifdef CONFIG_DEBUG_PL_CD_CFM
    /* Conforming Code utilize segment selector: 0x04c2
     * Segment Descriptor: 0xc0c3fe000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 03
     */
    Sel = 0x04c2;
    printk("Privilage Check when Access data on conforming Code segment.\n");
#elif defined CONFIG_DEBUG_PL_CD_UCFM
    /* Non-Conforming utilize segment selector: 0x02c2
     * Segment Descriptor: 0xc0c3fa000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 03
     */
    Sel = 0x02c2;
    printk("Privilage Check when Access data on nonconforming Code segment.\n");
#endif
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                         (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load a new code segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ds" :: "r" (Sel));
    /* Store data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("DS:  %#x -- Char: %s\n", DS, hello);

#endif

#endif /* CONFIG_DEBUG_PL_CD_C7 */

    return 0;
}
#if \
    defined CONFIG_DEBUG_PL_CD_C0_P0 | defined CONFIG_DEBUG_PL_CD_C1_P0 | \
    defined CONFIG_DEBUG_PL_CD_C1_P1 | defined CONFIG_DEBUG_PL_CD_C1_P2 | \
    defined CONFIG_DEBUG_PL_CD_C2_P0 | defined CONFIG_DEBUG_PL_CD_C2_P1 | \
    defined CONFIG_DEBUG_PL_CD_C2_P2 | defined CONFIG_DEBUG_PL_CD_C6_P0 | \
    defined CONFIG_DEBUG_PL_CD_C6_P1 | defined CONFIG_DEBUG_PL_CD_C7_P0 | \
    defined CONFIG_DEBUG_PL_CD_C7_P1 | defined CONFIG_DEBUG_PL_CD_C7_P2
late_debugcall(privilege_check_data_segment_from_CS);
#elif defined CONFIG_DEBUG_PL_CD_C0_P3 | \
      defined CONFIG_DEBUG_PL_CD_C3_P2 | \
      defined CONFIG_DEBUG_PL_CD_C3_P4 | \
      defined CONFIG_DEBUG_PL_CD_C3_P5 | \
      defined CONFIG_DEBUG_PL_CD_C4_P2 | \
      defined CONFIG_DEBUG_PL_CD_C4_P4 | \
      defined CONFIG_DEBUG_PL_CD_C4_P5 | \
      defined CONFIG_DEBUG_PL_CD_C5_P1 | \
      defined CONFIG_DEBUG_PL_CD_C5_P2 | \
      defined CONFIG_DEBUG_PL_CD_C5_P3
user1_debugcall_sync(privilege_check_data_segment_from_CS);
#endif

#ifdef CONFIG_DEBUG_PL_STACK_ESTABLISH
/*
 * Establish a lot of debug stack segment selectors and segment descriptors.
 *
 * Debug segment selector and segment descriptor list
 *
 * +-------------+--------------------+-------------+-----+-----+----+
 * | Segment Sel | Segment Descriptor | Kernel/User | CPL | RPL | DPL|
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0200      | 0xc0c392000000ffff | kernel      | 00  | 00  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0210      | 0xc0c3b2000000ffff | kernel      | 00  | 00  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0220      | 0xc0c3d2000000ffff | kernel      | 00  | 00  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0230      | 0xc0c3f2000000ffff | kernel      | 00  | 00  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0241      | 0xc0c392000000ffff | kernel      | 00  | 01  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0251      | 0xc0c3b2000000ffff | kernel      | 00  | 01  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0261      | 0xc0c3d2000000ffff | kernel      | 00  | 01  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0271      | 0xc0c3f2000000ffff | kernel      | 00  | 01  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0282      | 0xc0c392000000ffff | kernel      | 00  | 02  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0292      | 0xc0c3b2000000ffff | kernel      | 00  | 02  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02a2      | 0xc0c3d2000000ffff | kernel      | 00  | 02  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02b2      | 0xc0c3f2000000ffff | kernel      | 00  | 02  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02c3      | 0xc0c392000000ffff | kernel      | 00  | 03  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02d3      | 0xc0c3b2000000ffff | kernel      | 00  | 03  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02e3      | 0xc0c3d2000000ffff | kernel      | 00  | 03  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x02f3      | 0xc0c3f2000000ffff | kernel      | 00  | 03  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0300      | 0x00cb92000000ffff | User        | 03  | 00  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0310      | 0x00cbb2000000ffff | User        | 03  | 00  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0320      | 0x00cbd2000000ffff | User        | 03  | 00  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0330      | 0x00cbf2000000ffff | User        | 03  | 00  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0341      | 0x00cb92000000ffff | User        | 03  | 01  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0351      | 0x00cbb2000000ffff | User        | 03  | 01  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0361      | 0x00cbd2000000ffff | User        | 03  | 01  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0371      | 0x00cbf2000000ffff | User        | 03  | 01  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0382      | 0x00cb92000000ffff | User        | 03  | 02  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0392      | 0x00cbb2000000ffff | User        | 03  | 02  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03a2      | 0x00cbd2000000ffff | User        | 03  | 02  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03b2      | 0x00cbf2000000ffff | User        | 03  | 02  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03c3      | 0x00cb92000000ffff | User        | 03  | 03  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03d3      | 0x00cbb2000000ffff | User        | 03  | 03  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03e3      | 0x00cbd2000000ffff | User        | 03  | 03  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x03f3      | 0x00cbf2000000ffff | User        | 03  | 03  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 */
static struct desc_struct __unused debug_desc2[] = {
  { .a = 0x0000ffff, .b = 0xc0c39200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3b200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3d200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3f200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c39200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3b200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3d200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3f200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c39200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3b200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3d200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3f200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c39200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3b200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3d200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3f200}, /* kernel 1GB data at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0x00cb9200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbb200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbd200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbf200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cb9200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbb200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbd200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbf200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cb9200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbb200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbd200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbf200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cb9200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbb200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbd200}, /* user   3GB data at 0x00000000 */
  { .a = 0x0000ffff, .b = 0x00cbf200}, /* user   3GB data at 0x00000000 */
  { }
};

static int __unused establish_debug_stack_segment(void)
{
    unsigned short __unused start_Sel = 0x0200;
    unsigned short __unused end_Sel   = 0x03f3;
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;
    unsigned short __unused i = 0;

    for (Sel = start_Sel; Sel <= end_Sel; Sel += 0x10, i++) {
        desc = gdt + (Sel >> 0x3);
        desc->a = debug_desc2[i].a;
        desc->b = debug_desc2[i].b;
    }
 
    return 0;
}
device_debugcall(establish_debug_stack_segment);
#endif

#ifdef CONFIG_DEBUG_PL_STACK
/*
 * Privilege level checking when loading the SS register
 *
 * Privilege level checking also occurs when the SS register is loaded with the
 * segment selector for a stack segment. Here all privilege levels related to
 * the stack segment must match the CPL; that is, the CPL, the RPL of the stack
 * segment selector, and the DPL of the stack-segment descriptor must be the
 * same. If the RPL and DPL are not equal to the CPL, a general-protection
 * exception (#GP) is generated.
 */
static int __unused privilege_check_stack_segment(void)
{
    unsigned short __unused CS;
    unsigned short __unused DS;
    unsigned short __unused SS;
    unsigned short __unused ES;
    unsigned short __unused FS;
    unsigned short __unused Sel;
    unsigned short __unused CPL;
    unsigned short __unused RPL;
    unsigned short __unused DPL;
    unsigned int __unused offset;
    unsigned int __unused seg[2];
    struct desc_struct __unused *desc;
    const char __unused *hello = "Hello BiscuitOS";

    /* Obtain CPL from CS segment selector */
    __asm__ ("mov %%cs, %0" : "=m" (Sel));
    CPL = Sel & 0x3;

#ifdef CONFIG_DEBUG_PL_STACK_C0

#ifdef CONFIG_DEBUG_PL_STACK_C0_P0
    /*
     * CPL == RPL == DPL == 0
     *
     * The procedure in code segment A0 is able to access stack segment E0
     * using segment selector E0, because the CPL of code segment A0 and the
     * RPL of segment selector E0 are equal to the DPL of stack segment E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment A0 |       | Segment Sel. E0 |       | Stack Segment E0|
     * |                -|------>|                -|------>|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+ 
     *
     */
    /* Utilize segment selector: 0x0200
     * Segment Descriptor: 0xc0c392000000ffff
     * RPL: 00
     * CPL: 00
     * DPL: 00
     */
    Sel = 0x0200;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C0_P3
    /*
     * CPL == RPL == DPL == 3
     *
     * The procedure in code segment A3 is able to access Stack segment E3
     * using segment selector E3, because the CPL of code segment A3 and the
     * RPL of segment selector E3 are equal to the DPL of Stack segment E3.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment A3 |       | Segment Sel. E3 |       | Stack Segment E3|
     * |                -|------>|                -|------>|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /* Utilize segment selector: 0x03f3
     * Segment Descriptor: 0x00cbf2000000ffff
     * RPL: 03
     * CPL: 03
     * DPL: 03
     */
    Sel  = 0x03f3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into DS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);
    
#endif

#elif defined CONFIG_DEBUG_PL_STACK_C1 /* CONFIG_DEBUG_PL_STACK_C0 */

#ifdef CONFIG_DEBUG_PL_STACK_C1_P0
    /*
     * CPL:0 == RPL:0 > DPL:1
     *
     * The procedure in code segment B0 is not able to access stack segment E0
     * using segment selector E0, because the CPL of code segment B0 and the
     * RPL of segment selector E0 are both numerically lower than (more
     * privileged) the DPL of stack segment E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B0 |       | Segment Sel. E0 |       | Stack Segment E0|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /* Utilize segment selector: 0x0210
     * Segment Descriptor: 0xc0c3b2000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 01
     */
    Sel  = 0x0210;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C1_P1
    /*
     * CPL:0 == RPL:0 > DPL:2
     *
     * The procedure in code segment B1 is not able to access stack segment E1
     * using segment selector E1, because the CPL of code segment B1 and the
     * RPL of segment selector E1 are both numerically lower than (more
     * privileged) the DPL of stack segment E1.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B1 |       | Segment Sel. E1 |       | Stack Segment E1|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0220
     * Segment Descriptor: 0xc0c3d2000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 02
     */
    Sel  = 0x0220;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C1_P2
    /*
     * CPL:0 == RPL:0 > DPL:3
     *
     * The procedure in code segment B3 is not able to access data segment E3
     * using segment selector E3, because the CPL of code segment B3 and the
     * RPL of segment selector E3 are both numerically lower than (more
     * privileged) the DPL of stack segment E3.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment B3 |       | Segment Sel. E3 |       | Stack Segment E3|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 0         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0230
     * Segment Descriptor: 0xc0c3f2000000ffff
     * CPL: 00
     * RPL: 00
     * DPL: 03
     */
    Sel  = 0x0230;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_STACK_C2 /* CONFIG_DEBUG_PL_STACK_C1 */

#ifdef CONFIG_DEBUG_PL_STACK_C2_P0
    /*
     * CPL:0 > RPL:1 == DPL:1
     *
     * The procedure in code segment C0 is not able to access data segment E0
     * using segment selector E0, because the CPL of code segment C0 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E0, and the RPL of segment selector E0 is equal to DPL of stack segment
     * E0.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C0 |       | Segment Sel. E0 |       | Stack Segment E0|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0251
     * Segment Descriptor: 0xc0c3b2000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 01
     */
    Sel  = 0x0251;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C2_P1
    /*
     * CPL:0 > RPL:2 == DPL:2
     *
     * The procedure in code segment C1 is not able to access data segment E1
     * using segment selector E1, because the CPL of code segment C1 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E1, and the RPL of segment selector E1 is equal to DPL of stack segment
     * E1.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C1 |       | Segment Sel. E1 |       | Stack Segment E1|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x02a2
     * Segment Descriptor: 0xc0c3d2000000fffff
     * CPL: 00
     * RPL: 02
     * DPL: 02
     */
    Sel  = 0x02a2;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C2_P2
    /*
     * CPL:0 > RPL:3 == DPL:3
     *
     * The procedure in code segment C2 is not able to access data segment E2
     * using segment selector E2, because the CPL of code segment C2 is
     * numerically lower than (more privilege) then RPL of segment selector
     * E2, and the RPL of segment selector E2 is equal to DPL of stack segment
     * E2.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment C2 |       | Segment Sel. E2 |       | Stack Segment E2|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 3         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x02f3
     * Segment Descriptor: 0xc0c3f2000000ffff
     * CPL: 00
     * PRL: 03
     * DPL: 03 
     */
    Sel  = 0x02f3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                       (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_STACK_C3 /* CONFIG_DEBUG_PL_STACK_C2 */

#ifdef CONFIG_DEBUG_PL_STACK_C3_P2
    /*
     * CPL3 == RPL:3 < DPL:0
     *
     * The procedure in code segment D2 is not able to access stack segment E2
     * using segment selector E2, because the CPL of code segment D2 and the
     * RPL of segment selector E2 are both numerically greater than (less
     * privilege) the DPL of stack segment E2. (The CPL of code segmetn D2 is
     * equal to RPL of segment selecotr E2).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D2 |       | Segment Sel. E2 |       | Stack Segment E2|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x03C3
     * Segment Descriptor: 0x00cb92000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 00
     */
    Sel  = 0x03C3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C3_P4
    /*
     * CPL:3 == RPL:3 < DPL:1
     *
     * The procedure in code segment D4 is not able to access stack segment E4
     * using segment selector E4, because the CPL of code segment D4 and the
     * RPL of segment selector E4 are both numerically greater than (less
     * privilege) the DPL of stack segment E4. (The CPL of code segmetn D4 is
     * equal to RPL of segment selecotr E4).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D4 |       | Segment Sel. E4 |       | Stack Segment E4|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x03d3
     * Segment Descriptor: 0x00cbb2000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 01
     */
    Sel  = 0x03d3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C3_P5
    /*
     * CPL:3 == RPL:3 < DPL:2
     *
     * The procedure in code segment D5 is not able to access stack segment E5
     * using segment selector E5, because the CPL of code segment D5 and the
     * RPL of segment selector E5 are both numerically greater than (less
     * privilege) the DPL of stack segment E5. (The CPL of code segmetn D5 is
     * equal to RPL of segment selecotr E5).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment D5 |       | Segment Sel. E5 |       | Stack Segment E5|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 3         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x03e3
     * Segment Descriptor: 0x00cbd2000000ffff
     * CPL: 03
     * RPL: 03
     * DPL: 02
     */
    Sel  = 0x03e3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_STACK_C4 /* CONFIG_DEBUG_PL_STACK_C3 */

#ifdef CONFIG_DEBUG_PL_STACK_C4_P2
    /*
     * CPL:3 < RPL:0 == DPL:0
     *
     * The procedure in code segment E2 is not able to access stack segment E2
     * using segment selector E2, because the CPL of code segment E2 both are
     * numerically greater than (less privilege) the RPL of segment selector
     * E2 and the DPL of the stack segment E2. (The RPL of segment selector E2
     * is equal to DPL of the stack segment E2).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E2 |       | Segment Sel. E2 |       | Stack Segment E2|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 0         |       | DPL = 0         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0300
     * Segment Desscriptor: 0x00cb92000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 00
     */
    Sel  = 0x0300;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C4_P4
    /*
     * CPL:3 < RPL:1 == DPL:1
     *
     * The procedure in code segment E4 is not able to access stack segment E4
     * using segment selector E4, because the CPL of code segment E4 both are
     * numerically greater than (less privilege) the RPL of segment selector
     * E4 and the DPL of the stack segment E4. (The RPL of segment selector E4
     * is equal to DPL of the stack segment E4).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E4 |       | Segment Sel. E4 |       | Stack Segment E4|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 1         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0351
     * Segment Descriptor: 0x00cbb2000000ffff
     * CPL: 03
     * RPL: 01
     * DPL: 01
     */
    Sel  = 0x0351;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selecotr */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C4_P5
    /*
     * CPL:3 < RPL:2 == DPL:2
     *
     * The procedure in code segment E5 is not able to access stack segment E5
     * using segment selector E5, because the CPL of code segment E5 both are
     * numerically greater than (less privilege) the RPL of segment selector
     * E5 and the DPL of the stack segment E5. (The RPL of segment selector E5
     * is equal to DPL of the stack segment E5).
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment E5 |       | Segment Sel. E5 |       | Stack Segment E5|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 2         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x03a2
     * Segment Descriptor: 0x00cbd2000000ffff
     * CPL: 03
     * RPL: 02
     * DPL: 02
     */
    Sel  = 0x03a2;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_STACK_C5 /* CONFIG_DEBUG_PL_STACK_C4 */

#ifdef CONFIG_DEBUG_PL_STACK_C5_P1
    /*
     * CPL:3 < RPL:0 && RPL:0 > DPL:1
     * CPL:3 < DPL:1 < RPL:0
     *
     * The procedure in code segment F1 is not able to access stack segment E1
     * using segment selector E1. If the CPL of code segment F1 is numerically
     * greate than (less privilege) the DPL of stack segment E1, even the RPL
     * of segment selector E1 is numerically less than (more privilege) the
     * DPL of stack segment E1, the code segment F1 is able to access stack
     * segment E1.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment F1 |       | Segment Sel. E1 |       | Stack Segment E1|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 0         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilze segment selector: 0x0310
     * Segment Descriptor: 0x00cbb2000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 01
     */
    Sel  = 0x0310;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS)) ;
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C5_P2
    /*
     * CPL:3 < RPL:0 && RPL:0 > DPL:2
     * CPL:3 < DPL:2 < RPL:0
     *
     * The procedure in code segment F3 is not able to access stack segment E3
     * using segment selector E3. If the CPL of code segment F3 is numerically
     * greate than (less privilege) the DPL of stack segment E3, even the RPL
     * of segment selector E3 is numerically less than (more privilege) the DPL
     * of stack segment E3, the code segment F3 is able to access stack segment
     * E3.
     *
     *
     * +-----------------+       +-----------------+        +-----------------+
     * | Code Segment F3 |       | Segment Sel. E3 |        | Stack Segment E3|
     * |                -|------>|                -|---X--->|                 |
     * | CPL = 3         |       | RPL = 0         |        | DPL = 2         |
     * +-----------------+       +-----------------+        +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0320
     * Segment Descriptor: 0x00cbd2000000ffff
     * CPL: 03
     * RPL: 00
     * DPL: 02
     */
    Sel  = 0x0320;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS regsiter and trigger privilege 
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C5_P3
    /*
     * CPL:3 < RPL:1 && RPL:1 > DPL:2
     * CPL:3 < DPL:2 < RPL:1
     *
     * The procedure in code segment F2 is not able to access stack segment E2
     * using segment selector E2. If the CPL of code segment F2 is numerically
     * greate than (less privilege) the DPL of stack segment E2, even the RPL
     * of segment selector E2 is numerically less than (more privilege) the
     * DPL of stack segment E2, the code segment F2 is able to access stack
     * segment E2.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment F2 |       | Segment Sel. E2 |       | Stack Segment E2|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 3         |       | RPL = 1         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0361
     * Segment Descriptor: 0x00cbd2000000ffff
     * CPL: 03
     * RPL: 01
     * DPL: 02
     */ 
    Sel  = 0x0361;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printf("SS:  %#x -- Char: %s\n", SS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_STACK_C6 /* CONFIG_DEBUG_PL_STACK_C5 */

#ifdef CONFIG_DEBUG_PL_STACK_C6_P0
    /*
     * CPL:0 > RPL:2 && RPL:2 < DPL:1
     * CPL:0 > DPL:1 > RPL:2
     *
     * The procedure in code segment G0 should be able to access stack segment
     * E0 because code segment D's CPL is numerically less than the DPL of
     * stack segment E0. However, the RPL of segment selector E0 (which the
     * code segment G0 procedure is using to access data segment E0) is
     * numerically greater than the DPL of stack segment E0, so access is not
     * allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment G0 |       | Segment Sel. E0 |       | Stack Segment E0|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /* 
     * Utilize segment selector: 0x0292
     * Segment Descriptor: 0xc0c3b2000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 01
     */
    Sel  = 0x0292;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                  (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C6_P1
    /*
     * CPL:0 > RPL:3 && RPL:3 < DPL:1
     * CPL:0 > DPL:1 > RPL:3
     *
     * The procedure in code segment G1 should be able to access stack segment
     * E1 because code segment D's CPL is numerically less than the DPL of
     * stack segment E1. However, the RPL of segment selector E1 (which the
     * code segment G1 procedure is using to access stack segment E1) is
     * numerically greater than the DPL of stack segment E1, so access is not
     * allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment G1 |       | Segment Sel. E1 |       | Stack Segment E1|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 3         |       | DPL = 1         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x02d3
     * Segment Descriptor: 0xc0c3b2000000ffff
     * CPL: 00
     * RPL: 03
     * DPL: 01
     */
    Sel  = 0x02d3;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                        (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * Checking */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#endif

#elif defined CONFIG_DEBUG_PL_STACK_C7 /* CONFIG_DEBUG_PL_STACK_C6 */

#ifdef CONFIG_DEBUG_PL_STACK_C7_P0
    /*
     * CPL:0 > RPL:1 > DPL:2
     *
     * The procedure in code segment H0 should be not able to access stack
     * segment E0 because code segment D's CPL is numerically less than the
     * DPL of stack segment E0. However, the RPL of segment selector E0 (which
     * the code segment G0 procedure is using to access stack segment E0) is
     * numerically less than the DPL of stack segment E0, and the CPL of code
     * segment G0 is numerically less than the RPL of segment selector E0, so
     * access is allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H0 |       | Segment Sel. E0 |       | Stack Segment E0|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 2         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0261
     * Segment Descriptor: 0xc0c3d2000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 02
     */
    Sel  = 0x0261;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                                (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#elif defined CONFIG_DEBUG_PL_STACK_C7_P1
    /*
     * CPL:0 > RPL:1 > DPL:3
     *
     * The procedure in code segment H3 should be not able to access stack
     * segment E3 because code segment D's CPL is numerically less than the
     * DPL of data segment E3. However, the RPL of segment selector E3 (which
     * the code segment G3 procedure is using to access stack segment E3) is
     * numerically less than the DPL of stack segment E3, and the CPL of code
     * segment G3 is numerically less than the RPL of segment selector E3, so
     * access is allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H3 |       | Segment Sel. E3 |       | Stack Segment E3|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 1         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x0271
     * Segment Descriptor: 0xc0c3f2000000ffff
     * CPL: 00
     * RPL: 01
     * DPL: 03
     */
    Sel  = 0x0271;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel,
                   (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);
 
#elif defined CONFIG_DEBUG_PL_STACK_C7_P2
    /*
     * CPL:0 > RPL:2 > DPL:3
     *
     * The procedure in code segment H1 is not able to access stack segment
     * E1 because code segment D's CPL is numerically less than the DPL of
     * stack segment E1. However, the RPL of segment selector E1 (which the
     * code segment G1 procedure is using to access stack segment E1) is
     * numerically less than the DPL of stack segment E1, and the CPL of code
     * segment G1 is numerically less than the RPL of segment selector E1, so
     * access is allowed.
     *
     *
     * +-----------------+       +-----------------+       +-----------------+
     * | Code Segment H1 |       | Segment Sel. E1 |       | Stack Segment E1|
     * |                -|------>|                -|---X-->|                 |
     * | CPL = 0         |       | RPL = 2         |       | DPL = 3         |
     * +-----------------+       +-----------------+       +-----------------+
     *
     */
    /*
     * Utilize segment selector: 0x02b2
     * Segment Descriptor: 0xc0c3f2000000ffff
     * CPL: 00
     * RPL: 02
     * DPL: 03
     */
    Sel  = 0x02b2;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printk("Sel: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                         (unsigned int)desc->b, (unsigned int)desc->a);
    printk("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);
    /* Load a new stack segment selector into SS register and trigger privilege
     * checking. */
    __asm__ ("mov %0, %%ss" :: "r" (Sel));
    /* Store stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("SS:  %#x -- Char: %s\n", SS, hello);

#endif

#endif /* CONFIG_DEBUG_PL_STACK_C7 */

    return 0;
}
#endif

#if defined CONFIG_DEBUG_PL_STACK_C0_P0 | \
    defined CONFIG_DEBUG_PL_STACK_C1_P0 | \
    defined CONFIG_DEBUG_PL_STACK_C1_P1 | \
    defined CONFIG_DEBUG_PL_STACK_C1_P2 | \
    defined CONFIG_DEBUG_PL_STACK_C2_P0 | \
    defined CONFIG_DEBUG_PL_STACK_C2_P1 | \
    defined CONFIG_DEBUG_PL_STACK_C2_P2 | \
    defined CONFIG_DEBUG_PL_STACK_C6_P0 | \
    defined CONFIG_DEBUG_PL_STACK_C6_P1 | \
    defined CONFIG_DEBUG_PL_STACK_C7_P0 | \
    defined CONFIG_DEBUG_PL_STACK_C7_P1 | \
    defined CONFIG_DEBUG_PL_STACK_C7_P2
late_debugcall(privilege_check_stack_segment);
#elif defined CONFIG_DEBUG_PL_STACK_C0_P3 | \
      defined CONFIG_DEBUG_PL_STACK_C3_P2 | \
      defined CONFIG_DEBUG_PL_STACK_C3_P4 | \
      defined CONFIG_DEBUG_PL_STACK_C3_P5 | \
      defined CONFIG_DEBUG_PL_STACK_C4_P2 | \
      defined CONFIG_DEBUG_PL_STACK_C4_P4 | \
      defined CONFIG_DEBUG_PL_STACK_C4_P5 | \
      defined CONFIG_DEBUG_PL_STACK_C5_P1 | \
      defined CONFIG_DEBUG_PL_STACK_C5_P2 | \
      defined CONFIG_DEBUG_PL_STACK_C5_P3
user1_debugcall_sync(privilege_check_stack_segment);
#endif

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
