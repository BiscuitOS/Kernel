/*
 * Stack
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/head.h>
#include <asm/system.h>

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

/*
 * Stack Alignment
 *   The stack pointer for a stack segment should be aligned on 16-bit (word)
 *   or 32-bit (double-word) boundaries, depending on the width of the stack
 *   segment. The D flag in the segment descriptor for the current code
 *   segment sets the stack-segment width. The PUSH and POP instructions
 *   use the D flag to decrement or increment the stack pointer on a push or
 *   pop operation, respectively. When the stack width is 32 bits, the stack
 *   pointer is increamented or decremented in 32-bit increments. Pushing
 *   16-bit value onto a 32-bit wide stack can result in stack misaligned (
 *   that is , the stack pointer is not aligned on a double-word boundary).
 *   One exception to this rule is when the contents of a semgnet register (
 *   16-bit segment selector) are pushed onto a 32-bit wide stack. Here, 
 *   the processor automatically aligns the stack pointer to the next 32-bit
 *   boundary.
 *
 *   The processor does not check stack pointer alignment. It is the 
 *   reponsibility of the program, tasks, and system procedures running on
 *   the processor to maintain proper alignment of stack pointer. Misaligning
 *   pointer can cause serious performance degradation and in some 
 *   instances program failures.
 */
static void diagnose_stack_pointer(void)
{
    unsigned short cs;
    unsigned long *cs_seg;
    unsigned char __base[6];
    unsigned long *base_gdt;
    unsigned long esp0, esp1;

    /* 
     * Obtain D flag from Code Segment Descriptor. 
     *   The D flag in the segment descriptor for the current code
     *   segment sets the stack-segment width. The PUSH and POP instructions
     *   use the D flag to decrement or increment the stack pointer on a push or
     *   pop operation, respectively.
     */
    /* Obtain CS selector */
    __asm__ ("mov %%cs, %0"
            : "=m" (cs));
    /* Obtain the base address for GDT (assume the CS location on GDT)*/
    __asm__ ("sgdt %0"
            : "=m" (__base));
    base_gdt = (unsigned long *)(unsigned long)&__base[2];
    cs_seg = (unsigned long *)(((cs >> 3) * 8) + (unsigned char *)base_gdt);

    /*
     * Address-Size for Stack Accesses
     *   Instruction that use the stack implicitly (such as the PUSH and POP
     *   instructions) have two address-size attributes each of either 16 or 
     *   32 bits. This is because they always have the implicit address of 
     *   the top of the stack, and they may also have an explicit memory 
     *   address (for example, PUSH Array1[EBX]). The attribute of the 
     *   explicit address is determined by the D flag of the current code 
     *   segment adn the presence or absence of the 67H address-size prefix.
     *
     *   The address-size attribute of the top of the stack determines 
     *   whether SP or ESP is used for the stack access. Stack operations 
     *   with an address-size attribute of 16 use the 16-bit SP stack pointer
     *   register and can use maximum stack address of 0xFFFFH. Stack 
     *   operation with an address-size attribute of 32 bits use the 32-bit
     *   ESP register and can use a maximum address of 0xFFFFFFFFH. The 
     *   default address-size attribute for data segment used as stacks is
     *   controlled by the B flag of segment's descriptor. When this flag 
     *   is clear, the default address-size attribute is 16. When the flag
     *   is set, the address-size attribute is 32.
     */
    if (cs_seg[1] >> 22)
        printk("Stack Pointer Width (POP, PUSH): 32-bit [0 - FFFFFFFF]\n");
    else
        printk("Stack Pointer Width (POP, PUSH): 16-bit [0 - FFFF]\n");

    /* Automatical Align
     *   Pushing 16-bit value onto a 32-bit wide stack can result in stack 
     *   misaligned (that is , the stack pointer is not aligned on a 
     *   double-word boundary). One exception to this rule is when the 
     *   contents of a semgnet register (16-bit segment selector) are pushed 
     *   onto a 32-bit wide stack. Here, the processor automatically aligns 
     *   the stack pointer to the next 32-bit boundary.
     */
    __asm__ ("movl %%esp, %%eax\n\r"
             "push %%ds\n\r"
             "movl %%esp, %%ebx\n\r"
             "pop %%ds"
             : "=a" (esp0) ,"=b" (esp1));
    printk("Base ESP address for current Stack: %#x\n", esp0);
    printk("Misaligned and Automatical Align, ESP: %#x\n", esp1);
}

/* Test for CALL */
static void diagnose_call(void)
{
    printk("Inovke function in 'CALL'\n");
}

