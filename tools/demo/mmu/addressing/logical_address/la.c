/*
 * LA: logical address
 *
 * (C) 2018.10.18 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <demo/debug.h>

/*
 * Segment selector
 *   DS: Data segment selector
 *   CS: Code segment selector
 *   SS: Stack segment selector
 *   FS: Userland data segment selector
 *
 * A segment selector is a 16-bit identifier for a segment. It does not point
 * directly to the segment, but instead points to the segment descriptor that
 * defines the segment. A segment selector contains the following items.
 * 
 * * Index  (Bit3 through 15) -- Selects one of 8192 descriptor in the GDT
 *          or LDT. The processor multiplies the index value by 8 (the number
 *          of bytes in a segment descriptor) and adds the result to the base
 *          address of the GDT or LDT (from the GDTR or LDTR register, 
 *          respectively).
 *
 * * TI (Table indicator) flag
 *          (Bit2) -- Specifies the descriptor table to use: Clearing this
 *          flag selects the GDT; Setting this flag selects the current LDT.
 *   
 * * Requested Privilege Level (RPL)
 *          (Bit 0 and 1) -- Specifies the privilege level of the selector.
 *          The privilege level can range from 0 to 3, with 0 being the most
 *          privilegel level.    
 *
 * The layout of segment selector
 *
 * 15
 * +------------------------------------+----+-----+
 * |      Index                         | TI | RPL |
 * +------------------------------------+----+-----+
 */
static int seg_sel_endence(void)
{
    unsigned short __unused DS;
    unsigned short __unused CS;
    unsigned short __unused SS;
    unsigned short __unused FS;
    unsigned long __unused base;
    struct  desc_struct __unused *desc;

#ifdef CONFIG_DEBUG_LA_KERNEL

#ifdef CONFIG_DEBUG_LA_SEL_DS
    /* Data segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (DS));

    if (DS & 0x4) {
        /* Segment descriptor locate on LDT */
        desc = current->ldt + (DS >> 0x3);
    } else {
        /* Segment descriptor locate on GDT */
        desc = gdt + (DS >> 0x3);
    }
    base = get_base(*desc);
    printk("Data Sel: %#x, Seg base address:   %#x\n", DS, (unsigned int)base);
#endif
#ifdef CONFIG_DEBUG_LA_SEL_CS
    /* Code segment selector */
    __asm__ ("mov %%cs, %0" : "=m" (CS));
   
    if (CS & 0x4) {
        /* Segment descriptor locate on LDT */
        desc = current->ldt + (CS >> 0x3);
    } else {
        /* Segment descriptor locate on GDT */
        desc = gdt + (CS >> 0x3);
    }
    base = get_base(*desc);
    printk("Code Sel: %#x, Seg base address:   %#x\n", CS, (unsigned int)base);
#endif
#ifdef CONFIG_DEBUG_LA_SEL_SS
    /* Stack segment selector */
    __asm__ ("mov %%ss, %0" : "=m" (SS));

    if (SS & 0x4) {
        /* Stack Segment descriptor locate on LDT */
        desc = current->ldt + (SS >> 0x3);
    } else {
        desc = gdt + (SS >> 0x3);
    }
    base = get_base(*desc);
    printk("Stack Sel: %#x, Seg base address:  %#x\n", SS, (unsigned int)base);
#endif
#ifdef CONFIG_DEBUG_LA_SEL_FS
    /* Userland data segment selector */
    __asm__ ("mov %%fs, %0" : "=m" (FS));

    if (FS & 0x4) {
	/* Userland data segment descriptor locate on LDT */
        desc = current->ldt + (FS >> 0x3);
    } else {
        /* Userland data segment descriptor locate on GDT */
        desc = gdt + (FS >> 0x3);
    }
    base = get_base(*desc);
    printk("UData Sel: %#x, Seg base address:  %#x\n", FS, (unsigned int)base);
#endif

#endif
    return 0;
}

/*
 * The logcial-address of various variable
 *
 *  * Local variable
 *  * Global variable
 *  * Instruction
 *  * Stack variable
 */
unsigned long __unused global_variable;

