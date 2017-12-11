/*
 * GDT (Global Descriptor Table)
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <test/debug.h>

/*
 * Get base address and limit from GDTR
 *  The LGDT and SGDT instruction load and store the GDTR register,
 *  respectively. On power up or reset of the processor, the base
 *  address is set to the default value of 0 and the limit is set
 *  to 0xffffh. A new base address must be loaded into the GDTR as
 *  part of the processor initialization process for protected-mode
 *  operation.
 *
 *  @limit: the limit for GDT
 *  @base:  the base address of GDT
 *  
 *  @return: the linar address of GDT.
 */
int parse_gdtr(unsigned short *limit, unsigned long *base)
{
    /*
     * GDTR: Global Descriptor Table Register
     * ---------------------------------------------------
     * | 32-bit Linar Base address  | 16-bit Table Limit |
     * ---------------------------------------------------
     */
    unsigned char __base[6];
    /* 
     * SGDT -- Store Global Descriptor Table Register.
     *   Stores the content of the global descriptor table register(GDTR)
     *   in the destination operand. The destination operand specifies a
     *   memory location.
     *
     *   In legacy or compatibility mode, the destination operand is a
     *   6-byte memory location. If the operand-size attribute is 16 or 32
     *   bits, the 16-bit limit field of the register is stored in the low
     *   2 bytes of the memory location and the 32-bit base address is 
     *   stored in the high bytes.
     *   
     *   SGDT is useful only by operating-system software. However, it
     *   can be used in application program without causing an exception
     *   to be generated if CR4.UMIP = 0.
     */
    __asm__ ("sgdt %0"
             : "=m" (__base));
    /*
     * Operation
     *   IF instruction is SGDT
     *     IF OperandSize = 16 or OperandSize = 32
     *       THEN
     *         DEST[0:15]   <- GDTR(Limit)
     *         DEST[16:47]  <- GDTR(Base)
     *       FI
     *     ELSE (*64-bit Mode)
     *         DEST[0:15]   <- GDTR(Limit)
     *         DEST[16:79]  <- GDTR(Base)
     *     FI
     *   FI
     */
    *limit = *(unsigned short *)(unsigned long)__base;
    *base  = *(unsigned long *)(unsigned long)&__base[2];

    return 0;
}

/*
 * Analysic a specify segment descriptors
 *   A segment descriptor is a data structure in a GDT or LDT that
 *   provides the processor with size and location of a segment,
 *   as well as access control and status information. Segment descriptors
 *   are typically created by compilers, links, loaders, or the operating
 *   system or executive, but not application programs. 
 *
 * @selector: The selector of specify segment
 *
 * @return: struction of segment descriptor or NULL.
 */
struct seg_desc *segment_descriptors(unsigned long selector)
{
    /*
     * 31--------24---------20------------15------------------7-----------0
     * | Base 31:24|G|D/B|AVL|segLimt 19:16|P| DPL | S | Type |Base 23:16 |
     * --------------------------------------------------------------------
     * | Base Address 15:0                 | Segment Limit 15:0           |
     * -------------------------------------------------------------------- 
     */
    unsigned short limit;
    unsigned long base;
    unsigned char *seg;
    struct gdt_node *gdt;
    struct seg_desc *desc;

    /*
     * The first segment descriptor on GDT is NULL.
     */
    if (!(selector >> 0x3))
        return NULL;

    if (!((selector >> 0x2) & 0x1)) {
        /* get base address for GDT on memory */
        parse_gdtr(&limit, &base);
        /* set base address of GDT or LDT */
        gdt = (struct gdt_node *)(unsigned long)base;
        /* get a specify segment descriptors on GDT or LDT */
        seg = (unsigned char *)(unsigned long)&gdt[selector >> 0x3];
    } else {
        /* get base address for LDT on memory */
        ;
    }
    /* allocate memory for struct seg_desc */
    desc = (struct seg_desc *)get_free_page();    

