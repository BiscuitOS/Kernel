/*
 * Stack switch on interrupt
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>

#include <test/debug.h>

/*
 * Stack
 *   The stack is a contiguous array of memory locations. It is contained
 *   in a segment and identified by the segment selector in the SS register.
 *   When using the flat memory model, the stack can be located anywhere
 *   in the linear address space for the program. A stack can be up to 4GB
 *   long, the maximum size of a segment.
 *
 *   Items are placed on the stack using the PUSH instruction and removed
 *   from the stack using the POP instruction. When an item is pushed onto
 *   the stack, the processor decrements the ESP register, then write the
 *   item at the new top of stack. When an item is popped off the stack,
 *   the processor read the item from the top of stack, then increments 
 *   the ESP register. In this manner, the stack grows down in memory (
 *   towards lesser address) when items are pushed on the stack and shrinks
 *   up (towards greater addresses) when the items are popped from the stack.
 *
 *   A program or operating system/executive can set up many stacks. For
 *   example, in multitasking system, each task can be given its own stack.
 *   The number of stacks in a system is limited by the maximum of segments
 *   and the available physical memory.
 *
 *   When a system sets up many stacks, only one stack -- the current stack
 *   is available at time. The current stack is the one contained in the 
 *   segment referenced by the SS register.
 *
 *   The processor references the SS register automatically for all stack
 *   operations. For example, when the ESP register is used as a memory
 *   address, it automatically points to an address in the current stack.
 *   Also, the CALL, RET, PUSH, POP, ENTER, and LEAVE instruction all
 *   perform operations on the curremt stack.
 */

/*
 * Setting up a Stack
 *   To set a stack and establish it as the current stack, the program or
 *   operating system/executive must do following:
 *   
 *   1. Establish a stack segment.
 *
 *   2. Load the segment selector for the stack segment into the SS register
 *      using a MOV, POP, or LSS instruction.
 *
 *   3. Load the stack pointer for the stack into the ESP register a MOV, POP,
 *      or LSS instruction. The LSS instruction can be used to load the SS
 *      and ESP register in one operation.
 */
