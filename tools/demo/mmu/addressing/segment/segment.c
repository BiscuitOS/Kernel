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

/*
 * Segment selector
 * A segment selector is a 16-bit identifier for a segment (See Figure). 
 * It does not point directly to the segment, but instead points to the 
 * segment descriptor that defines the segment. A segment selector contains 
 * the following items:
 *
 *
 *       15                                        3    2     0
 *       +-----------------------------------------+----+-----+
 *       |               Index                     | TI | RPL |
 *       +-----------------------------------------+----+-----+
 *                                                    A    A
 *                                                    |    |
 *       Table Indicator -----------------------------o    |
 *         0 = GDT                                         |
 *         1 = LDT                                         |
 *       Request Privilege Level (RPL) --------------------o
 *
 *
 * index   (Bit 3 through 15) -- Selects one of 8192 descriptor in the GDT or
 *         LDT. The processor multiplies the index value by 8 (the number of
 *         bytes in a segment descriptor) and adds the result to the base
 *         address of the GDT or LDT (from the GDTR or LDTR register).
 *
 * TI (table indicator) flag
 *         (Bit 2) -- Specifies the descriptor table to use: clearing this
 *         flag selects the GDT; setting this flag selects the current LDT.
 *
 * Requested Privilege Level (RPL)
 *         (Bits 0 and 1) -- Specifies the privilege level of the selector.
 *         The privilege level can range from 0 to 3, with 0 being the most
 *         privileged level.
 *
 * The first entry of the GDT is not used by the processor. A segment selector
 * that points to this entry of the GDT (that is, a semgnet selector with an
 * index of 0 and TI flag set to 0) is used as a null segment selector. The
 * processor does not generate an exception when a segment register(other than
 * the CS or SS registers) is loaded with a null selector. It does, however,
 * generate an exception when a segment register holding a null selector is 
 * used to access memory. A null selector can be used to initialize unused 
 * segment registers. Loading the CS or SS register with a null segment 
 * selector causes a general-protection exception (#GP) to be generated.
 *
 * Segment selectors are visible to application programs as part of a pointer
 * variable, but the values of selectors are usually assigned or modified by
 * link editors or links loaders, not application programs.
 */
static int segment_selector_entence(void)
{
    unsigned short __unused Sel;
    unsigned short __unused CS;
    unsigned short __unused DS;
    unsigned short __unused SS;
    unsigned short __unused FS;
    unsigned short __unused ES;
    unsigned short __unused GS;
    unsigned int __unused limit;
    unsigned int __unused base;
    unsigned int __unused right;
    unsigned int __unused offset;
    unsigned int __unused seg[2];
    const char __unused *hello = "Hello BiscuitOS";

#ifdef CONFIG_DEBUG_SEG_INS_LDS
    /*
     * LDS -- Loading far pointer into DS 
     *
     * Loads a far pointer (segment selector and offset) from the second 
     * operand (source operand) into a segment register and the first operand
     * (destination operand). The source operand specifies a 48-bit or a
     * 32-bit pointer in memory depending on the current setting of the 
     * operand-size attribute (32 bits or 16 bits, respectively). The 
     * instruction opcode and the destination operand specify a segment
     * register/general-purpose register pair. The 16bit segment selector
     * from the source operand is loaded into the DS segment selector. The
     * 32-bit or 16-bit offset is loaded into the register soecufued with 
     * the destination operands.
     *
     * If one of these instructions is executed in protected mode, additional
     * information from the segment descriptor pointed to by the segment
     * selector in the source operand is loaded in the hidden part of the 
     * selected segment register.
     *
     * Also in protected mode, a NULL selector (values 0000 through 0003) can
     * be loaded into DS, ES, FS, or GS registers without causing a protection
     * exception. (Any subsequent reference to a segment whose corresponding
     * segment register is loaded with a NULL selector, causes a general-
     * protection exception (#GP) and no memory reference to the segment 
     * occurs.)
     */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));
    __asm__ ("mov %%cs, %0" : "=m" (CS));
    seg[1] = CS & 0xFFFF;
    seg[0] = (unsigned int)(unsigned long)hello;
    __asm__ ("lds %1, %%eax\n\r"
             "movl %%eax, %0" 
             : "=m" (offset) : "m" (seg));
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    printk("LDS: loading DS segment selector...\n");
    printk("Orig-DS: %#x, MOV CS:%#x to DS:%#x\n", Sel, CS, DS);
    printk("Orig-mem: %#x: \"%s\", and Offset %#x: \"%s\"\n", seg[0], 
      (char *)(unsigned long)seg[0], offset, (char *)(unsigned long)offset);
