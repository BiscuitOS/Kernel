/*
 * IDT (Interrupt Descriptor Table)
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
 * Parse IDTR
 *   The IDTR register holds the base address (32 bits in protected mode).
 *   and 16-bit table limit for the IDT. The base address specifies the 
 *   linear address of byte 0 of the IDT. The table limit specifies the 
 *   number of bytes in the table.
 *
 *   The LIDT and SIDT instructions load and store the IDTR register, 
 *   resoectively. On power up or reset of the processor, the base address
 *   is set to the default value of 0 and the limit is set to 0xFFFFH. The
 *   base address and limit in the register can then be changed as part
 *   of the processor initialization processor.
 *
 *   47------------------------------16-15-------------------------0
 *   | 32-bit Linear Base Address      | 16-Bit Table limit        |
 *   ---------------------------------------------------------------
 */
static int parse_idtr(unsigned short *limit, unsigned long *base)
{
    unsigned char idtr[6];

    /*
     * SIDT -- Store Interrupt Descriptor Table Register
     *   Store tht content the interrupt descriptor table register (IDTR)
     *   in the destination operand. The desctination operand specifies
     *   a 6-byte memory location.
     */
    __asm__ ("sidt %0"
             : "=m" (idtr));

    /*
     * IF instruction is SIDT
     *   THEN
     *     DEST[0:15]  <- IDTR(limit)
     *     DEST[16:47] <- IDTR(Base)
     * FI
     */
    *limit = idtr[0] | (idtr[1] << 8);
    *base  = idtr[2] | (idtr[3] << 8) | (idtr[4] << 16) | (idtr[5] << 24);

    return 0;
}

/*
 * IDT Descriptors
 *   The IDT may contain any of three kind of gate descriptors:
 *   -> Task-gate descriptor
 *   -> Interrupt-gate descriptor
 *   -> Trap-gate descriptor
 *   Figure show the formats for the task-gate, interrupt-gate, and trap
 *   descriptors. The format of a task gate used in an IDT is the same
 *   as that of a task gate used in the GDT or an LDT. The task gate 
 *   contains the segment selector for a TSS for an exception and/or
 *   interrupt handler task.
 *
 *   Interrupt and trap gates are very similar to call gates. They contain
 *   a far pointer (segment selector and offset) that the processor uses
 *   to transfer program execution to a handler procedure in an exception-
 *   or interrupt-handler code segment. These gates differ in the way the
 *   processor handles the IF flag in the EFLAGS register.
 *
 *   Task Gate
 *   31---------------------------16-15--14---13-12--------8-7----------0
 *   |                              | P |  DPL  | 0 0 1 0 1 |           | 4
 *   --------------------------------------------------------------------
 *   31---------------------------16-15----------------------------------
 *   | TSS Segment Selector         |                                   | 0
 *   --------------------------------------------------------------------
 *
 *   Interrupt Gate
 *   31---------------------------16-15--14---13-12--------8-7-----5-4--0
 *   | Offset 21:16                 | P | DPL   | 0 D 1 1 0 | 0 0 0 |   | 4
 *   --------------------------------------------------------------------
 *   31---------------------------16-15---------------------------------0
 *   | Segment Selector             | Offset 15:0                       | 0
 *   --------------------------------------------------------------------
 *
 *   Trap Gate
 *   31---------------------------16-15--14---13-12--------8-7-----5-4--0
 *   | Offset 31:16                 | P | DPL   | 0 D 1 1 1 | 0 0 0 |   | 4
 *   --------------------------------------------------------------------
 *   31---------------------------16-15---------------------------------0
 *   | Segment Selector             | Offset 15:0                       | 0
 *   -------------------------------------------------------------------0
 */
static struct gate_desc *gate_descriptors(unsigned long vect)
{
    unsigned short limit;
    unsigned long base;
    unsigned char *gate;
    struct desc_node *idt;
    struct gate_desc *desc;

    /* get IDTR limit and base information */
    parse_idtr(&limit, &base);
    /* get IDT location on Memory */
    idt = (struct desc_node *)base;
    /* get specify gate in IDT by vect */
    gate = (unsigned char *)(unsigned long)&idt[vect];
    /* allocate memory for struct gate_desc */
    desc = (struct gate_desc *)get_free_page();
 
    /*
     * S (descriptor) flag
     *   Specify whether the segment descriptor is for a system segment(S
     *   flag is clear) or code or data segment (S flag is set). Gate on
     *   IDT, this bit must set to zero.
     * 
     *   desc->flag:0
     */
    desc->flag |= (gate[5] >> 0x4) & 0x1;

    /*
     * P (segment-present) flag
     *   Indicates whether the gate is present in memory (set) or not
     *   present (clear). If this flag is clear, the processor generates
     *   a segment-not-present exception(#NP) when a vector that
     *   points to the gate descriptor is loaded into a segment register.
     *   Memory management software can use this flag to control which 
     *   segments are actully loaded into physical memory at a given time.
     *   It offers a control in a addition to paging for managing vitual
     *   memory.
     *
     *   When this flag is clear, the operating system or executive is free
     *   to use the locations marked "Available" to store its own data, such
     *   as information regarding the whereabouts of the missing segment.
     *
     *   desc->flag:1
     */
    desc->flag |= (gate[5] >> 0x7) & 0x1;

    /*
     * Type field
     *   Indicates the segment or gate type and specifies the kind of 
     *   access that can be made to the segment and the direction of 
     *   growth. The interpretation of this field depends on whether the 
     *   descriptor type flag specifies an application (code or data)
     *   descriptor or a system descriptor. The encoding of the type field
     *   is different for code, data, and system descriptors.
     */
    desc->type = gate[5] & 0xF;

    /*
     * DPL (Descriptor privilege level) field
     *   Specifies the privilege level of the segment. The privilege level
     *   can range from 0 to 3, with 0 being the most privilege level.
     *   The DPL is used to control access to the segment.
     */
    desc->dpl = (gate[5] >> 0x5) | 0x3;

    /* Gate selector */
    desc->sel = gate[2] | (gate[3] << 8);    

    /* Task Gate */
    if (desc->type == 0x5) {
        desc->offset = -1; /* no exist */
    } else if ((desc->type & 0x7) == 0x6 || (desc->type & 0x7) == 0x7) {
        /* Interrupt Gate and Trap Gate */
        desc->offset = gate[0] | (gate[1] << 8) | 
                       (gate[6] << 16) | (gate[7] << 24);
    } else {
        printk("Unknown Gate type!\n");
        free_page((unsigned long)desc);
        return NULL;
    }

    return desc;
}

/* Debug Gate on IDT */
void debug_idt_segment_desc_common(void)
{
    /* Add debug item */

    /* ignore warning */
    if (1) {
        struct gate_desc *desc;

        desc = gate_descriptors(3);
        if (desc)
            free_page((unsigned long)desc);
    }
}