/*
 * Procedure Linking Information
 *   The processor provides two pointers for linking of procedures. the 
 *   Stack-frame base pointer and the return instruction pointer. When
 *   used in conjunction with a standard software procedure-call technique,
 *   these pointers permit reliable and coherent linking of procedures.
 */
static void stack_link_pointer(void)
{
    /*
     * Stack-Frame Base Pointer: EBP
     *   The stack is typically divided into frames. Each stack frame can
     *   then contain local variables, parameters to be passed to another
     *   procedure, and procedure linking information. The stack-frame base
     *   pointer (contained in the EBP register) identifies a fixed reference
     *   point within the stack frame for the called procedure. To use the
     *   stack-frame base pointer, the called procedure typically copies
     *   the contents of the ESP register into the EBP register prior to 
     *   pushing any local variables on the stack. The stack-frame base 
     *   pointer then permits easy access to data structures passed on the 
     *   stack, to the return instruction pointer, and to local variable added
     *   to the stack by the called procedure.
     *
     *   Like the ESP register, the EBP register automatically points to an 
     *   address in the current stack segment (that is, the segment sepecified
     *   by the current contents of the SS register).
     */
    __asm__ ("pushl %%ebp\n\r"
             "movl %%esp, %%ebp\n\r"
             "subl $16, %%esp\n\r"
             "pushl %%eax\n\r"
             "pushl %%ebx\n\r"
             "movl %%ebp, %%esp\n\r"
             "popl %%ebp"
             ::);

    /*
     * Return Instruction Pointer
     *   Prior to branching to the first instruction of the called procedure,
     *   the CALL instruction pushes the address in the EIP register onto
     *   the current stack. This address is then called the return-instruction
     *   pointer and it points to the instruction where execution of the 
     *   calling procedure should resume following a return from the called
     *   procedure. Upon returning from a called procedure, the RET instruction
     *   pops the return-instruction pointer from the stack back into the EIP
     *   register. Execution of the calling procedure then resumes.
     *
     *   The processor does not keep track of the location of the return-
     *   instruction pointer. It is thus up to the programmer to insure that
     *   stack pointer is pointing to the return-instruction pointer on the 
     *   stack, prior to issuing a RET instruction. A common way to reset
     *   the stack pointer to the point to the return-instruction pointer is
     *   to move the contents of the EBP register into the ESP register. If
     *   the EBP register is loaded with the stack pointer immediately 
     *   following a procedure call, it should point to the return instruction
     *   pointer on the stack.
     *
     *   The processor does not require that the return instruction pointer
     *   point back to the calling procedure. Prior to executing the RET
     *   instruction, the return instruction pointer can be manipulated in
     *   software to point to any address in the current code segment (near
     *   return) or another code segment (far return). Performing such an
     *   operation, however, should be undertaken very canutiously, using
     *   only well defined code entry points.
     */
    __asm__ ("pushl %%ebp\n\r"
             "movl %%esp, %%ebp\n\r"
             "call diagnose_call\n\r"
             "movl %%ebp, %%esp\n\r"
             "popl %%ebp\n\r"
             ::);
}

/*
 * Calling Procedure Using CALL and RET
 *   The CALL instruction allows control transfers to procedures within the
 *   current code segment (near call) and in a different code segment (far
 *   call). Near calls usually provide access to local procedures within
 *   the currently running program or task. Far calls are usually used to 
 *   access operating system procedures or procedures in a different task.
 *
 *   The RET instruction also allows near and far returns to match the near
 *   and far versions of the CALL instruction. In addition, the RET 
 *   instrcution allows a program to increment the stack pointer on a return
 *   to release parameters from the stack. The number of bytes release from
 *   from the stack is determined by an optional argument (n) to the RET
 *   instruction.
 */

/* Near RET Operation
 *   When executing a near return, the processor performs these actions:
 *   1) Pops the top-of-stack value (the return instruction pointer) into
 *      the EIP register.
 *   2) If the RET instruction has an optional n argument, increments the
 *      stack pointer by the number of bytes specified with the n operand
 *      to release parameters from the stack.
 *   3) Resumes execution of the calling procedure.
 */
static void stack_near_return(void)
{
    printk("Near Return from stack\n");

    __asm__ ("pushl %%ebp\n\r"
             "movl %%esp, %%ebp\n\r"
             "addl $24, %%esp\n\r"
             "ret"
             ::);
}

/*
 * Near CALL Operation
 *   When executing a near call, the processor does the following:
 *   1) Pushes the current value of the EIP register on the stack.
 *   2) Loads the offset of the called procedure in the EIP register.
 *   3) Begins execution of the called procedure
 */