#endif

#ifdef CONFIG_DEBUG_SEG_INS_LES
    /*
     * LES -- Loading a far pointer into ES
     * 
     * Loads a far pointer (segment selector and offset) from the second 
     * operand (source operand) into a segment register and the first operand
     * (destination operand). The source operand specifies a 48-bit or a
     * 32-bit pointer in memory depending on the current setting of the 
     * operand-size attribute (32 bits or 16 bits, respectively). The 
     * instruction opcode and the destination operand specify a segment
     * register/general-purpose register pair. The 16bit segment selector
     * from the source operand is loaded into the ES segment selector. The
     * 32-bit or 16-bit offset is loaded into the register soecufued with 
     * the destination operands.
     *
     * If one of these instructions is executed in protected mode, additional
     * information from the segment descriptor pointed to by the segment
     * selector in the source operand is loaded in the hidden part of the 
     * selected segment register.
     *
     * Also in protected mode, a NULL selector (values 0000 through 0003) can
     * be loaded into DS, ES, FS, or GS registers without causing a protection
     * exception. (Any subsequent reference to a segment whose corresponding
     * segment register is loaded with a NULL selector, causes a general-
     * protection exception (#GP) and no memory reference to the segment 
     * occurs.)
     */
    __asm__ ("mov %%es, %0" : "=m" (Sel));
    __asm__ ("mov %%cs, %0" : "=m" (CS));
    seg[1] = CS & 0xFFFF;
    seg[0] = (unsigned int)(unsigned long)hello;
    __asm__ ("les %1, %%eax\n\r"
             "movl %%eax, %0"
             : "=m" (offset) : "m" (seg));
    __asm__ ("mov %%es, %0" : "=m" (ES));
    printk("LES: Loading ES segment selector...\n");
    printk("Orig-ES: %#x, MOV SS:%#x to ES:%#x\n", Sel, CS, ES);
    printk("Orig-mem: %#x: \"%s\", and Offset %#x: \"%s\"\n", seg[0],
      (char *)(unsigned long)seg[0], offset, (char *)(unsigned long)offset);
#endif

#ifdef CONFIG_DEBUG_SEG_INS_LGS
    /*
     * LGS -- Loading a far pointer into GS
     * 
     * Loads a far pointer (segment selector and offset) from the second 
     * operand (source operand) into a segment register and the first operand
     * (destination operand). The source operand specifies a 48-bit or a
     * 32-bit pointer in memory depending on the current setting of the 
     * operand-size attribute (32 bits or 16 bits, respectively). The 
     * instruction opcode and the destination operand specify a segment
     * register/general-purpose register pair. The 16bit segment selector
     * from the source operand is loaded into the GS segment selector. The
     * 32-bit or 16-bit offset is loaded into the register soecufued with 
     * the destination operands.
     *
     * If one of these instructions is executed in protected mode, additional
     * information from the segment descriptor pointed to by the segment
     * selector in the source operand is loaded in the hidden part of the 
     * selected segment register.
     *
     * Also in protected mode, a NULL selector (values 0000 through 0003) can
     * be loaded into DS, ES, FS, or GS registers without causing a protection
     * exception. (Any subsequent reference to a segment whose corresponding
     * segment register is loaded with a NULL selector, causes a general-
     * protection exception (#GP) and no memory reference to the segment 
     * occurs.)
     */
    __asm__ ("mov %%gs, %0" : "=m" (Sel));
    __asm__ ("mov %%cs, %0" : "=m" (CS));
    seg[1] = CS & 0xFFFF;
    seg[0] = (unsigned int)(unsigned long)hello;
    __asm__ ("lgs %1, %%eax\n\r"
             "movl %%eax, %0"
             : "=m" (offset) : "m" (seg));
    __asm__ ("mov %%gs, %0" : "=m" (GS));
    printk("LGS: Loading GS segment selector...\n");
    printk("Orig-GS: %#x, MOV CS:%#x to GS:%#x\n", Sel, CS, GS);
    printk("Orig-mem: %#x: \"%s\", and Offset %#x: \"%s\"\n", seg[0],
      (char *)(unsigned long)seg[0], offset, (char *)(unsigned long)offset);