static int variable_logical_endence(void)
{
    unsigned long __unused local_variable;
    unsigned long __unused stack_variable;
    unsigned long __unused instruction;
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;

#ifdef CONFIG_DEBUG_LA_LOCAL
    /*
     * The logical-address of local variable.
     *   The local variable locate on local stack, and local stack locate on
     *   global stack, so the logical-address of local varibale contains a
     *   stack segment selector and a 32-bit offset, as Figure:
     *
     * Local variable
     * 
     * 15(Stack segment selector)0    31                                  0
     * +--------------+----+-----+   +-----------------------------------+
     * |    Index     | TI | RPL | : |             Offset                |
     * +--------------+----+-----+   +-----------------------------------+
     * 
     */
     __asm__ ("mov %%ss, %0" : "=m" (Sel));
     printk("Local variable logical-address:     %#x:%#x\n", 
                  (unsigned int)Sel, (unsigned int)&local_variable);
#endif

#ifdef CONFIG_DEBUG_LA_GLOBAL
    /*
     * The logical-address of global variable.
     *   The global variable locate on data segment, so the logical-address
     *   of global variable contains a data segment selector and a 32-bit
     *   offset, as Figure:
     *
     * Global variable
     * 
     * 15 (Data Segment selecotr)0   31                                  0
     * +--------------+----+-----+   +-----------------------------------+
     * |    Index     | TI | RPL | : |             Offset                |
     * +--------------+----+-----+   +-----------------------------------+
     * 
     */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));
    printk("Global variable logical-address:    %#x:%#x\n",
                (unsigned int)Sel, (unsigned int)&global_variable);
#endif

#ifdef CONFIG_DEBUG_LA_STACK
    /*
     * The logical-address of stack variable.
     *   The stack variable locate on stack, so the logical-address of stack
     *   variable contains a stack segment selector and a 32-bit offset, as
     *   Figure:
     *
     * Stack variable
     * 
     * 15(Stack segment selector)0    31                                  0
     * +--------------+----+-----+   +-----------------------------------+
     * |    Index     | TI | RPL | : |             Offset                |
     * +--------------+----+-----+   +-----------------------------------+
     * 
     */
    __asm__ ("mov %%ss, %0\n\r"
             "mov %%esp, %1\n\r"
             : "=m" (Sel), "=m" (stack_variable));
    printk("Stack variable logical-address:     %#x:%#x\n",
                  (unsigned int)Sel, (unsigned int)stack_variable);
#endif

#ifdef CONFIG_DEBUG_LA_CODE
    /*
     * The logical-address of instruction
     *   The instruction locate on Code segment, so the logical-address of 
     *   instruction contains a code segment selector and a 32-bit offset,
     *   as Figure:
     *
     * Instruction
     * 
     * 15 (Code segment selector)0    31                                  0
     * +--------------+----+-----+   +-----------------------------------+
     * |    Index     | TI | RPL | : |             Offset                |
     * +--------------+----+-----+   +-----------------------------------+
     * 
     */
    __asm__ ("mov %%cs, %0\n\r"
             "call SubPro\n\r"
             "popl %%eax\n\r"
             "movl %%eax, %1\n\r"
             "jmp out\n\r"
             "SubPro:ret\n\r"
             "out:"
             : "=m" (Sel), "=m" (instruction));
    printk("Instruction logical-address:        %#x:%#x\n",
                  (unsigned int)Sel, (unsigned int)instruction);
#endif
    return 0;
}