static void stack_near_call(void)
{
    __asm__ ("pushl %%ebp\n\r"
             "movl %%esp, %%ebp\n\r"
             "call stack_near_return\n\r"
             "movl %%ebp, %%esp\n\r"
             "popl %%ebp"
             ::);
}

/*
 * Far RET Operation
 *   When executing a far return, the processor does the follow:
 *   1) Pops the top-of-stack value (the return instruction pointer) into
 *      the EIP register.
 *   2) Pops the top-of-stack value (the segment selector for the code
 *      segment being returned to) into the CS register.
 *   3) If the RET instruction has an optinal n argument, increments the
 *      stack pointer by the number of bytes specified with the n operand
 *      to release parameters from the stack.
 *   4) Resumes execution of the calling procedure.
 */
static void stack_far_return(void)
{
}

/*
 * Far CALL Operation
 *   When executing a far call, the processor performs these actions:
 *   1) Pushes the current value of the CS register on the stack.
 *   2) Pushes the current value of the EIP register on the stack.
 *   3) Loads the segment selector of the segment that contains the called
 *      procedure in the CS register.
 *   4) Loads the offset of the called procedure in the EIP register
 *   5) Begins execution of the called procedure.
 */
static void stack_far_call(void)
{
}

/*
 * Parameter Passing
 *   Parameters can be passed between procedures in any of three ways:
 *   1) Through general-purpose registers
 *   2) Through in an argument list
 *   3) Through on the stack
 */
/* Obtain paramter from general-purpose register */
static void parse_paramter_from_register(void)
{
    unsigned long eax, ebx, ecx, edx, esi, edi;

    __asm__ ("movl %%eax, %0\n\r"
             "movl %%ebx, %1\n\r"
             "movl %%ecx, %2\n\r"
             "movl %%edx, %3\n\r"
             "movl %%esi, %4\n\r"
             "movl %%edi, %5"
             : "=m" (eax), "=m" (ebx), "=m" (ecx),
               "=m" (edx), "=m" (esi), "=m" (edi)
             :);
    printk("EAX [%#x] EBX [%#x] ECX [%#x] EDX [%#x] ESI [%#x] EDI [%#x]\n",
           eax, ebx, ecx, edx, esi, edi);
}

/* Obtain paramter from Stack */
static void parse_paramter_from_stack(unsigned long p0, unsigned long p1,
             unsigned long p2, unsigned long p3)
{
    printk("Obtain paramter from Stack: [%#x] [%#x] [%#x] [%#x]\n",
              p0, p1, p2, p3);
}

/* Obtain paramter from argument list */
static void parse_argument_list(char *list)
{
    printk("Obtain paramter from argument list: [%c] [%c] [%c] [%c]\n",
               list[0], list[1], list[2], list[3]);
}

char argument[] = { 'a', 'b', 'c', 'd'};
/* Passing Paramter */
static void stack_paramenter_pass(void)
{
    /*
     * Passing Parameters through the General-Purpose Register
     *   The processor does not save the state of the general-purpose registers
     *   on procedure calls. A calling procedure can thus pass up to six
     *   parameters to the called procedure by copying the parameters into 
     *   any of these registers (except the ESP and EBP registers) prior
     *   to executing the CALL instruction. The called procedure can likewise
     *   pass parameters back to the calling procedure through general-
     *   purpose register.
     */
    __asm__ ("pushl %%eax\n\r"
             "pushl %%ebx\n\r"
             "pushl %%ecx\n\r"
             "pushl %%edx\n\r"
             "pushl %%esi\n\r"
             "pushl %%edi\n\r"
             "movl $1, %%eax\n\r"
             "movl $2, %%ebx\n\r"
             "movl $3, %%ecx\n\r"
             "movl $4, %%edx\n\r"
             "movl $5, %%esi\n\r"
             "movl $6, %%edi\n\r"
             "call parse_paramter_from_register\n\r"
             "popl %%edi\n\r"
             "popl %%esi\n\r"
             "popl %%edx\n\r"
             "popl %%ecx\n\r"
             "popl %%ebx\n\r"
             "popl %%eax"
             ::);

    /*
     * Passing Parameters on the Stack
     *   To pass a large number of parameters to the called procedure, the
     *   parameters can be placed on the stack, in the stack frame for the
     *   calling procedure. Here, it is useful to use the stack-frame base
     *   pointer (in the EBP register) to make a frame boundary for easy 
     *   access to the parameters.
     *
     *   The stack can also be used to pass parameters back from the called
     *   procedure to the calling procedure.
     */
    __asm__ ("pushl %%eax\n\r"
             "pushl %%ebp\n\r"
             "movl %%esp, %%ebp\n\r"
             "subl $16, %%esp\n\r"
             "movl $1, %%eax\n\r"
             "pushl %%eax\n\r"
             "movl $2, %%eax\n\r"
             "pushl %%eax\n\r"
             "movl $3, %%eax\n\r"
             "pushl %%eax\n\r"
             "movl $4, %%eax\n\r"
             "pushl %%eax\n\r"
             "call parse_paramter_from_stack\n\r"
             "movl %%ebp, %%esp\n\r"
             "popl %%ebp\n\r"
             "popl %%eax"
             ::);

    /*
     * Passing Parameter in an Argument List
     *   An alternate method of passing a larger number of paramters (or a
     *   data structure) to the called procedure is to place the parameters
     *   in an argument list in one of the data segments in memory. A pointer
     *   to the argument list can then be passed to the called procedure
     *   through a general-purpose register or the stack. Parameters can also
     *   be passed back to the calling procedure in this same manner.
     */
     __asm__ ("pushl %%eax\n\r"
              "pushl %%ebp\n\r"
              "movl %%esp, %%ebp\n\r"
              "subl $4, %%esp\n\r"
              "movl $argument, %%eax\n\r"
              "pushl %%eax\n\r"
              "call parse_argument_list\n\r"
              "movl %%ebp, %%esp\n\r"
              "popl %%ebp\n\r"
              "popl %%eax"
              ::);
}