#endif

#ifdef CONFIG_DEBUG_SEG_INS_LFS
    /*
     * LFS -- Loading a far pointer into FS
     * 
     * Loads a far pointer (segment selector and offset) from the second 
     * operand (source operand) into a segment register and the first operand
     * (destination operand). The source operand specifies a 48-bit or a
     * 32-bit pointer in memory depending on the current setting of the 
     * operand-size attribute (32 bits or 16 bits, respectively). The 
     * instruction opcode and the destination operand specify a segment
     * register/general-purpose register pair. The 16bit segment selector
     * from the source operand is loaded into the FS segment selector. The
     * 32-bit or 16-bit offset is loaded into the register soecufued with 
     * the destination operands.
     *
     * If one of these instructions is executed in protected mode, additional
     * information from the segment descriptor pointed to by the segment
     * selector in the source operand is loaded in the hidden part of the 
     * selected segment register.
     *
     * Also in protected mode, a NULL selector (values 0000 through 0003) can
     * be loaded into DS, ES, FS, or GS registers without causing a protection
     * exception. (Any subsequent reference to a segment whose corresponding
     * segment register is loaded with a NULL selector, causes a general-
     * protection exception (#GP) and no memory reference to the segment 
     * occurs.)
     */
    __asm__ ("mov %%fs, %0" : "=m" (Sel));
    __asm__ ("mov %%cs, %0" : "=m" (CS));
    seg[1] = CS & 0xFFFF;
    seg[0] = (unsigned int)(unsigned long)hello;
    __asm__ ("lfs %1, %%eax\n\r"
             "movl %%eax, %0"
             : "=m" (offset) : "m" (seg));
    __asm__ ("mov %%fs, %0" : "=m" (FS));
    printk("LFS: Loading FS segment selector...\n");
    printk("Orig-FS: %#x, MOV CS:%#x to FS:%#x\n", Sel, CS, FS);
    printk("Orig-mem: %#x: \"%s\", and Offset %#x: \"%s\"\n", seg[0],
      (char *)(unsigned long)seg[0], offset, (char *)(unsigned long)offset);
#endif

#ifdef CONFIG_DEBUG_SEG_INS_LSS
    /*
     * LSS -- Loading a far pointer into SS
     * 
     * Loads a far pointer (segment selector and offset) from the second 
     * operand (source operand) into a segment register and the first operand
     * (destination operand). The source operand specifies a 48-bit or a
     * 32-bit pointer in memory depending on the current setting of the 
     * operand-size attribute (32 bits or 16 bits, respectively). The 
     * instruction opcode and the destination operand specify a segment
     * register/general-purpose register pair. The 16bit segment selector
     * from the source operand is loaded into the SS segment selector. The
     * 32-bit or 16-bit offset is loaded into the register soecufued with 
     * the destination operands.
     *
     * If one of these instructions is executed in protected mode, additional
     * information from the segment descriptor pointed to by the segment
     * selector in the source operand is loaded in the hidden part of the 
     * selected segment register.
     *
     * Also in protected mode, a NULL selector (values 0000 through 0003) can
     * be loaded into DS, ES, FS, or GS registers without causing a protection
     * exception. (Any subsequent reference to a segment whose corresponding
     * segment register is loaded with a NULL selector, causes a general-
     * protection exception (#GP) and no memory reference to the segment 
     * occurs.)
     */
    __asm__ ("mov %%ss, %0" : "=m" (Sel));
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    seg[1] = DS & 0xFFFF;
    seg[0] = (unsigned int)(unsigned long)hello;
    __asm__ ("lss %1, %%eax\n\r"
             "movl %%eax, %0"
             : "=m" (offset) : "m" (seg));
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    printk("LSS: Loading SS segment selector...\n");
    printk("Orig-SS: %#x, MOV CS:%#x to SS:%#x\n", Sel, DS, SS);
    printk("Orig-mem: %#x: \"%s\", and Offset %#x: \"%s\"\n", seg[0],
      (char *)(unsigned long)seg[0], offset, (char *)(unsigned long)offset);