    /*** parse segment descriptors ***/
    /*
     * Segment limit field
     *   Specify the size of the segment. The processor puts together
     *   two segment limit fields to from a 20-bit value. The processor
     *   interprets the segment limit in one of two ways, depending on
     *   the setting of the G(granularity) flag:
     *   -> If the granularity flag is clear, the segment size can range
     *      from 1 byte to 1M byte, in byte increments.
     *   -> If the granularity flag is set, the segment size can range 
     *      from 4 KBytes to 4GBytes, in 4-KByte increments.
     *   The processor uses the segment limit in two different ways,
     *   depending on whether the segment is an expand-up or an expand-down
     *   segment. for more information about segment types. For expand-up
     *   segments, the offset in a logical address can range from 0 to the
     *   segment limit. Offsets greater than the segment limit generate 
     *   general-protection exceptions(#GP, for all segment other than SS)
     *   or stack-fault exceptions (#SS for the SS segment). For expand-down
     *   segments, the segment limit has the reverse function. The offset
     *   can range from the segment limit plus 1 to 0xFFFFFFFF or 0xFFFFH,
     *   depending on the setting of the B flag. Offset less then or equal
     *   to the segment limit generate general-protection exception or
     *   stack-fault exceptions. Decreasing the value in the segment limit
     *   field for an expand-down segment allocates new memory at the bottom
     *   of the segment's address space, rather than at the top. IA-32
     *   architecture stacks always grow downwards, making this mechanism
     *   convenient for expandable stacks.
     */    
    desc->limit = seg[0] | (seg[1] << 8) | ((seg[7] & 0xF) << 16);

    /* 
     * Base address fields
     *   Defines the location of byte 0 of the segment within the 4-GByte
     *   linear address space. The processor puts together the three
     *   base address fields to form a single 32-bit value. Segment base
     *   addresses should by aligned to 16-bytes boundaries. Although
     *   16-byte alignment is not required, this alignment allows programs
     *   to maximize performance by aligning code and data on 16-byte
     *   boundaries.
     */
    desc->base = seg[2] | (seg[3] << 8) | (seg[4] << 16) | (seg[7] << 24);

    /*
     * Type field
     *   Indicates the segment or gate type and specifies the kind of 
     *   access that can be made to the segment and the direction of 
     *   growth. The interpretation of this field depends on whether the 
     *   descriptor type flag specifies an application (code or data)
     *   descriptor or a system descriptor. The encoding of the type field
     *   is different for code, data, and system descriptors.
     */
    desc->type = seg[5] & 0xF;
     
    /*
     * S (descriptor) flag
     *   Specify whether the segment descriptor is for a system segment(S
     *   flag is clear) or code or data segment (S flag is set).
     * 
     *   desc->flag:0
     */
    desc->flag |= (seg[5] >> 4) & 0x1;
    
    /*
     * DPL (Descriptor privilege level) field
     *   Specifies the privilege level of the segment. The privilege level
     *   can range from 0 to 3, with 0 being the most privilege level.
     *   The DPL is used to control access to the segment.
     */
    desc->dpl = (seg[5] >> 5) & 0x3;

    /*
     * P (segment-present) flag
     *   Indicates whether the segment is present in memory (set) or not
     *   present (clear). If this flag is clear, the processor generates
     *   a segment-not-present exception(#NP) when a segment selector that
     *   points to the segment descriptor is loaded into a segment register.
     *   Memory management software can use this flag to control which 
     *   segments are actully loaded into physical memory at a given time.
     *   It offers a control in a addition to paging for managing vitual
     *   memory.
     *
     *   When this flag is clear, the operating system or executive is free
     *   to use the locations marked "Available" to store its own data, such
     *   as information regarding the whereabouts of the missing segment.
     *
     *   31--------16-15--14-13-12--11----8-7--------------------------0
     *   | Available | O | DPL | S | Type  | Available                 |
     *   ---------------------------------------------------------------
     *   |                 Available                                   |
     *   ---------------------------------------------------------------
     *
     *   desc->flag:1
     */
    desc->flag |= ((seg[5] >> 7) & 0x01) << 1;

    /*
     * D/B (default operation size/ default stack pointer size/ or 
     *      upper bound) flag
     *   Performs different functions depending on whether the segment 
     *   descriptor is an executable code segment, an expand-down data
     *   segment, or a stack segment. (This flag should always be set to 1
     *   for 32-bit code and data segments and to 0 for 16-bit code and 
     *   data segments).
     *
     *   -> Executable code segment:
     *      The flag is called the D flag and it indicates the default 
     *      length for effective addresses and operands referenced by
     *      instructions in the segment. If the flag is set, 32-bit address
     *      and 32-bit or 8-bit operands are assumed. If it is clear, 16-bit
     *      addresses and 16-bit or 8-bit operands are assumed.
     *      The instruction prefix 66H can be used to select an operand size
     *      other than the default, and the prefix 67H can be used select 
     *      an address size other than the default.
     *
     *   -> Stack segment (data segment pointer to by the SS register)
     *      The flag is called the B(big) flag and it specifies the size of
     *      the stack pointer used for implicit stack operations (such as
     *      pushes, pops, and calls). If the flag is set, a 32-bit stack
     *      pointer is used, which is store in the 32-bit ESP register.
     *      If the flag is clear, a 16-bit stack pointer is used, which is
     *      stored in the 16-bit SP register. If the stack segment is set up
     *      to be an expand-down data segment, the B flag also specifies the
     *      upper bound of the stack segment.
     *
     *   -> Expand-down data segment
     *      The flag is called the B flag and it specifies the upper bound 
     *      of the segment. If the flag is set, the upper bound is 
     *      0xFFFFFFFFH (4 GBytes). If the flag is clear, the upper bound
     *      is 0xFFFFH (64 KBytes).
     *
     *    desc->flag:2
     */
    desc->flag |= ((seg[6] >> 6) & 0x01) << 0x2;

