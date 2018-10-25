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

    return 0;
}

static int segment_entence(void)
{
    /* Segment selector entence */
    segment_selector_entence();

    return 0;
}

device_debugcall(segment_entence);
