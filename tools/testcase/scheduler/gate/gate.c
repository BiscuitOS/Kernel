/*
 * System Descriptor
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>

#include <test/debug.h>

/*
 * When the S (descriptor type) flag in a segment descriptor is clear,
 * the descriptor type is a system descriptor. The processor recognizes
 * the following types of system descriptors:
 *
 * -> Local descriptor-table (LDT) segment descriptor.
 * -> Task-state segment (TSS) descriptor.
 * -> Call-gate descriptor
 * -> Interrupt-gate descriptor
 * -> Trap-gate descriptor
 * -> Task-gate descrrptor
 *
 * These descriptor types fall into two catagories: system-segment 
 * descriptor and gate descriptor. System-segment descriptor point to
 * system segment (LDT and TSS segments). Gate descriptors are in themselves
 * "gates", which hold pointers to procedure entry points in code segments
 * (call, interrupt, and trap gates) or which hold segment selectors for
 * TSS's (task gates).
 */

/*
 * System descriptor type
 *   Table shows the encoding of the type field for system-segment descriptor
 *   and gate descriptors. Note that system descriptors in IA-32 mode are
 *   16 bytes instead of 8 bytes.
 *
 * ------------------------------------------------------------------------
 * |     Type field        |           Description                        |    
 * ------------------------------------------------------------------------
 * | Dec | 11 | 10 | 9 | 8 |             32-bit Mode                      |
 * ------------------------------------------------------------------------
 * |  0  | 0  |  0 | 0 | 0 | Reserved                                     |    
 * ------------------------------------------------------------------------
 * |  1  | 0  |  0 | 0 | 1 | 16-bit TSS (Available)                       |    
 * ------------------------------------------------------------------------
 * |  2  | 0  |  0 | 1 | 0 | LDT                                          |    
 * ------------------------------------------------------------------------
 * |  3  | 0  |  0 | 1 | 1 | 16-bit TSS (Busy)                            |    
 * ------------------------------------------------------------------------
 * |  4  | 0  |  1 | 0 | 0 | 16-bit Call Gate                             |    
 * ------------------------------------------------------------------------
 * |  5  | 0  |  1 | 0 | 1 | Task Gate                                    |    
 * ------------------------------------------------------------------------
 * |  6  | 0  |  1 | 1 | 0 | 16-bit Interrupt Gate                        |    
 * ------------------------------------------------------------------------
 * |  7  | 0  |  1 | 1 | 1 | 16-bit Trap Gate                             |    
 * ------------------------------------------------------------------------
 * |  8  | 1  |  0 | 0 | 0 | Reserved                                     |    
 * ------------------------------------------------------------------------
 * |  9  | 1  |  0 | 0 | 1 | 32-bit TSS Avaiable                          |    
 * ------------------------------------------------------------------------
 * | 10  | 1  |  0 | 1 | 0 | Reserved                                     |    
 * ------------------------------------------------------------------------
 * | 11  | 1  |  0 | 1 | 1 | 32-bit TSS (Busy)                            |    
 * ------------------------------------------------------------------------
 * | 12  | 1  |  1 | 0 | 0 | 32-bit Call Gate                             |    
 * ------------------------------------------------------------------------
 * | 13  | 1  |  1 | 0 | 1 | Reserved                                     |    
 * ------------------------------------------------------------------------
 * | 14  | 1  |  1 | 1 | 0 | 32-bit Interrupt Gate                        |    
 * ------------------------------------------------------------------------
 * | 15  | 1  |  1 | 1 | 1 | 32-bit Trap Gate                             |
 * ------------------------------------------------------------------------    
 *
 * @desc: segment descriptor
 *
 * @return: 0 is correct.
 *          1 is data or code segment descriptor
 */
