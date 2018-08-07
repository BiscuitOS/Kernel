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
static __unused int gdtr_sdtr(void)
{
    struct gdtr_desc gdtr;

    __asm__ ("sgdt %0" : "=m" (gdtr));

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
    gdtr_sdtr();
#endif

#ifdef CONFIG_DEBUG_GDTR_LGDT
    gdtr_lgdt();
#endif
    return 0;
}
late_debugcall(debug_gdtr);