#endif

#ifdef CONFIG_DEBUG_SEG_INS_LSL
    /*
     * Load segment limit
     *
     * Loads the unscrambled segment limit from segment descriptor specified
     * with the second operand (source operand) into the first operand (
     * destination operand) and sets the ZF flag in the EFLAGS register. The
     * source operand (which can be a register or a memory location) contains
     * the segment selector for the segment descriptor being accessed. The
     * destination operand is a general-purpose register.
     *
     * The processor performs access checks as part of the loading process. 
     * Once loaded in the destination register, software can compare the 
     * segment limit with the offset of a pointer.
     *
     * The segment limit is a 20-bit value contained in byte 0 and 1 and in 
     * first 4 bits of byte 6 if the segment descriptor. If the descriptor
     * has a byte granular segment limit (the granularity flag is set to 0),
     * the destination operand is loaded with a byte grnular value (byte 
     * limit). If the descriptor has a page granular segment limit (the 
     * granularity flag is set to 1), the LSL instruction will translate the
     * page granular limit (page limit) into a byte limit before loading
     * it into the destination operand. The translation is performed by 
     * shifting the 20-bit "raw" limit left 12 bits and filling the low-order
     * 12 bits with 1s.
     *
     * When the operand size is 32 bits, the 32-bit byte limit is stored
     * in the destination operand. When the operand size is 16 bits, a valid
     * 32-bit limit is computed; however, the upper 16 bits are truncated and
     * only the low-order 16 bits are loaded into the destination operand.
     *
     * This instruction performs the following checks before it loaded the 
     * segment limit into the destinaion register:
     *
     * * Checks that at the segment selector is not NULL.
     *
     * * Checks that the segment selector points to a descriptor that is 
     *   within the limits of the GDT or LDT being accessed.
     *
     * * Check that the descriptor type is valid for this instruction. All
     *   code and data segment descriptor are valid for (can be accessed with)
     *   the LSL instruction. 
     *
     * If the segment descriptor cannot be accessed or is an invalid type for
     * the instruction, the ZF flag is cleared and no value is loaded in the
     * destination operand.
     */

    /* Obtain a unscrambled segment selector */
    __asm__ ("mov %%ds, %0" : "=r" (Sel));
    /* Load segment limit */
    __asm__ ("lsl %1, %0" : "=r" (limit) : "r" (Sel));
    printk("Sel: %#x limit: %#x\n", Sel, limit + 1);
#endif