static int translation_address(void)
{
    unsigned short __unused Sel;
    unsigned long __unused offset;
    struct desc_struct __unused *desc;
    unsigned long __unused virtual;
    unsigned long __unused linear;
    unsigned long __unused base, limit;
    const char __unused *demo = "Hello BiscuitOS";

#ifdef CONFIG_DEBUG_VA_2_LA
    /*
     * Translate virtual address to logical address
     *
     * The application or kernel program access each byte in program's address 
     * space with a **virtual address**. A **virtual address** contains a 
     * bit-width with machine offset from 0 to maximum address. Linux 32-bit 
     * allows splitting the user and kernel address ranges in different ways: 
     * 3G/1G user/kernel.
     *
     * To translate a virtual address into a logical address, the processor 
     * does the following:
     *
     * 1. Find a segment selector. If virtual address points to kernel data 
     *    segment, processor will select DS as segment selector; If virtual 
     *    address points to code segment, processor will select CS as segment 
     *    selector; If virtual address points to stack, processor will select 
     *    SS as segment selector; If virtual address points to user application 
     *    data, the processor will select FS as segment selector.
     *
     * 2. Use the virtual address as offset for logcial address.
     *
     * Translation as Figure:
     *
     *
     *                             Virtual address
     *
     *                             31                           0
     *                             +----------------------------+
     *                             |                            |
     *                             +----------------------------+
     *                             |
     *                             |
     *                             |
     *                             |
     * Logical address             |
     *                             V
     * 15                      0   31                           0
     * +-----------------------+   +----------------------------+
     * |    Segment Selector   | : |           Offset           |
     * +-----------------------+   +----------------------------+
     *
     */
    /* Virtual address of string "Hello BiscuitOS" */
    virtual = (unsigned long)demo;
    /* 
     * Translate virtual-address to logical address 
     * Becase "Hello BiscuitOS" locate on local stack, and local stack is 
     * part of global stack, so the segment selector of "Hello BiscuitOS"
     * is SS.
     */
    __asm__ ("mov %%ss, %0" : "=m" (Sel));
    offset = virtual;
    printk("Logical-address: %#x: %#x\n", Sel, (unsigned int)offset);
#endif

#ifdef CONFIG_DEBUG_LA_2_VA
    /*
     * Transalte logical address to virtual address
     *
     * A logical address consists of a 16-bit segment selector and a 32-bit 
     * offset. For every application and kernel program, each byte is 
     * accessed on virtual space. It is easy to obtain virtual address from 
     * logical address, processor only cover offset into virtual address, 
     * as Figure.
     *
     *  Logical address
     *
     *  15                      0   31                           0
     *  +-----------------------+   +----------------------------+
     *  |    Segment Selector   | : |           Offset           |
     *  +-----------------------+   +----------------------------+
     *                              |
     *                              |
     *                              |
     *                              |
     *             Virtual address  V
     *                              31                           0
     *                              +----------------------------+
     *                              |                            |
     *                              +----------------------------+
     *
     */
     /* A exist logical-address for "Hello BiscuitOS": Sel.offset */
     
     /* Translate a logical-address to virtual address */
     virtual = offset;
     printk("Virtual-address: %#x\n", (unsigned int)virtual);
#endif

#ifdef CONFIG_DEBUG_LA_2_LNA
    /*
     * Translate logical-address to linear-address
     *
     * The processor translates every logical address into a linear address. 
     * A linear address is a 32-bit address in the processor's linear address
     * space. Like the physical address space, the linear address space is a 
     * flat(unsegmented), 2^32-byte address space, with address ranging from 0 
     * to 0xFFFFFFFFH. The linear address space contains all the segments and 
     * system tables defined for a system.
     *
     * To translate a logical address into a linear address, the processor 
     * does the following:
     *
     * 1. Uses the offset in the segment selector to locate the segment 
     *    descriptor for the segment in the GDT or LDT and reads it into the 
     *    processor. (This step is needed only when a new segment selector is 
     *    loaded into a segment register.)
     *
     * 2. Examines the segment descriptor to check the access right and range 
     *    of the segment to insure that the segment is accessible and theat 
     *    the offset is within the limits of the segment.
     *
     * 3. Adds the base address of the segment from the segment descriptor to 
     *    the offset to form a linear address.
     *
     *
     *  Logical address
     *
     *  15                      0   31                           0
     *  +-----------------------+   +----------------------------+
     *  |    Segment Selector   | : |           Offset           |
     *  +-----------------------+   +----------------------------+
     *    |                                        |
     *    |                                        |
     *    |                                        |
     *    |     Descriptor Table                   |
     *    |    +----------------+                  |
     *    |    |                |                  |
     *    |    |                |                  |
     *    |    |                |                  |
     *    |    |                |                  |
     *    |    |                |                  |
     *    |    |                |                  |
     *    |    |                |                  |
     *    |    +----------------+                  V
     *    |    |    Segment     |                +---+
     *    o--->|   Descriptor  -|--------------->| + |
     *         +----------------+                +---+
     *         |                |                  |
     *         +----------------+                  |
     *                                             |
     *                                             |
     *                                             |
     *                              32             V             0
     *                              +----------------------------+
     *                              |       Linear Address       |
     *                              +----------------------------+
     */

    /* A exist logical address of "Hello BiscuitOS": Sel.offset */

    /* 1. Use segment selector to locate the segment descriptor for the 
     *    segment in the GDT or LDT
     */
    if (Sel & 0x4) {
        /* Segment descriptor locate on LDT */
        desc = current->ldt + (Sel >> 0x3);
    } else {
        /* Segment descriptor locate on GDT */
        desc = gdt + (Sel >> 0x3);
    }
    
    /* 2. Examines the segment descriptor to check the access right and range 
     *    of the segment to insure that the segment is accessible and theat 
     *    the offset is within the limits of the segment.
     */
    limit = get_limit(Sel);
    if (offset > limit)
        panic("The offset of logical-address overflower Segment limit");
    if ((Sel & 0x3) > ((desc->b >> 12) & 0x3))
        panic("Segment selector no right to access segment");

    /* 3. Adds the base address of the segment from the segment descriptor to 
     *    the offset to form a linear address.
     */
    linear = get_base(*desc) + offset;
    printk("Linear-address:  %#x\n", (unsigned int)linear);

#endif

    return 0;
}

static int la_entence(void)
{
    /* Seg. Selector endence */
    seg_sel_endence();

    /* variable endence */
    variable_logical_endence();

    /* Translating for linear, logical, and virtual address */
    translation_address();
    return 0;
}

#ifdef CONFIG_DEBUG_LA_KERNEL
device_debugcall(la_entence);
#elif defined (CONFIG_DEBUG_LA_USERLAND)
user1_debugcall_sync(la_entence);
#endif