static int system_gate_type(struct seg_desc *desc)
{
    if (desc->flag & 0x01)
        return 1;

    switch (desc->type) {
    case 1:
        printk("16-bit TSS (Available)\n");
        break;
    case 2:
        printk("LDT\n");
        break;
    case 3:
        printk("16-bit TSS (Busy)\n");
        break;
    case 4:
        printk("16-bit Call Gate\n");
        break;
    case 5:
        printk("Task Gate\n");
        break;
    case 6:
        printk("16-bit Interrupt Gate\n");
        break;
    case 7:
        printk("16-bit Trap Gate\n");
        break;
    case 9:
        printk("32-bit TSS (Available)\n");
        break;
    case 11:
        printk("32-bit TSS (Busy)\n");
        break;
    case 12:
        printk("32-bit Call Gate\n");
        break;
    case 14:
        printk("32-bit Interrupt Gate\n");
        break;
    case 15:
        printk("32-bit Trap Gate\n");
        break;
    default:
        /* Reserved */
        break;
    }
    return 0; 
}

/*
 * Tss Descriptor
 *   The TSS, like all other segments, is defined by a segment descriptor.
 *   Figure show the format of a TSS descriptor. TSS descriptors may only
 *   be placed in the GDT. They cannot be placed in an LDT or IDT.
 *
 *   An attempt to access a TSS using a segment selector with its TI flag
 *   set (which indicates the current LDT) causes a general-protection 
 *   exception (#GP) to be generated during CALLs and JMPs. it causes an
 *   invalid TSS exception (#TS) during IRETs. A general-protection
 *   exception is also generated if an attempt is made to load a segment
 *   selector for a TSS into a segment register.
 *
 * 31---------24----------------------16---------11-8-7------------------0
 * | Base 21:24 |G|0|0|0|AVL|Limit 19:16|P|DPL|0|Type| Base 23:16        |
 * 31---------------------------------16---------------------------------0
 * | Base 15:00                         | Segment limit 15:00            |
 * ----------------------------------------------------------------------- 
 */
static void parse_TSS_segment(void)
{
    unsigned short tr;
    struct seg_desc *desc;
    struct tss_struct *ts;

    /*
     * Find a busy or inactive task
     * The busy flag (B) in the type field indicates whether the task is
     * busy. A busy task is currently running or suspended. A type field
     * with a value of 1001B indicates an inactive task. a value of 1011B
     * indicates a busy task. Task are not recursive. The processor uses
     * the busy flag to detect an attempt to call a task whose execution
     * has been interrupted. To insure that there is only one busy flag is
     * associated whith a task, each TSS should have only one TSS descriptor
     * that points to it.
     */
    __asm__ ("str %0"
             : "=m" (tr));

    desc = segment_descriptors(tr);

    system_gate_type(desc);

    /*
     * The base, limit, and DPL fields and the granularity and present flags
     * have functions similar to their use in data-segment descriptions. When
     * the G flag is 0 in a TSS descriptor for a 32-bit TSS, the limit field
     * must have a value eqular to or greated than 67H, one byte less than
     * the minmum size of a TSS. Attempting to switch to a task whose TSS
     * descriptor has a limit less than 67H generates an invalid-TSS 
     * exception (#TS). A larger limit is required if an I/O permission bit
     * map is included or if the operating system stores additional data.
     * The processor does not check for a limit greater than 67H on a task
     * switch. However, it does check when accessing the I/O permission bit
     * map or interrupt redirection bit map.
     */
    printk("G bit: %d\n", desc->flag >> 0x03);
    printk("Limit: %#x\n", desc->limit);

    /*
     * Any program or procedure with access to a TSS descriptor (that is,
     * whose CPL is numerically equal to or less than the DPL of the TSS
     * descriptor) can dispatch the task with a call or a jump.
     */
    printk("DPL: %#x\n", desc->dpl);
    printk("CPL: %#x\n", segment_descriptor_cpl());

    /*
     * In most system, the DPLs of TSS descriptors are set to values less
     * than 3, so that only privileged software can perform task switching.
     * However, in multitasking applications, DPLs for some TSS descriptors
     * may be set to 3 to allow task switching at the application (or user)
     * privilege level. 
     */

    if ((desc->flag >> 0x1) & 0x1)
        printk("TSS present on Main-Memory\n");
    printk("Base: %#x\n", desc->base);

    /* parse struct tss_struct */
    ts = (struct tss_struct *)desc->base;
    printk("cs %#x\n", ts->cs);

    /* done */
    free_page((unsigned long)desc);
}

/* common debug system descriptor entry */
void debug_system_descriptor_common(void)
{
    /* add test item */

    /* ignore warning for un-unsed */
    if (0) {
        parse_TSS_segment();
    }
}
