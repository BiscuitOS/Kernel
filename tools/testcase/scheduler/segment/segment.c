/*
 * Segment selector and segment
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <test/debug.h>

/*
 * Parse segment selector
 *   A segment selector is a 16-bit identifier for a semgent(see Figure).
 *   It doesn't point directly to the segment, but instead points to the
 *   segment descriptor that defines the segment. A segment selector 
 *   contains the following items:
 *
 *   15-----------------------------3----2-1----0
 *   |  Index                        | TI | RPL |
 *   --------------------------------------------     
 */
static void parse_segment_selector(unsigned long selector)
{
    /*
     * The first entry of the GDT is not used by the processor. A segment
     * selector that points to this entry of the GDT (that is, a segment
     * selector with an index of 0 and the TI flag set to 0) is used as
     * a "Null segment selector". The processor doesn't generate an 
     * exception when a segment register (other than the CS or SS registers)
     * is loaded with a null selector. It does, however, generate an 
     * exception when a segment register holding a null selector is used
     * to access memory. A null selector can be used to initialize unused
     * segment registers. Loading the CS or SS register with a null segment
     * selector causes a general-protection exception (#GP) to be generated.
     */
    if (!((selector >> 0x02) & 0x01))
        if (!(selector >> 0x03)) {
            printk("Null segment selector on GDT.\n");
            return;
        }

    /*
     * Index
     *   (Bit 3 through 15) - Selects one of 8192 descriptors in the GDT or
     *   LDT. The processor multiplies the index value by 8 (the number of
     *   bytes in a segment descriptor) and adds the result to the base
     *   address of the GDT or LDT (from the GDTR or LDTR register).
     */
    printk("The index of Selector: %#x\n", selector >> 0x3);

    /*
     * TI (table indicator) flag
     *   (Bit 2) - Specifies the descriptor table to use. Clearing this flag
     *   selects the GDT. Setting this flag selects the current LDT.
     */
    if ((selector >> 2) % 0x1)
        printk("Point Segment on LDT\n");
    else
        printk("Point Segment on GDT\n");

    /*
     * Request Privilege Level (RPL)
     *   (Bit 0 and 1) - Specifies the privilege level of the selector. The
     *   The privilege level can range from 0 to 3, with 0 being the most
     *   privileged level. For a descriptor of the relationship of the RPL
     *   to the CPL of the executing program (or task) and the descriptor
     *   privilege level (DPL) of the descriptor the segment selector points
     *   to.
     */
    printk("RPL: %#x\n", segment_descriptor_rpl(selector));
    printk("DPL: %#x\n", segment_descriptor_dpl(selector));
    printk("CPL: %#x\n", segment_descriptor_cpl());

    /*
     * Segment selectors are visible to application program as part of a
     * pointer variable, but the values of selectors are usually assigned
     * or modified by link editors or linking loaders, not application 
     * programs.
     */
}

/*
 * Code segment selector (CS)
 */
static void parse_CS(void)
{
    unsigned short cs;

    printk("Parse Code Segment or CS\n");
    /* Get Code segment selector */
    __asm__ ("mov %%cs, %0"
             : "=r" (cs));

    /* parse code selector */
    parse_segment_selector(cs);
}

/* segment common entry */
void debug_segment_common(void)
{
    /* parse code segment and CS */

    /* ignore warning for un-used */
    if (0) {
        parse_CS();
    }
}