#ifdef CONFIG_DEBUG_SEG_INS_LAR
    /*
     * LAR -- Load Access right Byte
     *
     * Loads the access rights from the segment descriptor specified by
     * the second operand (source operand) into the first operand (
     * destination operand) and sets the ZF flag in the flag register. The
     * source operand (which can be a register or a memory location) contains
     * the segment selector for the segment descriptor being accessed. If
     * the source operand is a memory address, only 16 bits of data are
     * accessed. The destination operand is general-purpose register.
     * 
     * The processor performs access checks as part of the loading process.
     * Once loaded in the destination register, software can perform addition
     * checks on the access rights information.
     *
     * The access rights for a segment descriptor include fields located in
     * the second doubleword (bytes 4-7) of the segment descriptor. The
     * following fields are loaded by the LAR instruction:
     *
     * * Bits 7:0   returned as 0
     * * Bits 11:8  return the segment type
     * * Bit 12     return the S flag
     * * Bits 14:13 return the DPL
     * * Bit 15     return the P flag
     * * The following fields are returned only if the operand size is greater
     *   than 16 bits:
     *   -- Bit 19:16 are undefined.
     *   -- Bit 20 returns the software-available bit in the descriptor.
     *   -- Bit 21 return the L flag.
     *   -- Bit 22 return the D/B flag.
     *   -- Bit 23 return the G flag.
     *   -- Bit 31:24 are return as 0.
     *
     * This instruction performs the following checks before it loads the 
     * access rights in the destination register:
     * 
     * * Checks that the segment selector is not NULL.
     * * Checks that the segment selector points to a descriptor that is 
     *   within the limits of the GDT or LDT being accessed.
     * * Checks that the descriptor type is valid for this instruction. All 
     *   code and data segment descriptors are valid for (can be accessed
     *   with) the LAR instruction. 
     * * If the segment is not a conforming code segment, it checks that the
     *   specified segment descriptor is visible at the CPL (that is, if the
     *   CPL and the RPL of the segment selector are less than or equal to
     *   the DPL of the segment selector).
     *
     * If the segment descriptor cannot be accessed or is an invalid type
     * for the instruction, the ZF flag is cleared and no access rights are
     * loaded in the destination operand.   
     *
     */
    /* Obtain a segment selector */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));
    /* Load segment access right */
    __asm__ ("lar %1, %0" : "=r" (right) : "r" (Sel));
    printk("Sel: %#x right: %#x\n", Sel, right);
#endif

#ifdef CONFIG_DEBUG_SEG_INS_MOV
    /*
     * MOV instruction
     */
    /* Obtain special register */
    __asm__ ("mov %%ds, %0" : "=m" (DS));
    /* Moving CS to DS */
    __asm__ ("mov %%cs, %0" : "=m" (CS));
    /* Destination segment selector */
    __asm__ ("mov %1, %0" : "=r" (Sel) : "r" (CS));
    printk("Orignal DS: %#x => %#x\n", DS, Sel);
#endif

#ifdef CONFIG_DEBUG_SEG_INS_POP
    /*
     * POP instruction
     */
    /* prepare seg */
    __asm__ ("mov %%cs, %0\n\r"
             "mov %%ds, %1" : "=m" (CS), "=m" (DS));
    __asm__ ("push %1\n\r"
             "pop %%ds\n\r"
             "mov %%ds, %0" : "=m" (Sel) : "m" (CS));

    printk("Orignal DS: %#x => %#x\n", DS, Sel);
#endif

#ifdef CONFIG_DEBUG_SEG_CS

#endif

    return 0;
}

/*
 * Global Descriptor Table Register(GDTR)
 *
 * The GDTR regsiter holds the base address (32 bits in protected mode) 
 * and the 16-bit table limit for the GDT. The base address specifies 
 * the linear address of byte 0 of the GDT; the table limit specifies 
 * the number of bytes in the table.
 *
 *
 *      47                             16 15                      0
 *      +--------------------------------+------------------------+
 * GDTR |   32-bit linear Base address   |   16-bit Table limit   |
 *      +--------------------------------+------------------------+
 *
 *
 * The LGDT and SGDT instruction load and store the GDTR register, 
 * respectively. On power up or reset of the processor, the base address 
 * is set to the default value of 0 and the limit is set to 0xFFFFH. A 
 * new base address must be loaded into the GDTR as part of the processor 
 * initialization process for protected-mode operation.
 */
