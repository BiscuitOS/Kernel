/*
 * IDTR: Interrupt Descriptor Table Register
 *
 * (C) 2018.08.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <demo/debug.h>

struct idtr_desc 
{
    unsigned short limit;
    unsigned long linear;
};

/*
 * SIDT -- Store Interrupt Descriptor Table Register
 *
 *   SIDT m
 *
 *   Stores the content the interrupt descriptor table register (IDTR) in
 *   the destination operand. The destination operand specifies a 6-byte
 *   memory location.
 *
 *   In non-64-bit modes, the 16-bit limit field of the register is stored
 *   in the low 2 bytes of the memory location and the 32-bit base address
 *   is stored in the high 4 bytes.
 *
 *   SIDT is only useful in operating-system software. Howeve, it can be
 *   used in application program without causing an exception to be 
 *   generated if CR4.UMIP = 0.
 *
 *   IF instruction is SIDT
 *       THEN
 *           IF OperandSize = 16 or OperandSize = 32
 *               THEN
 *                   DEST[0:15]   <---- IDTR(limit)
 *                   DEST[16:47]  <---- IDTR(Base); FI
 *               ELSE (* 64-bit Mode *)
 *                   DEST[0:15]   <---- IDTR(limit)
 *                   DEST[16:79]  <---- IDTR(Base) 
 *           FI
 *   FI
 */
static __unused int idtr_sidt(void)
{
    struct idtr_desc idtr;

    __asm__ ("sidt %0" : "=m" (idtr) :);
    return 0;
}

/*
 * LIDT -- Load Interrupt Descriptor Table Register
 *
 *   LIDT m
 *
 *   Loads the values in the source operand into the interrupt descriptor
 *   table register (IDTR). The source operand specifies a 6-byte memory
 *   location that contains the base address (a linear address) and the
 *   limit (size of table in bytes) of the interrupt descriptor table (IDT).
 *   If operand-size attribute is 32 bits, a 16-bit limit (lower 2 bytes of
 *   the 6 byte data operand) and a 32-bit base address (upper 4 bytes of
 *   data operand) are loaded into the register. 
 *
 *   The LIDT instructions are used only in operating-system software. They
 *   are not used in application programs. They are the only instructions 
 *   that directly load a linear address (that is, not a segment-relative
 *   address) and a limit in protected mode. 
 *
 *   IF instruction is LIDT
 *       THEN
 *           IF 32-bit Operand Size
 *               THEN
 *                   IDTR(limit)    <---- SRC[0:15]
 *                   IDTR(Base)     <---- SRC[16:47]
 *               FI
 *           FI
 *       FI
 *   FI
 */
static __unused int idtr_lidt(void)
{
    struct idtr_desc idtr;

    __asm__ ("sidt %0" : "=m" (idtr) :);

    __asm__ ("lidt %0" :: "m" (idtr));
    return 0;
}

static int debug_idtr(void)
{
#ifdef CONFIG_DEBUG_IDTR_SIDT
    idtr_sidt();
#endif

#ifdef CONFIG_DEBUG_IDTR_LIDT
    idtr_lidt();
#endif
    return 0;
}
late_debugcall(debug_idtr);