    /*
     * G (granularity) flag
     *   Determines the scaling of the segment limit field. When the 
     *   granularity flag is clear, the segment limit is interpreted in byte
     *   units. When flag is set, the segment limit is interpreted in 4-KByte
     *   units. (This flag does not affect the granularity of the base 
     *   address. it is always byte granular). When the granularity flag is
     *   set, the twelve least significant bits of an offset are not tested
     *   when checking the offset against the segment limit. For example,
     *   when the granularity flag is set, a limit of 0 results in valid
     *   offsets from 0 to 4095.
     *
     *   desc->flag:3
     */
    desc->flag |= ((seg[6] >> 7) & 0x01) << 0x3;

    /*
     * L (64-bit code segment) flag
     *   In IA-32e mode, bit 21 of the second doubleword of the segment
     *   descriptor indicates whether a code segment contains native 64-bit
     *   code. A value of 1 indicates instructions in this code segment
     *   are executed in 64-bit mode. A value of 0 indicates the instructions
     *   in this code segment are executed in compatibility mode. If L-bit
     *   is set, then D-bit must be cleard. When not in IA-32e mode or for
     *   non-code segments, bit 21 is reserved and should always be set to 0.
     *
     *   desc->flag:4
     */
    desc->flag |= ((seg[6] >> 5) & 0x01) << 0x04;

    /*
     * Available and reserved bits
     *   Bit 20 of the second doubleword of the segment descriptor is
     *   available for use by system software.
     *
     *   desc->flag:5
     */
    desc->flag |= ((seg[6] >> 4) & 0x01) << 0x05;
    return desc;
}

/*
 * Segment Descriptor Type
 *   When the S(descriptor type) flag in a segment descriptor is set,
 *   the descriptor is for either a code or data segment. The highest order
 *   bit of the the type field (bit 11 of second double word of the 
 *   segment descriptor) then determines whether the descriptor is for a
 *   data segment (clear) or a code segment (set).
 *
 * @desc: struction of segment descriptor
 *
 * @return: 0 is a code or data segment
 *          1 is a system segment
 *          2 is first segment descriptor on GDT
 */