static int GDTR_entence(void)
{
    unsigned int __unused base;
    unsigned int __unused limit;
    unsigned int __unused virtual;
    unsigned int __unused linear;
    unsigned short __unused GDTR[3];
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;

#ifdef CONFIG_DEBUG_GDTR_SGDT
    /*
     * SGDT -- Store Global Descriptor Table Register
     *
     * Stores the content of the global descriptor table register (GDTR) in
     * the destination operand. The destination operand specifies a memory
     * location.
     *
     * In legacy or compatibility mode, the destination operand is a 6-byte
     * memory location. If the operand-size attribute is 16 or 32 bits, the
     * 16-bit limit field of the register is stored in the low 2 bytes of 
     * the memory location and the 32-bit base address is stored in the high
     * 4 bytes.
     *
     * SGDT is useful only by operating-system software. However, it can be
     * used in application programs without causing an exception to be 
     * generated if CR4.UMIP = 0.
     */

    __asm__ ("sgdt %0" : "=m" (GDTR));
    /* Base address */
    base = GDTR[1] | (GDTR[2] << 16);
    /* Limit */
    limit = GDTR[0];
    
    /* Virtual address of GDT */
    virtual = (unsigned int)(unsigned long)gdt;
    /* Logical-address of GDT.
     *  Sel: DS (kernel data segment selector)
     *  Offset: virtual address of 'gdt'.
     *  => Sel:Offset 
     */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));

    /* linear address of GDT */
    if (Sel & 0x4) {
        /* Segment Selector locate on LDT */
        desc = current->ldt + (Sel >> 3);
    } else {
        /* Segment Selector locate on GDT */
        desc = gdt + (Sel >> 3);
    }
    linear = get_base(*desc) + virtual;
    
    printk("GDT linear address: %#x\n", linear);
    printk("SGDT: GDTR-base: %#x limit %#x\n", base, limit);
#endif

#ifdef CONFIG_DEBUG_GDTR_LGDT
    /*
     * LGDT -- Load Global Descriptor Table Register
     *
     * Loads the values in the source operand into the global descriptor 
     * table register (GDTR). The source operand specifies a 6-byte memory
     * location that contains the base address (a linear address) and the
     * limit (size of table in bytes) of the global descriptor table (GDT).
     * If operand-size attribute is 32 bits, a 16-bit limit (lower 2 bytes
     * of the 6-byte data operand) and a 32-bit base address (upper 4 bytes
     * of the data operand) are loaded into the register. If the operand-size
     * attribute is 16 bits, a 16-bit limit (lower 2 bytes) and a 24-bit 
     * base address (third, fourth, and fifth byte) are loaded. Here, the 
     * high-order byte of the operand is not used and the high-order byte
     * of the base address in the GDTR is filled with zeros.
     * 
     * The LGDT instruction are used only in operating-system software; they
     * are not used in application programs. They are the only instructions
     * that directly load a linear address (that is, not a segment-relative
     * address) and a limit in protected mode. They are commonly executed in
     * real-address mode to allow processor initialization prior to 
     * switching to protected mode.
     */

    /* Step0: Obtain the virtual address of 'gdt' */
    virtual = (unsigned int)(unsigned long)gdt;

    /* Step1: Obtian the logical-address
     *  The 'gdt' locate on kernel Data segment, so DS as segment selector.
     *  And virtual address as offset for logical address.
     */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));

    /* Step2: Obtian the linear-address
     *  The linear-address contain the base address of segment and offset.
     */
    if (Sel & 0x4) {
        /* Segment locate on LDT */
        desc = current->ldt + (Sel >> 3);
    } else {
        /* Segment locate on GDT */
        desc = gdt + (Sel >> 3);
    }
    if (virtual > get_limit(Sel))
        panic("The Offset over segment limit");
    linear = get_base(*desc) + virtual;

    /* Step3: Obtain gdt limit */

    /* Step4: Load the base address and limit into GDTR */
    GDTR[1] = linear & 0xFFFF;
    GDTR[2] = (linear >> 16) & 0xFFFF;

    __asm__ ("lgdt %0" :: "m" (GDTR));
#endif

    return 0;
}

/*
 * Local Descriptor Table (LDTR)
 *
 * The LDTR register holds the 16-bit segment selector, base address (32 bits
 * in protected mode), segment limit, and descriptor attributes for the LDT.
 * The base address specifies the linear address of byte 0 of the LDT segment;
 * the segment limit specifies the number of bytes in the segment.
 *
 *
 * LDTR
 *
 * 15            0
 * +-------------+  +----------------------------+---------------+-----------+
 * |   Seg.Sel   |  | 32-bit linear base address | Segment limit | Attribute |
 * +-------------+  +----------------------------+---------------+-----------+
 *
 *
 * The LLDT and SLDT instructions load and store the segment selector part of
 * the LDTR register, respectively. The segment that contains the LDT must have
 * a segment descriptor in the GDT. When the LLDT instruction loads a segment
 * selector in the LDTR: the base address, limit, and descriptor attribute from
 * the LDT descriptor are automatically loaded in the LDTR.
 *
 * When a task switch occurs, the LDTR is automatically loaded with the segment
 * selector and descriptor for the LDT for the new task. The contents of the
 * LDTR are not automatically saved prior to writing the new LDT information
 * into the register.
 *
 * On power up or reset of the processor, the segment selector and base address
 * are set to the default value of 0 and the limit is set to 0x0FFFFH.
 */