/*
 * Saving Procedure State Information
 *   The processor does not save the contents of the general-purpose 
 *   registers, segment register, or the EFLAGS register on a procedure
 *   call. A calling procedure should explicitly save the values in any
 *   of the general-prupose registters that it will need when it resumes
 *   execution after a return. These values can be saved on the stack
 *   or in memory in one of the data segment.
 */
/* parse pusha */
static void parse_popa(unsigned long edi, unsigned long esi, 
            unsigned long ebp, unsigned long esp, unsigned long ebx,
            unsigned long edx, unsigned long ecx, unsigned long eax)
{
    /*
     * PUSHA pushes the values
     *   in all the general-purpose registers on the stack in the following
     *   order: EAX, ECX, EDX, EBX, ESP (the value prior to executing the PUSHA
     *   instruction), EBP, ESI, and EDI.
     */
    printk("PUSHA: EAX [%#x] EBX [%#X] ECX [%#x] EDX [%#x]\n"
                  "ESI [%#x] EDI [%#x]\n", eax, ebx, ecx, edx, esi, edi);
}

/* Saving procedure state information */
static void save_procedure_state(void)
{
    unsigned long eflags;

    /*
     * PUSHA and POPA
     *   The PUSHA and POPA instructions facilitate saving and restoring the 
     *   contents of the general-purpose registers. PUSHA pushes the values
     *   in all the general-purpose registers on the stack in the following
     *   order: EAX, ECX, EDX, EBX, ESP (the value prior to executing the PUSHA
     *   instruction), EBP, ESI, and EDI. The POPA instruction pops all the 
     *   register values saved with a PUSHA instruction (expect the ESP value)
     *   from the stack to their respective registers.
     */
    __asm__ ("pusha\n\r"
             "pushl %%ebp\n\r"
             "movl %%esp, %%ebp\n\r"
             "movl $1, %%eax\n\r"
             "movl $2, %%ebx\n\r"
             "movl $3, %%ecx\n\r"
             "movl $4, %%edx\n\r"
             "movl $5, %%esi\n\r"
             "movl $6, %%edi\n\r"
             "pusha\n\r"
             "call parse_popa\n\r"
             "movl %%ebp, %%esp\n\r"
             "popl %%ebp\n\r"
             "popa"
             ::);

    /*
     * If a called procedure changes the state of any of the segment 
     * registers explicitly, it should restore them to their former values
     * before executing a return to the calling procedure.
     */

    /*
     * If a calling procedure needs to maintain the state of the EFLAGS 
     * register, it can save and restore all or part of the register
     * using the PUSHF/PUSHFD and POPF/POPFD instructions. The PUSHF 
     * instruction pushes the lower word of the EFLAGS register on the 
     * stack, while the PUSHFD instruction pushes the entire register. 
     * The POPF instruction pops a word from the stack into the lower
     * word of the EFLAGS register, while the POPFD instruction pops a
     * double word from the stack into the register.
     */
    __asm__ ("pushl %%ebp\n\r"
             "movl %%esp, %%ebp\n\r"
             "pushfl\n\r"
             "movl (%%esp), %%eax\n\r"
             "popfl\n\r"
             "movl %%ebp, %%esp\n\r"
             "popl %%ebp"
             : "=a" (eflags));
    printk("Obtain Current Task EFLAGS: %#x\n", eflags);
}

