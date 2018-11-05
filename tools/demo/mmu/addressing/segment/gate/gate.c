/*
 * Gate mechanism
 *
 * (C) 2018.11.05 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>

#include <demo/debug.h>

#ifdef CONFIG_DEBUG_CALL_GATE_ESTABLISH
/*
 * Establish a lot of debug data segment selectors and segment descriptors.
 *
 * Debug segment selector and segment descriptor list
 *
 * +-------------+--------------------+-------------+-----+-----+----+
 * | Segment Sel | Segment Descriptor | Code/Gates  | CPL | RPL | DPL|
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0203      | 0xc0c39a000000ffff | Code        | 00  | 00  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0213      | 0xc0c3ba000000ffff | Code        | 00  | 00  | 01 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0223      | 0xc0c3da000000ffff | Gode        | 00  | 00  | 02 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0233      | 0xc0c3fa000000ffff | Gode        | 00  | 00  | 03 |
 * +-------------+--------------------+-------------+-----+-----+----+
 * | 0x0240      | 0x0000ec0002000000 | Gates       | 00  | 00  | 00 |
 * +-------------+--------------------+-------------+-----+-----+----+
 */
static struct desc_struct __unused debug_descX[] = {
  { .a = 0x0000ffff, .b = 0xc0c39a00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3ba00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3da00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3fa00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x02030000, .b = 0x0000EC00}, /* Call Gate */
  { }
};

static int __unused establish_debug_call_gate(void)
{
    unsigned short __unused start_Sel = 0x0200;
    unsigned short __unused end_Sel   = 0x0240;
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;
    unsigned short __unused i = 0;

    for (Sel = start_Sel; Sel <= end_Sel; Sel += 0x10, i++) {
        desc = gdt + (Sel >> 0x3);
        desc->a = debug_descX[i].a;
        desc->b = debug_descX[i].b;
    }
 
    return 0;
}
device_debugcall(establish_debug_call_gate);
#endif

static int __unused far_called_procedure(void)
{
    unsigned short __unused CS;

    __asm__ ("mov %%cs, %0" : "=m" (CS));
    printk("CS:  %#x\n", CS);

    return 0;
}

#ifdef CONFIG_DEBUG_SEG_GATE_CALL
/*
 * Access code segment through Call Gate.
 *
 * To access a call gate, a far pointer to the gate is provided as a target
 * operand in a CALL or JMP instruction. The segment selector from this pointer
 * identifies the call gate (See Figure); the offset from the pointer is
 * required, but not used or checked by the processor. (The offset can be set
 * to any value.)
 *
 * When the processor has accessed the call gate, it uses the segment selector
 * from the call gate to locate the segment descriptor for the destination code
 * segment. (This segment descriptor can be in the GDT or the LDT.) It then
 * combines the base address from the code-segment descriptor with the offset
 * from the call gate to form the linear address of the procedure entry point
 * in the code segment.
 */
static int __unused access_call_gate(void)
{
    unsigned short __unused Sel;
    unsigned short __unused CS;
    unsigned short __unused CPL;
    unsigned short __unused RPL;
    unsigned short __unused DPL;
    struct desc_struct __unused *desc;

    __asm__ ("mov %%ss, %0" : "=m" (CS));
    CPL = CS & 0x3;

    /* Call Gate utilize segment selector: 0x0240 
     * Segment Descriptor: 0x0000EC0002000000
     * CPL: 0x03
     * RPL: --
     * DPL: --
     * Code utilize segment selector: 0x0200
     * Segment Descriptor: 0xc0c392000000ffff
     * CPL: --
     * RPL: --
     * DPL: 0x00
     */
    Sel  = 0x0243;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    DPL  = (desc->b >> 13) & 0x3;
    printf("Call Gate: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                          (unsigned int)desc->b, (unsigned int)desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);

    /* Write called pointer entry into CALL gate */
    desc->a |= (unsigned long)far_called_procedure & 0xFFFF;
    desc->b |= (((unsigned long)far_called_procedure >> 16) & 0xFFFF) << 16;

    /* Access code segment through Call gate. Privilege check rule for call
     * Gates:
     * CALL instruction:
     *
     *   CPL <= Call gate DPL; RPL <= Call Gate DPL
     *   Destination conforming code segment DPL <= CPL
     *   Destination nonconforming code segment DPL <= CPL
     **/
    __asm__ ("call $0x0243, %0" :: "i" (far_called_procedure));

    return 0;
}
user1_debugcall_sync(access_call_gate);
#endif