static int LDTR_entence(void)
{
    unsigned short __unused Sel;

#ifdef CONFIG_DEBUG_LDTR_SLDT
    /*
     * SLDT -- Store local descriptor table register
     *
     * Stores the segment selector from the local descriptor table register (
     * LDTR) in the destination operand. The destination operand can be a
     * general-purpose register or a memory location. The segment selector 
     * stored with this instruction points to the segment descriptor (located
     * in the GDT) for the current LDT. This instruction can only by executed
     * in protected mode.
     *
     * DEST <---- LDTR(SegmentSelector);
     */
    __asm__ ("sldt %0" : "=m" (Sel));

    printk("SLDT: Sel for LDT => %#x\n", Sel);
#endif

#ifdef CONFIG_DEBUG_LDTR_LLDT
    /*
     * LLDT -- Load local descriptor table register
     *
     * Loads the source operand into the segment selector field of the local
     * descriptor table register (LDTR). The source operand (a general-purpose
     * register or a memory location) contains a segment selector that points
     * to a local descriptor table (LDT). After the segment selector is loaded
     * in the LDTR, the processor uses the segment selector to locate the 
     * segment descriptor for the LDT in the global descriptor table (GDT).
     * It then loads the segment limit and base address for the LDT from the 
     * segment descriptor into the LDTR. The segment register DS, ES, SS, FS,
     * GS, and CS are not affectd by this instruction, nor is the LDTR field
     * in the task segment (TSS) for the current task.
     *
     * If bits 2-15 of the source operand are 0, LDTR is marked invalid and
     * LLDT instruction completes silently. However, all subsequent references
     * to descriptors in the LDT (except by the LAR, VERR, VERW or LSL 
     * instructions) cause a general protection exception (#GP).
     */
    __asm__ ("lldt %0" :: "m" (Sel));
#endif
    return 0;
}

/*
 * IDTR Interrupt Descriptor Table Register
 *
 * The IDTR register holds the base address (32 bits in protected mode) and
 * 16-bit table limit for the IDT. The base address specifies the linear 
 * address of byte 0 of the IDT; the table limit specifies the number of 
 * bytes in the table. The LIDT and SIDT instructions load and store the IDTR 
 * register, reppectively. On power up or reset of the processor, the base 
 * address is set to the default value of 0 and the limit is set to 0x0FFFFH. 
 * The base address and limit in the register can then be changed as part of 
 * the processor initialization process.
 *
 *
 * IDTR
 * 
 *      47                             16 15                      0
 *      +--------------------------------+------------------------+
 *      |   32-bit linear Base address   |   16-bit Table limit   |
 *      +--------------------------------+------------------------+
 *
 */
static int IDTR_entence(void)
{
    unsigned short __unused IDTR[3];
    unsigned short __unused limit;
    unsigned int __unused base;

#ifdef CONFIG_DEBUG_IDTR_SIDT
    /*
     * SIDT -- Store Interrupt Descriptor Table Register
     *
     * Stores the content the interrupt descriptor table register (IDTR) in
     * the destination oprand. The destination operand specifies a 6-byte 
     * memory location.
     * 
     * The 16-bit limit field of the register is stored in the low 2 bytes of
     * the memory location and the 32-bit base address is stored in the high
     * 4 bytes.
     */
    __asm__ ("sidt %0" : "=m" (IDTR));
    /* Base linear address of IDTR */
    base = IDTR[1] | (IDTR[2] << 16);
    /* Limit of IDTR */
    limit = IDTR[0];

    printk("SIDT: IDTR base: %#x limit %#x\n", base, limit);
#endif

#ifdef CONFIG_DEBUG_IDTR_LIDT
    /*
     * LIDT -- Load Interrupt Descriptor Table Register.
     *
     * Loads the values in the source operand into the interrupt descriptor
     * table register (IDTR). The source operand specifies a 6-byte memory
     * location that contains the base address (a linear address) and the 
     * limit (size of table in bytes) of the interrupt descriptor table.
     * If operand-size attribute is 32 bits, a 16-bit limit (lower 2 bytes of
     * the 6-byte data operand) and a 32-bit base address (upper 4 bytes of
     * the data operand) are loaded into the register. If the operand-size
     * attribute is 16 bits, a 16-bit limit (lower 2 bytes) and a 24-bit
     * base address (third, fourth, and fifth byte) are loaded. Here, the
     * high-order byte of the operand is not used and the high-order byte of
     * the base address in the IDTR is filled with zero.
     */
    __asm__ ("lidt %0" :: "m" (IDTR));
#endif
    return 0;
}