/*
 * Calls to Other Privilege Levels
 *   The IA-32 architecture's protection machanism recognizes four 
 *   privilege levels, numbered from 0 to 3, where a greater number mean
 *   less privilege. The reason to use privilege levels is to improve the
 *   reliablity of operating systems.
 * 
 *   The highest privilege level 0 is used for segments that contain the 
 *   most critical code modules in the system, usually the kernel of an
 *   operating system. The outer rings are used for segments that contain
 *   code modules for less critical software.
 *
 *   Code modules in lower privilege segments can only access modules
 *   operating at higher privilege segments by means of a tightly controlled
 *   and protected interface called a gate. Attempts to access higher 
 *   privilege segments without going through a protection gate and without
 *   having sufficient access right causes a general-protection exception
 *   (#GP) to be generated.
 *
 *   If an operating system or executive uses this multilevel protection 
 *   mechanism, a call to a procedure that is in a more privileged protection
 *   level than the calling procedure is handled in a similar manner as a
 *   far call. The differences are as follow:
 *
 *   1) The segment selector provided in the CALL instruction references a
 *      special data structure called a call gate descriptor. Among other
 *      things, the call gate descriptor provides the following:
 *      
 *      -- Access right information
 *
 *      -- The segment selector for the code segment of the called procedure
 *
 *      -- An offset into the code segment (that is, the instruction pointer
 *         for the called procedure)
 *
 *   2) The processor switches to a new stack to execute the called procedure
 *      Each privilege level has its own stack. The segment selector and
 *      stack pointer for the privilege level 3 stack are stored in the SS
 *      and ESP registers, respectively, and are automatically saved when
 *      a call to a more privileged level occurs. The segment selectors and
 *      stack pointers for the privilege level 2, 1, and 0 stacks are stored
 *      in a system segment called the task state segment (TSS)
 *
 *   The use of a call gate and the TSS during a stack switch are transparent
 *   to the calling procedure, except when a general-protection exception is 
 *   raised.    
 */

/*
 * CALL Operation Between Privilege Levels
 *   When making a call to more privileged protection level, the processor
 *   does the following:
 *
 *   1) Performs an access right check (privilege check)
 *
 *   2) Temporarily saves (internally) the current contents of the SS,ESP
 *      CS, and EIP register.
 *
 *   3) Loads the segment selector and stack pointer for the new stack (that
 *      is, the stack for the privilege level being called) from the TSS
 *      into the SS and ESP register and switches to the new stack.
 *
 *   4) Pushes the temporarily saved SS and ESP values for the calling 
 *      procedure's stack onto the new stack.
 *
 *   5) Copies the parameters from the calling procedure's stack to the new
 *      stack. A value in the call gate descriptor determines how many
 *      parameters to copy to the new stack.
 *
 *   6) Pushes the temporarily saved CS and EIP values for the calling
 *      procedure to the new stack.
 *
 *   7) Loads the segment selector for the new code segment and the new
 *      instruction pointer from the call gate into the CS and EIP registers,
 *      resoectively.
 *
 *   8) Begins execution of the called procedure at the new privilege level.
 */
static void emulate_user_call_kernel(void)
{
}

/*
 * RET Operation Between Privilege Levels
 *   When executing a return from the privileged procedure, the processor
 *   performs these actions:
 *
 *   1) Performs a privilege check.
 *
 *   2) Restores the CS and EIP registers to their values prior to the call.
 *
 *   3) If the RET instrcution has an optional n argument, increments the 
 *      stack pointer by the number of the bytes specified with the n operand
 *      to release parameters from the stack. If the call gate descriptor
 *      specifies that one or more parameters be copied from one stack to the
 *      other, a RET n instruction must be used to release the parameters
 *      from both stacks. Here, the n operand specifies the number of bytes
 *      occupied on each stack by the parameters. On a return, the processor
 *      increments ESP by n for each stack to step over (effectively remove)
 *      these parameters from the stacks.
 *
 *   4) Restores the SS and ESP register to their values prior to the call,
 *      which causes a switch back to the stack of the calling procedure.
 *
 *   5) If the RET instrcution has an optional n argument, increaments the 
 *      stack pointer by the number of bytes specified with the n operand to
 *      release parameters from the stack.
 *
 *   6) Resumes execution of the calling procedure.
 */
static void emulate_kernel_return_userland(void)
{
}