static void establish_kernel_stack(void)
{
    /* establish a new segment for stack */
    unsigned short selector;
    unsigned short index;
    unsigned long  seg_desc[2] = { 0, 0 };
    unsigned long  *desc;
    unsigned long  *base_address;
    unsigned long  limit;
    unsigned char  __base[6];
    unsigned long  *gdt_base;
    unsigned short gdt_limit;
    unsigned long  *stack_esp;
    unsigned long  *stack_ebp;

    struct stack_p {
        long *a;
        short *b;
    } *new_stack; 

    /*
     * Initialize Segment descriptor
     *   Establish a new segment on GDT, the offset of segment is "0x50".
     *   This segment only kernel can access.
     *
     *   15-----------------------------3----2-1----0
     *   |  Index                        | TI | RPL |
     *   -------------------------------------------- 
     */
    index = 0x12;
    selector = (index << 0x3) | (0x0 << 0x2) | 0x00;
 
    /*
     * Allocate a page for new stack, and base address of stack segment is
     * base address of new page. The limit of stack segment is length of 
     * new page.
     */
    base_address = (unsigned long *)get_free_page();
    limit = PAGE_SIZE - 1;

    /*
     * Stack Segment Descriptor
     *
     * 31--------24---------20------------15------------------7-----------0
     * | Base 31:24|G|D/B|AVL|segLimt 19:16|P| DPL | S | Type |Base 23:16 |
     * --------------------------------------------------------------------
     * | Base Address 15:0                 | Segment Limit 15:0           |
     * --------------------------------------------------------------------
     */
    /*
     * Segment limit field for Stack Segment
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
    seg_desc[0] |= limit & 0xFFFF;
    seg_desc[1] |= ((limit >> 16) & 0xF) << 15;

    /* 
     * Base address fields for Stack Segment
     *   Defines the location of byte 0 of the segment within the 4-GByte
     *   linear address space. The processor puts together the three
     *   base address fields to form a single 32-bit value. Segment base
     *   addresses should by aligned to 16-bytes boundaries. Although
     *   16-byte alignment is not required, this alignment allows programs
     *   to maximize performance by aligning code and data on 16-byte
     *   boundaries.
     */
    seg_desc[0] |= ((unsigned long)base_address & 0xFFFF) << 16;
    seg_desc[1] |= ((unsigned long)base_address >> 16) & 0xFF;
    seg_desc[1] |= (((unsigned long)base_address >> 24) & 0xFF) << 24;

    /*
     * Type field for Stack Segment
     *   Indicates the segment or gate type and specifies the kind of 
     *   access that can be made to the segment and the direction of 
     *   growth. The interpretation of this field depends on whether the 
     *   descriptor type flag specifies an application (code or data)
     *   descriptor or a system descriptor. The encoding of the type field
     *   is different for code, data, and system descriptors.
     *
     *   For stack segment, it belong to code segment and it must be 
     *   Read/Write and accessed.
     */
    seg_desc[1] |= 0x3 << 8;     
     
    /*
     * S (descriptor) flag for Stack Segment
     *   Specify whether the segment descriptor is for a system segment(S
     *   flag is clear) or code or data segment (S flag is set).
     * 
     *   For stack segment, it must be set.
     */
    seg_desc[1] |= 0x1 << 12;

    /*
     * DPL (Descriptor privilege level) field for Stack Segment
     *   Specifies the privilege level of the segment. The privilege level
     *   can range from 0 to 3, with 0 being the most privilege level.
     *   The DPL is used to control access to the segment.
     *
     *   For kernel stack, DPL must be 0x00.
     *   For userland stack, DPL must be 0x03.
     */
    seg_desc[1] |= 0x0 << 13;

    /*
     * P (segment-present) flag for Stack Segment
     *   Indicates whether the segment is present in memory (set) or not
     *   present (clear). If this flag is clear, the processor generates
     *   a segment-not-present exception(#NP) when a segment selector that
     *   points to the segment descriptor is loaded into a segment register.
     *   Memory management software can use this flag to control which 
     *   segments are actully loaded into physical memory at a given time.
     *   It offers a control in a addition to paging for managing vitual
     *   memory.
     *
     *   For Stack, this bit must be set.
     */
    seg_desc[1] |= 0x1 << 15;

    /*
     * D/B (default operation size/ default stack pointer size/ or 
     *      upper bound) flag for Stack Segment
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
     *    For Stack Segment on 32-bit protect mode, this bit must be set.
     */
    seg_desc[1] |= 0x1 << 22;

    /*
     * G (granularity) flag for Stack Segment
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
     *   For stack segment, this bit is cleared.
     */
    seg_desc[1] |= 0x1 << 0x17;

    /* load segment descriptor of stack to GDT */
    /*
     * GDTR: Global Descriptor Table Register
     * ---------------------------------------------------
     * | 32-bit Linar Base address  | 16-bit Table Limit |
     * ---------------------------------------------------
     */
    __asm__ ("sgdt %0"
             : "=m" (__base));
    gdt_limit = *(unsigned short *)(unsigned long)__base;
    gdt_base  = (unsigned long *)(unsigned long)__base[2];

    /* Diagnose whether stack selector overflower limit of gdt */
    if ((index * 8) > gdt_limit || index == 0) {
        printk("The selecot of stack overflower GDT limit\n");
        free_page((unsigned long)base_address);
        return;
    }

    /* Load LSB of new segment descriptor */
    desc  = (unsigned long *)((selector >> 3) * 8 + (unsigned char *)gdt_base);
    *desc = seg_desc[0];
    /* Load MSB of new segment descriptor */
    desc  = (unsigned long *)(4 + (unsigned char *)desc);
    *desc = seg_desc[1]; 

    /*
     * LSS
     *   Loads a far pointer (segment selector and offset) from the second
     *   operand (source operand) into a segment register and the frist 
     *   operand (destination operand). The source operand specifies a 
     *   48-bit or a 32-bit pointer in memory depending on the current 
     *   setting of the operand-size attribute. The instruction opcode
     *   and the destination operand specify a segment register/
     *   general-purpose register pair. The 16-bit segment selector from the 
     *   source operand is loaded into the segment register specified with
     *   the opcode (DS, SS, ES, FS or GS). The 32-bit or 16-bit offset is
     *   loaded into the register specified with the destination operand.
     *
     *   If one of these instructions is executed in protected mode,
     *   additional information from the segment descriptor pointed to by
     *   the segment selector in the source operand is loaded in the hidden
     *   part of the selected segment register.
     */
    /* specify ESP register point to end of Stack segment */
    new_stack = (struct stack_p *)base_address;
    stack_esp = (unsigned long *)(0xFF6 + (unsigned char *)base_address);
    new_stack->a = (long *)stack_esp;
    /* specify SS point new stack segment */
    new_stack->b = (short *)(4 + (unsigned char *)base_address);
    *(new_stack->b) = selector;

    /* Specify EBP on stack */
    stack_ebp = (unsigned long *)(limit + (unsigned char *)base_address);
    __asm__ ("movl %0, %%ebp"
             :: "m" (stack_ebp));

    __asm__ ("movl %0, %%eax\n\r"
             "lss (%%eax), %%ebx"
             :: "m" (new_stack));
    /*
     * Also in protect mode, a NULL selector (values 0000 through 0003) can
     * be loaded into DS, ES, FS, or GS registers without causing a 
     * protection exception. (Any subsequent reference to a segment whose
     * corresponding segment register is loaded with a NULL selector, causes
     * a general-protection exception (#GP) and no memory reference to the
     * segment occurs);
     *
     * #GP(0)
     *   If a NULL selector is loaded into the SS register.
     * #GP(selector)
     *   If the SS register is being loaded and sny of the following is true:
     *   the segment selector index is not within the descriptor table limits,
     *   the segment selector RPL is not equal to CPL, the segment is a 
     *   non-writable data segment, or DPL is not equal to CPL.
     */
}

void common_stack_switch(void)
{

    if (1) {
        /* Establish a new stack */
        establish_kernel_stack();
    }
}