/*
 * Task Register (TR)
 *
 * The task register holds the 16-bit segment selector, base address (32 bits 
 * in protected mode), segment limit, and descriptor attribute for the TSS of 
 * the current task. The selector references the TSS descriptor in the GDT. 
 * The base address specifies the linear address of byte 0 of the TSS; the 
 * segment limit specifies the number of bytes in the TSS.
 *
 *
 * Task Register
 *
 * 15            0
 * +-------------+  +----------------------------+---------------+-----------+
 * |   Seg.Sel   |  | 32-bit linear base address | Segment limit | Attribute |
 * +-------------+  +----------------------------+---------------+-----------+
 *
 *
 * The LTR and STR instructions load and store the segment selector part of 
 * the task register, respectively. When the LTR instruction loads a segment 
 * selector in the task register, the base address, limit, and descriptor 
 * attribute from the TSS descriptor are automatically loaded into the task 
 * register. On power up or reset of the processor, the base address is set 
 * to the default value of 0 and the limit is set to 0xFFFFH.
 *
 * When a task switch occurs, the task register is automatically loaded with 
 * the segment selector and descriptor for the TSS for the new task. The 
 * contents of the task register are not automatically saved prior to writing 
 * the new TSS information into the register.
 *
 */
static int TR_entence(void)
{
    unsigned short __unused TR;

#ifdef CONFIG_DEBUG_TR_STR
    /*
     * STR -- Store Task Register
     *
     * Stores the segment selector from the task register (TR) in the 
     * destination operand. The destination operand can be a general-purpose
     * register or a memory location. The segment selector stored with this
     * instruction points to the task state segment (TSS) for the currently
     * running task.
     *
     * When the destination operand is a 32-bit register, the 16-bit segment
     * selector is copied into the lower 16 bits of the register and the upper
     * 16 bits of the register are cleared. When the destination operand is a
     * memory location, the segment selector is written to memory as a 16-bit
     * quantity, regardless of operand size.
     */
    __asm__ ("str %0" : "=m" (TR));

    printk("Task Register: Sel %#x\n", TR);
#endif

#ifdef CONFIG_DEBUG_TR_LTR
    /*
     * LTR -- Load Task Register
     *
     * Loads the source operand into the segment selector field of the task
     * register. The source operand (a general-purpose register or a memory
     * location) contains a segment selector that points to a task state 
     * segment (TSS). After the segment selector is loaded in the task 
     * register, the processor uses the segment selector to locate the segment
     * descriptor for the TSS in the global descriptor (GDT). It then loads 
     * the segment limit and base address for the TSS from the segment 
     * descriptor into the task register. The task pointed to by the task
     * register is marked busy, but a switch to the task does not occur.
     *
     */
    __asm__ ("ltr %0" :: "m" (TR));
#endif
    return 0;
}

static int segment_entence(void)
{
    /* Segment selector entence */
    segment_selector_entence();

    /* Global Descriptor Table Register */
    GDTR_entence();

    /* Local Descriptor Table Register */
    LDTR_entence();

    /* Interrupt Descriptor Table Register */
    IDTR_entence();

    /* Task Register */
    TR_entence();
    return 0;
}

device_debugcall(segment_entence);