/*
 * Call Operation for Interrupt or Exception Handling Procedure
 *   A call to an interrupt or exception handler procedure is similar to a
 *   procedure call to another protection. Here, the vector reference one
 *   of two kinds of gates in the IDT: an interrupt gate or a trap gate.
 *   Interrupt and trap gates are similar to call gates in that they provide
 *   the following information:
 * 
 *   1) Access rights information
 *
 *   2) The segment selector for the code segment that contains the handler
 *      procedure.
 *
 *   3) An offset into the code segment to the first instruction of the 
 *      handler procedure.
 *
 *   The difference between an interrupt gate and a trap gate is as follows.
 *   If an interrupt or exception handler is called through an interrupt 
 *   gate, the processor clear the interrupt enable (IF) flag in the EFLAGS
 *   register to prevent subsequent interrupts from interfering with the
 *   execution of the handler. When a handler is called through a trap gate,
 *   the state of the IF flag is not changed.
 */

/* Interrupt CALL and IRET Operation on same privilege level
 *   If the code segment for the handler procedure has the same privilege
 *   privilege level as the currently executing program or task, the handler
 *   procedure uses the current stack. If the handler executes at a more
 *   privileged level, the processor switches to the stack for the handler's
 *   privilege level.
 *
 *   If no stack switch occures, the processor does the following when calling
 *   an interrupt or exception handler:
 *
 *   1) Pushes the current contents of the EFLAGS, CS, and EIP registers (
 *      in the order) on the stack.
 *
 *   2) Pushes an error code (if appropriate) on the stack.
 *
 *   3) Loads the segment selector for the new code segment and the new
 *      instruction pointer (from the interrupt gate or trap gate) into
 *      the SS and EIP registers, respectively.
 *
 *   4) Clear the IF flag in the EFLAGS register.
 *
 *   5) Begins exception of the handler procedure.
 *
 *   A return from an interrupt or exceeption handler is initiated with the
 *   IRET instrcution. The IRET instruction is similar to the far RET 
 *   instruction, exception that is also restores the contents of the EFLAGS
 *   register for the interrupted procedure. When executing a return from
 *   an interrupt or exception handler from the same privilege level as the
 *   interrupt procedure, the processor performs these actions:
 *
 *   1) Restores the CS and EIP registers to their values prior to the 
 *      interrupt or exception.
 *
 *   2) Restores the EFLAGS register.
 *
 *   3) Increments the stack pointer appropriately.
 *
 *   4) Resumes execution of the insterrupted procedure.
 */
static void same_privilege_interrupt_return(void)
{
    unsigned long eflags = 0;
    unsigned long eip = 0;
    unsigned long error_code = 0;
    unsigned short cs = 0;

    /* 
     * Stack: layer 
     * 
     * |------------------------------------------------|
     * |  EFLAG (Calling Procedure)                     |
     * |------------------------------------------------|
     * |  Code Segment (Calling Procedure)              |
     * |------------------------------------------------|
     * |  EIP (Calling Procedure)                       |
     * |------------------------------------------------|
     * |  ERROR CODE (Calling Procedure)                |
     * |------------------------------------------------|
     * |  Local Stack (Called Procedure)                |
     * |------------------------------------------------|
     * |  ESP (Current Procedure)                       |
     * |------------------------------------------------|  
     */
    __asm__ volatile ("pushl %%ebp\n\r"
            "movl %%esp, %%ebp\n\r"
            "addl $0x1C, %%esp\n\r" /* system allocate 0x18 to local stack */
            "popl %%eax\n\r"
            "popl %%ebx\n\r"
            "popl %%ecx\n\r"
            "popl %%edx\n\r"
            "movl %%ebp, %%esp\n\r"
            "popl %%ebp"
            : "=a" (error_code), "=b" (eip), 
              "=c" (cs), "=d" (eflags));
    printk("Interrupt:EFLAGS [%#x] CS [%#x] EIP [%#x] ERROR_CODE [%#x]\n",
            eflags, cs, eip, error_code);
    if (!((eflags >> 8) & 0x1))
        printk("Interrupt Gate clear TF flag on EFLAGS\n");

    /* return from interrupt: ESP points EIP for calling procedure */
    __asm__ volatile ("addl $0x1C, %%esp\n\r"
                      "iret"
                      ::);
}

/*
 * Call Operation for Interrupt or Exception Handling Procedure
 *   A call to an interrupt or exception handler procedure is similar to a
 *   procedure call to another protection. Here, the vector reference one
 *   of two kinds of gates in the IDT: an interrupt gate or a trap gate.
 *   Interrupt and trap gates are similar to call gates in that they provide
 *   the following information:
 * 
 *   1) Access rights information
 *
 *   2) The segment selector for the code segment that contains the handler
 *      procedure.
 *
 *   3) An offset into the code segment to the first instruction of the 
 *      handler procedure.
 *
 *   The difference between an interrupt gate and a trap gate is as follows.
 *   If an interrupt or exception handler is called through an interrupt 
 *   gate, the processor clear the interrupt enable (IF) flag in the EFLAGS
 *   register to prevent subsequent interrupts from interfering with the
 *   execution of the handler. When a handler is called through a trap gate,
 *   the state of the IF flag is not changed.
 */