int segment_descriptor_type(struct seg_desc *desc)
{

    if (!desc) {
        printk("The segment is first member: NULL\n");
        return 2;
    }

    if (!(desc->flag & 0x1)) {
        printk("The segment descriptor point to a system segment.\n");
        return 1;
    } else
        printk("The segment descriptor point to a data/code segment.\n");

    /*
     * For data segments, the three low-order bits of the type field (bits
     * 8, 9 and 10) are interpreted as accessed (A), write-enable (W), and 
     * expansion-direction (E). See Table for a description of the encodeing
     * of the bits in the byte field for code and data segments. Data 
     * segments can be read-only or read/write segments, depending on the
     * setting of the write-enable bit.
     *
     * Table. Code- and Data-Segment Types
     * ----------------------------------------------------------------------
     * |      Type Field       | Descriptor Type | Description              |
     * ----------------------------------------------------------------------
     * | Dec | 11 | 10 | 9 | 8 |                 |                          |
     * |     |    | E  | W | A |                 |                          |
     * ----------------------------------------------------------------------
     * |  0  | 0  | 0  | 0 | 0 |      Data       | Read-Only                |
     * ----------------------------------------------------------------------
     * |  1  | 0  | 0  | 0 | 1 |      Data       | Read-Only, accessed      |
     * ----------------------------------------------------------------------
     * |  2  | 0  | 0  | 1 | 0 |      Data       | Read/Write               |
     * ----------------------------------------------------------------------
     * |  3  | 0  | 0  | 1 | 1 |      Data       | Read/Write, accessed     |
     * ----------------------------------------------------------------------
     * |  4  | 0  | 1  | 0 | 0 |      Data       | Read/Write, expand-down  |
     * ----------------------------------------------------------------------
     * |  5  | 0  | 1  | 0 | 1 |      Data       | Read-Only, expand-down   |
     * |     |    |    |   |   |                 | accessed                 |
     * ----------------------------------------------------------------------
     * |  6  | 0  | 1  | 1 | 0 |      Data       | Read/Write, expand-down  |
     * ----------------------------------------------------------------------
     * |  7  | 0  | 1  | 1 | 1 |      Data       | Read/Write, expand-down  |
     * |     |    |    |   |   |                 | accessed                 |
     * ----------------------------------------------------------------------
     * |     |    | C  | R | A |                 |                          |
     * ----------------------------------------------------------------------
     * |  8  | 1  | 0  | 0 | 0 |      Code       | Execute-Only             |
     * ----------------------------------------------------------------------
     * |  9  | 1  | 0  | 0 | 1 |      Code       | Execute-Only, accessed   |
     * ----------------------------------------------------------------------
     * | 10  | 1  | 0  | 1 | 0 |      Code       | Execute/Read             |
     * ----------------------------------------------------------------------
     * | 11  | 1  | 0  | 1 | 1 |      Code       | Execute/Read, accessed   |
     * ----------------------------------------------------------------------
     * | 12  | 1  | 1  | 0 | 0 |      Code       | Execute-Only, conforming |
     * ----------------------------------------------------------------------
     * | 13  | 1  | 1  | 0 | 1 |      Code       | Execute-Only, conforming |
     * |     |    |    |   |   |                 | accessed                 |
     * ----------------------------------------------------------------------
     * | 14  | 1  | 1  | 1 | 0 |      Code       | Execute/Read, conforming |
     * ----------------------------------------------------------------------
     * | 15  | 1  | 1  | 1 | 1 |      Code       | Execute/Read, conforming |
     * |     |    |    |   |   |                 | accessed                 |
     * ----------------------------------------------------------------------
     */
    if (desc->type & 0x8) {
        /* Code segment */
        printk("Code Segment:");
        /* Accssed?*/
        if (desc->type & 0x1)
            printk(" Accessed");
        /* Execute-only or Execute/Read ? */
        if (desc->type & 0x2)
            printk(" Execute/Read");
        else
            printk(" Execute-only");
        /* conforming ? */
        if (desc->type & 0x4)
            printk(" conforming");
        printk("\n");
    } else {
        /* Data segment */
        printk("Data Segment:");
        /* Accessed ? */
        if (desc->type & 0x1)
            printk(" Accessed");
        /* Read-Only or Read/Write ? */
        if (desc->type & 0x2)
            printk(" Read/Write");
        else
            printk(" Read-Only");
        /* expand-down ? */
        if (desc->type & 0x4)
            printk(" expand-down");
        printk("\n");
    }

    /*
     * The accessed bit indicates whether the segment has been accessed since
     * the last time the operating-system or executive cleared the bit.
     * The processor sets this bit whenever it loads a segment for the 
     * segment into a segment register, assuming that the type of memory
     * that contains the segment descriptor supports processor writes.
     * The bit remains set until explicitly cleared. This bit can be used
     * both for virtual memory management and for debugging.
     */
    /* reload segment selector to segment register */
    if (desc->type & 0x8) {
        /* reload code segment */
        __asm__ ("pushl %%eax\n\t"
                 "mov %%cs, %%ax\n\t"
               //"mov %%ax, %%cs\n\t"  /* cause Invaild operand Error */
                 "popl %%eax"
                 ::);

        if (desc->type & 0x1)
            printk("loading new code segment selector.\n");
        else
            printk("new code segment doesn't accessed.\n");
    } else {
        /* reload data segment */
        __asm__ ("pushl %%eax\n\t"
                 "movl %%ds, %%eax\n\t"
                 "movl %%eax, %%ds\n\t"
                 "popl %%eax"
                 ::);

        if (desc->type & 0x1)
            printk("loading new data segment selector.\n");
        else
            printk("New data segment doesn't accessed.\n");
    }

    /*
     * For code segments, the three low-order bits of the type field are
     * interpreted as accessed (A), read enable (R), and conforming (C).
     * Code segments can be execute-only or execute/read, depending on the
     * setting of the read-enable bit. An execute/read segment might be
     * used when constants or other static data have been placed with 
     * instruction code in a ROM. Here, data can be read from the code
     * segment either by using an instruction with a CS override prefix or
     * by loading a segment selector for the code segment in a data-segment
     * register (the DS, ES, FS, or GS register). In protected mode, code
     * segments are not writeable.
     */
    if (desc->type & 0x8) {
        __asm__ ("pushl %%eax\n\t"
                 "movl %%cs, %%eax\n\t"
                 "movl %%eax, %%gs\n\t"
                 "popl %%eax"
                 ::);
    }

    /*
     * Code segment can be either conforming or nonconforming. A transfer
     * of execution into a more-privileged conforming segment allows
     * execution to continue at the current privilege level. A transfer
     * into a nonconforming segment at a different privilege level results
     * in a general-protection exception (#GP), unless a call gate or task
     * gate is used. System utilies that do not access protected facilities
     * and handlers for some types of exception (such as, divide error or 
     * overflow) may by loaded in conforming code segments. Utilies that
     * need to be protected from less privileged programs and programs and
     * procedures should be placed in nonconforming code segments.
     */
    /* no actual routine to proceure */

    /*
     * Note
     *   Exception cannot be transferred by a call or a jump to a less-
     *   privileged (numerically higher privilege level) code segment,
     *   regardless of whether the target segment is a conforming or 
     *   nonconforming code segment. Attempting such as execution transfer
     *   will result in a general-protection exception.
     */

    /*
     * All data segments are nonconforming, meaning that they cannot be
     * accessed by less privileged programs or procedures (code executing
     * at numerically higher privilege levels). Unless code segments,
     * however, data segments can be accessed by more privilege programs
     * or procedures (code executing at numerically lower privilege levels)
     * without using a specical access gate.
     */
    /* now, no routine to procedure */

    /*
     * If the segment descriptors in the GDT or an LDT are placed in ROM,
     * the processor can enter an indefinite loop if software or the 
     * processor attempts to update (write to) the ROM-based segment
     * descriptors. To prevent this problem, set the accessed bits for all
     * segment descriptors placed in a ROM. Also, remove operating-system
     * or executive code that attempts to modify segment descriptors 
     * located in ROM.
     */
    return 0;
}

