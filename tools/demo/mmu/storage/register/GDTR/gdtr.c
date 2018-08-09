/*
 * GDTR: Global Descriptor Table Register
 *
 * (C) 2018.08.07 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>

#include <demo/debug.h>

struct gdtr_desc 
{
    unsigned short limit;
    unsigned long linear;
};

/*
 * SGDT -- Store Global Descriptor Table Register
 *  
 *   SGDT m
 *
 *   Stores the content of the global descriptor table register (GDTR) in
 *   the destination operand. The destination operand specifies a memory
 *   location.
 *
 *   In legacy or compatibility mode, the destination operand is a 6-byte
 *   memory location. If the operand-size attribute is 16 or 32 bits, the
 *   16-bit limit field of the register is store in the low 2 bytes of the
 *   memory location and the 32-bit base address is stored in the high
 *   4 bytes.
 *
 *   SGDT is useful only by operating-system software. However, it can be
 *   used in application program without causing an exception to be 
 *   generated if CR4.UMIP = 0.
 *
 *   Operation
 *   IF instruction is SGDT
 *       IF OperandSize = 16 or OperandSize = 32 (Legacy or Compatibility Mode)
 *           DEST[0:15]  <----- GDTR(limit)
 *           DEST[16:47] <----- GDTR(Base) (Full 32-bit base address stored)
 *       ELSE (64-bit Mode)
 *           DEST[0:15]  <----- GDTR(limit)
 *           DEST[16:79] <----- GDTR(Base) (Full 64-bit base address stored)
 *       FI
 *   FI
 *
 */
/* Operand of SGDT */
static __unused int gdtr_sgdt(void)
{
    struct gdtr_desc gdtr;

    __asm__ ("sgdt %0" : "=m" (gdtr));

    /*
     * limit
     *   The table limit specifies the number of bytes in the table.
     *   On GDT, the number of bytes for a segment descriptor is 8 bytes,
     *   and the first eight of GDT is reserved by system, detail as follow:
     *
     *   +----------------------------------------------------+
     *   |   Number   |              Describe                 |
     *   +----------------------------------------------------+
     *   | 0          | NULL Descriptor                       | 
     *   +----------------------------------------------------+
     *   | 1          | no use                                | 
     *   +----------------------------------------------------+
     *   | 2          | Kernel Code segment descriptor        | 
     *   +----------------------------------------------------+
     *   | 3          | Kernel Data segment descriptor        | 
     *   +----------------------------------------------------+
     *   | 4          | User Code segment descriptor          | 
     *   +----------------------------------------------------+
     *   | 5          | User Data segment descriptor          | 
     *   +----------------------------------------------------+
     *   | 6          | no use                                | 
     *   +----------------------------------------------------+
     *   | 7          | no use                                | 
     *   +----------------------------------------------------+
     *   | 8          | Task0 LDT segment descriptor          | 
     *   +----------------------------------------------------+
     *   | 9          | Task0 TSS segment descriptor          | 
     *   +----------------------------------------------------+
     *   | ....       | ....                                  | 
     *   +----------------------------------------------------+
     *
     *   The system defined a structure to manage GDT that named 'gdt' which
     *   defined on 'arch/x86/boot/head.S' as follow:
     *
     *   .align 4
     *   gdt:
     *       .quad 0x0000000000000000   NULL descriptor
     *       .quad 0x0000000000000000   not used 
     *       .quad 0xc0c39a000000ffff   0x10 kernel 1GB code at 0xC0000000
     *       .quad 0xc0c392000000ffff   0x18 kernel 1GB data at 0xC0000000
     *       .quad 0x00cbfa000000ffff   0x23 user   3GB code at 0x00000000
     *       .quad 0x00cbf2000000ffff   0x2b user   3GB data at 0x00000000
     *       .quad 0x0000000000000000   not used
     *       .quad 0x0000000000000000   not used
     *       .fill 2*NR_TASKS,8,0       space for LDT's and TSS's etc
     *
     *   On GDT, the first eight segment descriptor reserved by system, and
     *   each task contain two segment descriptor (TSS and LDT). So the 
     *   number of segment descriptor on GDT is:
     *
     *       number = 8 + 2 * NR_TASKS
     *   
     *   So the total number of byte for GDT is:
     * 
     *       total = (8 + 2 * NR_TASKS) * 8
     *
     *   Becase the first byte of GDT is 0, so the total number of GDT is:
     *
     *       total = (8 + 2 * NR_TASKS) * 8 - 1
     */
    if (gdtr.limit != ((8 + 2 * NR_TASKS) * 8 - 1))
        panic("SGDT: invalid gdtr limit");
    return 0;
}

/*
 * LGDT - Load Global Descriptor Table Register
 *
 *   LGDT m32
 *
 *   Loads the values in the source operand into the global descriptor table
 *   register (GDTR). The source operand specifies a 6 byte memory location
 *   that contains the base address (a linear address) and the limit (size
 *   of table in bytes) of the global descriptor table (GDT). If operand-size
 *   attribute is 32 bits, a 16-bit limit (lower 2 bytes of the 6-byte data
 *   operand) and a 32-bit base address (upper 4 bytes of the data operand)
 *   are loaded into the register.
 *
 *   The LGDT instruction are used only in operating-system software. They
 *   are not used in application programs. They are the only instructions
 *   that directly load a linear address (that is, not a segment-relative
 *   address) and a limit in protected mode.
 *
 *   IF Instruction is LGDT
 *       IF OperandSize = 32
 *           THEN
 *               GDTR(limit)   <--- SRC[0:15]
 *               GDTR(Base)    <--- SRC[16:47]
 *           FI
 *       FI
 *   FI
 */
static __unused int gdtr_lgdt(void)
{
    struct gdtr_desc gdtr;

    __asm__ ("sgdt %0" : "=m" (gdtr));

    __asm__ ("lgdt %0" :: "m" (gdtr));

    return 0;
}

static int debug_gdtr(void)
{
#ifdef CONFIG_DEBUG_GDTR_SGDT
    gdtr_sgdt();
#endif

#ifdef CONFIG_DEBUG_GDTR_LGDT
    gdtr_lgdt();
#endif
    return 0;
}
late_debugcall(debug_gdtr);