static void establish_interrupt_gate(void)
{
    set_intr_gate(0x99, &same_privilege_interrupt_return);

    /* trigger this interrupt */
    __asm__ ("int $0x99");
}

/* Trap CALL and IRET Operation on same privilege level
 *   If the code segment for the handler procedure has the same privilege
 *   privilege level as the currently executing program or task, the handler
 *   procedure uses the current stack. If the handler executes at a more
 *   privileged level, the processor switches to the stack for the handler's
 *   privilege level.
 *
 *   If no stack switch occures, the processor does the following when calling
 *   an interrupt or exception handler:
 *
 *   1) Pushes the current contents of the EFLAGS, CS, and EIP registers (
 *      in the order) on the stack.
 *
 *   2) Pushes an error code (if appropriate) on the stack.
 *
 *   3) Loads the segment selector for the new code segment and the new
 *      instruction pointer (from the interrupt gate or trap gate) into
 *      the SS and EIP registers, respectively.
 *
 *   4) Begins exception of the handler procedure.
 *
 *   A return from an interrupt or exceeption handler is initiated with the
 *   IRET instrcution. The IRET instruction is similar to the far RET 
 *   instruction, exception that is also restores the contents of the EFLAGS
 *   register for the interrupted procedure. When executing a return from
 *   an interrupt or exception handler from the same privilege level as the
 *   interrupt procedure, the processor performs these actions:
 *
 *   1) Restores the CS and EIP registers to their values prior to the 
 *      interrupt or exception.
 *
 *   2) Restores the EFLAGS register.
 *
 *   3) Increments the stack pointer appropriately.
 *
 *   4) Resumes execution of the insterrupted procedure.
 */
static void same_privilege_trap_return(void)
{
    unsigned long eflags;
    unsigned long eip;
    unsigned long error_code;
    unsigned short cs;

    /* Obtain information from Stack of calling procedure
     * Stack: layer 
     * 
     * |------------------------------------------------|
     * |  EFLAG (Calling Procedure)                     |
     * |------------------------------------------------|
     * |  Code Segment (Calling Procedure)              |
     * |------------------------------------------------|
     * |  EIP (Calling Procedure)                       |
     * |------------------------------------------------|
     * |  ERROR CODE (Calling Procedure)                |
     * |------------------------------------------------|
     * |  Local Stack (Called Procedure)                |
     * |------------------------------------------------|
     * |  ESP (Current Procedure)                       |
     * |------------------------------------------------|  
     */
    __asm__ volatile ("pushl %%ebp\n\r"
            "movl %%esp, %%ebp\n\r"
            "addl $0x1C, %%esp\n\r" /* system allocate 0x18 to local stack */
            "popl %%eax\n\r"
            "popl %%ebx\n\r"
            "popl %%ecx\n\r"
            "popl %%edx\n\r"
            "movl %%ebp, %%esp\n\r"
            "popl %%ebp"
            : "=a" (error_code), "=b" (eip),
              "=c" (cs), "=d" (eflags));
    printk("Trap: EFLAGS [%#x] CS [%#x] EIP [%#x] ERROR_CODE [%#x]\n",
               eflags, cs, eip, error_code);
    if ((eflags >> 8) & 0x1)
        printk("Trap Gate doesn't clear TF flag on Eflags\n");

    /* Return from trap: ESP points EIP for calling procedure */
    __asm__ volatile ("addl $0x1C, %%esp\n\r"
                      "iret"
                      ::);
}

/*
 * Call Operation for Interrupt or Exception Handling Procedure
 *   A call to an interrupt or exception handler procedure is similar to a
 *   procedure call to another protection. Here, the vector reference one
 *   of two kinds of gates in the IDT: an interrupt gate or a trap gate.
 *   Interrupt and trap gates are similar to call gates in that they provide
 *   the following information:
 * 
 *   1) Access rights information
 *
 *   2) The segment selector for the code segment that contains the handler
 *      procedure.
 *
 *   3) An offset into the code segment to the first instruction of the 
 *      handler procedure.
 *
 *   The difference between an interrupt gate and a trap gate is as follows.
 *   If an interrupt or exception handler is called through an interrupt 
 *   gate, the processor clear the interrupt enable (IF) flag in the EFLAGS
 *   register to prevent subsequent interrupts from interfering with the
 *   execution of the handler. When a handler is called through a trap gate,
 *   the state of the IF flag is not changed.
 */
