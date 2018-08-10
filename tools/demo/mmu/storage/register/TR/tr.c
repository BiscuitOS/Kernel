/*
 * TR: Task Register
 *
 * (C) 2018.08.09 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

/*
 * STR -- Store Task Register
 *
 *   STR m16
 *
 *   Stores the segment selector from the task register (TR) in the
 *   destination operand. The destination operand can be a general-purpose
 *   register or a memory location. The segment selector stored with this
 *   instruction points to the task segment (TSS) for the currently running
 *   task.
 *
 *   When the destination operand is a 32-bit register, the 16-bit segment
 *   selector is copied into the low 16 bits of the register and the upper
 *   16 bits of the register are cleared. When the destination operand is
 *   a memory locating, the segment selector is written to memory as a 16-bit
 *   quantity, regardless of operand size.
 *
 *   DEST <--- TR (Segment Selector)
 *
 */
static __unused int tr_str(void)
{
    unsigned short sel;

    __asm__ ("str %0" : "=m" (sel) :);

    return 0;
}

/*
 * LTR -- Load Task Register
 *
 *   LTR m16
 *
 *   Loads the source operand into the segment selector field of the task
 *   register. The source operand (a general-purpose register or a memory
 *   location) contains a segment selector that points to a task state
 *   segment (TSS). After the segment selector is loaded in the task
 *   register, the processor uses the segment selector to locate the segment
 *   descriptor for the TSS in the global descriptor table (GDT). It then
 *   loads the segment limit and base address for the TSS from the segment
 *   descriptor into the task register. The task pointed to by the task
 *   register is marked busy, but a switch to the task does not occur.
 *
 *   The LTR instruction is provided for use in operating-system software.
 *   It should not be used in application programs. It can only be executed
 *   in protected mode when the CPL is 0. It is commonly used in
 *   initialization code to establish the first task to be executed.
 *
 *   The operand-size attribute has no effect on this instruction.
 *
 *   IF SRC is a NULL selector
 *       THEN #GP(0)
 *   IF SRC(Offset) > descriptor table limit OR IF SRC(type) != global
 *       THEN #GP(Segment selector)
 *   FI
 *   Read segment descriptor
 *   IF Segment descriptor is not for an available TSS
 *       THEN #GP(Segment selector)
 *   IF segment descriptor is not present
 *       THEN #NP(Segment selector)
 *   FI
 *   TSS segment descriptor (busy)    <--- 1
 *   (*locked read-modify-write operation on the entire descriptor when setting
 *      busy flag *)
 *   TaskRegister(SegmentSelector)    <--- SRC
 *   TaskRegister(SegmentDescriptor)  <--- TSS segment descriptor
 */
static __unused int tr_ltr(void)
{
    unsigned short tr;

    __asm__ ("str %0" : "=m" (tr) :);

    __asm__ ("ltr %0" :: "m" (tr));
    return 0;
}

static int debug_tr(void)
{
#ifdef CONFIG_DEBUG_TR_STR
    tr_str();
#endif

#ifdef CONFIG_DEBUG_TR_LTR
    tr_ltr();
#endif
    return 0;
}
late_debugcall(debug_tr);