/*
 * DPL (Descriptor privilege level)
 */
int segment_descriptor_dpl(unsigned long selector)
{
    struct seg_desc *desc;
    unsigned long dpl;

    desc = segment_descriptors(selector);
    if (!desc)
        return -1;

    dpl = desc->dpl;
   
    free_page((unsigned long)desc);
    return dpl;
}

/*
 * CPL
 */
int segment_descriptor_cpl(void)
{
    unsigned long cs;

    __asm__ ("movl %%cs, %0"
             : "=r" (cs));

    return segment_descriptor_dpl(cs);
}

/*
 * Stack segment
 */
int parse_stack_segment_descriptor(void)
{
    unsigned long ss;
    struct seg_desc *desc;

    printk("Parse stack segment.\n");
    /* get stack segment selector */
    __asm__ ("movl %%ss, %0"
             : "=r" (ss));

    /* get stack segment descritpor from GDT or LDT */
    desc = segment_descriptors(ss);
    /*   
     * Stack type
     *   Stack segments are data segment which must be read/write segments.
     *   Loading the SS register with a segment selector for a nonwritable
     *   data segment generates a general-protection exception (#GP). If
     *   the size of a stack segment needs to be changed dynamically, the 
     *   stack segment can be an expand-down data segment (expansion-direction
     *   flag set). Here, dynamically changing the segment limit causes stack
     *   space to be added to the bottom of the stack. If the size of a stack
     *   segment is intended to remain static, the stack segment may be 
     *   either an expand-up or expand-down type.
     */
    segment_descriptor_type(desc);

    /* release resource */
    if (desc)
        free_page((unsigned long)desc);
    return 0;
}

/*
 * Code segment descriptor
 */
int parse_code_segment_descriptor(void)
{
    unsigned long cs;
    struct seg_desc *desc;

    printk("Parse Code segment\n");
    /* get code segment selector */
    __asm__ ("movl %%cs, %0"
             : "=r" (cs));

    /* get code segment descriptor from GDT */
    desc = segment_descriptors(cs);

    /* parse code segment type */
    segment_descriptor_type(desc);

    /* release resource */
    if (desc)
        free_page((unsigned long)desc);
    return 0;
}

/* debug gdt common enter */
void debug_gdt_common(void)
{
    /* add test item here */
}