static void establish_trap_gate(void)
{
    set_trap_gate(0x88, &same_privilege_trap_return);

    /* trigger this trap */
    __asm__("int $0x88");
}

/* 
 * Trap CALL Operation on different privilege level
 *   When executing a return from an interrupt or exception handler from
 *   a different privilege level than the interrupt procedure, the 
 *   processor performs these actions:
 *
 *   1) Performs a privilege check.
 *
 *   2) Restores the CS and EIP registers theirs values prior to the 
 *      interrupt or exception.
 *
 *   3) Restore the EFLAGS register.
 *
 *   4) Restore the SS and ESP register to their values prior to the
 *      interrupt or exception, resulting in a stack switch back to
 *      the stack of the interrupt procedure.
 *
 *   5) Resumes execution of the interrupted procedure.
 */
static void multip_privilege_trap_return(void)
{
    unsigned long esp, eflags, eip, error_code;
    unsigned short ss, cs;

    __asm__ volatile ("pushl %%ebp\n\r"
            "movl %%esp, %%ebp\n\r"
            "addl $0x48, %%esp\n\r"
            "popl %%eax\n\r"
            "popl %%ebx\n\r"
            "popl %%ecx\n\r"
            "popl %%edx\n\r"
            "popl %%edi\n\r"
            "popl %%esi\n\r"
            "movl %%ebp, %%esp\n\r"
            "popl %%ebp"
            : "=a" (error_code), "=b" (eip), "=c" (cs),
              "=d" (eflags), "=D" (esp), "=S" (ss));
    printk("Trap: EIP [%#x] CS [%#x] ESP [%#x] SS [%#x] EFLAGS [%#x] "
           "ERROR [%#x]\n", eip, cs, esp, ss, eflags, error_code);
}

/* 
 * Trap CALL Operation on different privilege level
 *   If a stack switch does occur, the processor does the following:
 *
 *   1) Temporarily saves (internally) the current contents of the SS,
 *      ESP, EFLAGS, CS and EIP registers.
 *
 *   2) Loads the segment selector and stack pointer for the new stack (that
 *      is, the stack for the privilege level being called) from the TSS
 *      into the SS and ESP registers and swithes to the new stack.
 *
 *   3) Pushes the temporarily saved SS, ESP, EFLAGS, CS, and EIP values
 *      for the interrupted procedure's stack onto the new stack.
 *
 *   4) Pushes an error code on the new stack (if appropriate).
 *
 *   5) Loads the segment selector for the new code segment and the new
 *      instruction pointer (from the trap gate) into to the CS and EIP
 *      registers, respectively.
 *
 *   6) Begins execution of the handler procedure at the new privilege
 *      level.
 */
static void establish_trap_gate_multip_privilege(void)
{
    set_system_gate(0x89, &multip_privilege_trap_return);
}
static void trigger_trap_gate_multip_privilege(void)
{
    __asm__ ("int $0x89");
}

void debug_stack_common(void)
{
#ifdef CONFIG_DEBUG_KERNEL_LATER
    diagnose_stack_pointer();
    stack_link_pointer();
    stack_far_call();
    stack_far_return();
    stack_paramenter_pass();
    save_procedure_state();
    establish_trap_gate();
    establish_interrupt_gate();
    if (0) { /* #GP */
        /* Establish a new stack */
        establish_kernel_stack();
    }
#endif

#ifdef CONFIG_DEBUG_USERLAND_EARLY
    trigger_trap_gate_multip_privilege();
#endif

    /* ignore warning */
    if (0) {
        diagnose_call();
        stack_near_call();
        stack_near_return();
        parse_paramter_from_register();
        parse_paramter_from_stack(1, 2, 3, 4);
        parse_argument_list(argument);
        parse_popa(0, 0, 0, 0, 0, 0, 0, 0);
        emulate_user_call_kernel();
        emulate_kernel_return_userland();
        diagnose_stack_pointer();
        stack_link_pointer();
        stack_near_call();
        stack_far_call();
        stack_far_return();
        stack_paramenter_pass();
        save_procedure_state();
        establish_kernel_stack();
        establish_interrupt_gate();
        establish_interrupt_gate();
        establish_trap_gate();
        trigger_trap_gate_multip_privilege();
    } 
}

#ifdef CONFIG_TESTCASE_MULT_PRIVILEGE
/* debug kernel stack on userland */
void debug_stack_kernel_on_userland(void)
{
    establish_trap_gate_multip_privilege();
}
#endif
