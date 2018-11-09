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
 * Establish a lot of debug code segment selectors and segment descriptors.
 *
 * Debug segment selector and segment descriptor list
 *
 * +-------------+--------------------+-------------+----------+-----+
 * | Segment Sel | Segment Descriptor | Conforming  | Ker/User | DPL |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0200      | 0xc0c39e000000ffff | Conforming  | Kernel   | 00  |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0210      | 0xc0c3be000000ffff | Conforming  | Kernel   | 01  |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0220      | 0xc0c3de000000ffff | Conforming  | Kernel   | 02  |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0230      | 0x00cbfe000000ffff | Conforming  | User     | 03  |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0240      | 0xc0c39a000000ffff | Non-Conform | Kernel   | 00  |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0250      | 0xc0c3ba000000ffff | Non-Conform | Kernel   | 01  |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0260      | 0xc0c3da000000ffff | Non-Conform | Kernel   | 02  |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0270      | 0x00cbfa000000ffff | Non-Conform | User     | 03  |
 * +-------------+--------------------+-------------+----------+-----+
 * | 0x0280      | 0x0000000000000000 | Gate        | Reserved | XX  |
 * +-------------+--------------------+-------------+----------+-----+
 */
static struct desc_struct __unused debug_descY[] = {
  { .a = 0x0000ffff, .b = 0xc0c39e00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3be00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3de00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0x00cbfe00}, /* user   3GB code at 0x00000000 */
  { .a = 0x0000ffff, .b = 0xc0c39a00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3ba00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0xc0c3da00}, /* kernel 1GB code at 0xC0000000 */
  { .a = 0x0000ffff, .b = 0x00cbfa00}, /* user   3GB code at 0x00000000 */
  { .a = 0x00000000, .b = 0x00000C00}, /* Reserved for Gates */
  { }
};

static int __unused establish_debug_call_gate(void)
{
    unsigned short __unused start_Sel = 0x0200;
    unsigned short __unused end_Sel   = 0x0280;
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;
    unsigned short __unused i = 0;

    for (Sel = start_Sel; Sel <= end_Sel; Sel += 0x10, i++) {
        desc = gdt + (Sel >> 0x3);
        desc->a = debug_descY[i].a;
        desc->b = debug_descY[i].b;
    }

    return 0;
}
device_debugcall(establish_debug_call_gate);
#endif

#ifdef CONFIG_DEBUG_SEG_GATE_CALL

#ifdef CONFIG_DEBUG_CALL_GATE_NCF
static int __unused far_called_procedure(void)
{
    unsigned short __unused CS;

    __asm__ ("mov %%cs, %0" : "=m" (CS));
    printf("CS:  %#x\n", CS);

    return 0;
}

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

    /* Call Gate utilize segment selector: 0x0283
     * CPL: 0x03
     * RPL: 0x03
     * DPL: 0x3
     * Code utilize segment selector: 0x0200
     * Segment Descriptor: 0xc0c39e000000ffff
     * CPL: --
     * RPL: --
     * DPL: 0x00
     */
    Sel  = 0x0283;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    /* DPL for CALL gate */
    desc->b |= 0x3 << 13;
    DPL  = (desc->b >> 13) & 0x3;
    /* Set up EIP for called procedure */
    desc->a |= (unsigned long)far_called_procedure & 0xFFFF;
    desc->b |= (unsigned long)far_called_procedure & 0xFFFF0000;
    /* Set up code segment selector for called procedure */
    desc->a |= 0x0200 << 16;
    /* Valid CALL Gates */
    desc->b |= 1 << 15;

    printf("Call Gate: %#x Segment Descriptor: %#08x%08x\n", Sel, 
                          (unsigned int)desc->b, (unsigned int)desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);

    /* Access code segment through Call gate. Privilege check rule for call
     * Gates:
     * CALL instruction:
     *
     *   CPL <= Call gate DPL; RPL <= Call Gate DPL
     *   Destination conforming code segment DPL <= CPL
     *   Destination nonconforming code segment DPL <= CPL
     **/
    __asm__ ("call $0x0283, %0" :: "i" (far_called_procedure));

    return 0;
}
user1_debugcall_sync(access_call_gate);
#endif

#ifdef CONFIG_DEBUG_CALL_GATE_SWTICH_STACK

static int __unused stack_called_procedure(void)
{
    unsigned short __unused CS;

    __asm__ ("mov %%cs, %0" : "=m" (CS));
    printk("CS --- %#x\n", CS);

    return 0;
}

/*
 * Switch stack through CALL Gates
 *
 */
static int __unused switch_stack_with_call_gate(void)
{
    unsigned short __unused Sel;
    unsigned short __unused CS;
    unsigned short __unused SS;
    unsigned short __unused CPL;
    unsigned short __unused RPL;
    unsigned short __unused DPL;
    unsigned int __unused EIP;
    unsigned int __unused ESP;
    struct desc_struct __unused *desc;

    __asm__ ("mov %%ss, %0" : "=m" (CS));
    CPL = CS & 0x3;

    /* Call Gate utilize segment selector: 0x0283
     * CPL: 0x03
     * RPL: 0x03
     * DPL: 0x3
     * Code utilize segment selector: 0x0200
     * Segment Descriptor: 0xc0c39e000000ffff
     * CPL: --
     * RPL: --
     * DPL: 0x00
     */
    Sel  = 0x0283;
    RPL  = Sel & 0x3;
    desc = gdt + (Sel >> 3);
    /* DPL for CALL gate */
    desc->b |= 0x3 << 13;
    DPL  = (desc->b >> 13) & 0x3;
    /* Set up EIP for called procedure */
    desc->a |= (unsigned long)stack_called_procedure & 0xFFFF;
    desc->b |= (unsigned long)stack_called_procedure & 0xFFFF0000;
    /* Set up code segment selector for called procedure */
    desc->a |= 0x0200 << 16;
    /* Valid CALL Gates */
    desc->b |= 1 << 15;

    /*
     * Obtain CS, EIP, SS, ESP value before far call.
     */
    __asm__ ("call 1f\n\r"
             "1:\n\r"
             "popl %0"
             : "=r" (EIP));
    __asm__ ("mov %%ss, %0\n\r"
             "movl %%esp, %1"
             : "=m" (SS), "=m" (ESP));
    printf("CS: %#x EIP: %#x SS: %#x ESP: %#x\n", CS, EIP, SS, ESP);

    /* Access code segment through Call gate. Privilege check rule for call
     * Gates:
     * CALL instruction:
     *
     *   CPL <= Call gate DPL; RPL <= Call Gate DPL
     *   Destination conforming code segment DPL <= CPL
     *   Destination nonconforming code segment DPL <= CPL
     **/
    __asm__ ("call $0x0283, %0" :: "i" (stack_called_procedure));

    return 0;
}
user1_debugcall_sync(switch_stack_with_call_gate);
#endif
#endif // CONFIG_DEBUG_SEG_GATE_CALL
