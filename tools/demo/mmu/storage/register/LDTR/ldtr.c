/*
 * LDTR: Local Descriptor Table Register
 *
 * (C) 2018.08.09 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>

#include <demo/debug.h>

/*
 * SLDT -- Store Local Descriptor Table Register
 *
 *   SLDT m
 *
 *   Stores the segment selector from the local descriptor table regsiter in
 *   the destination operand. The destination operand can be a general-purpos
 *   register or a memory location. The segment selector stored with this
 *   instruction points to the segment descriptor (located in the GDT) for
 *   the current LDT. This instruction can only be executed in protect mode.
 *
 *   DEST <---- LDTR (Segnemnt Selector)
 */
static __unused int ldtr_sldt(void)
{
    unsigned short sel;

    __asm__ ("sldt %0" : "=m" (sel) :);

    return 0;
}

/*
 * LLDT - Load Local Descriptor Table Register
 *
 *
 *   Loads the source operand into the segment selector field of the local
 *   descriptor table register (LDTR). The source operand (a general-purpose
 *   register or a memory location) contains a segment selector that points
 *   to a local descriptor table (LDT). After the segment selector is loaded
 *   in the LDTR, the processor uses the segment selector to locate the 
 *   segment descriptor for the LDT in the global descriptor table (GDT). It
 *   then loads the segment limit and base address for the LDT from the 
 *   segment descritor into the LDTR. Then segment registers DS, ES, SS,
 *   FS, GS, and CS are not affected by this instruction, nor is the LDTR
 *   field in the task state segment (TSS) for the current task.
 *
 *   If bits 2-15 of the source operand are 0, LDTR is marked invalid and
 *   the LLDT instruction completes silently. However, all subsequent 
 *   references to descriptors in the LDT (except by the LAR, VERR, VERW or
 *   LSL instrcutions) cause a general protection exception (#GP).
 *
 *   The operand-size attribute has no effect on this instruction.
 *
 *   The LLDT instruction is provided for use in operating-system software.
 *   It should not be used in application programs. This instruction can
 *   only be executed in protected mode.
 *
 *   IF SRC(Offset) > descriptor table limit
 *       THEN #GP (segment selector);
 *   FI
 *   IF segment selector is valid
 *       Read segment descriptor
 *       IF Segment Descriptor(Type) != LDT
 *           THEN #GP (segment selector);
 *       FI
 *       IF Segment descriptor is not present
 *           THEN #NP (segment selector);
 *       FI
 *
 *       LDTR(Segment Selector)    <--- SRC;
 *       LDTR(Segment Descriptor)  <--- GDT Segment Descriptor
 *   ELSE LDTR <---- INVALID
 *   FI
 */
static __unused int ldtr_lldt(void)
{
    unsigned short sel;

    __asm__ ("sldt %0" : "=m" (sel) :);

    __asm__ ("lldt %0" :: "m" (sel));

    return 0;
}

static int debug_gdtr(void)
{
#ifdef CONFIG_DEBUG_GDTR_SLDT
    ldtr_sldt();
#endif

#ifdef CONFIG_DEBUG_GDTR_LLDT
    ldtr_lldt();
#endif
    return 0;
}
late_debugcall(debug_gdtr);
