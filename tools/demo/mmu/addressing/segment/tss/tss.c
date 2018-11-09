/*
 * TSS: Task-State Segment
 *
 * (C) 2018.11.7 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/unistd.h>

#include <demo/debug.h>

#ifdef CONFIG_DEBUG_SEG_TSS_DESC
static int __unused tss_descriptor_entence(void)
{
    unsigned short __unused TR;
    unsigned short __unused DPL;
    unsigned int __unused base;
    unsigned int __unused limit;
    struct desc_struct __unused *desc;

    /*
     * Task Register (TR)
     *
     * The task register holds the 16-bit segment selector, base address (32
     * bits in protected mode), segment limit, and descriptor attribute for
     * the TSS of the current task. The selector references the TSS descriptor
     * in the GDT. The base address specifies the linear address of byte 0 of
     * the TSS; the segment limit specifies the number of bytes in the TSS.
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
     * the task register, respectively. When the LTR instruction loads a
     * segment selector in the task register, the base address, limit, and
     * descriptor attribute from the TSS descriptor are automatically loaded
     * into the task register. On power up or reset of the processor, the base
     * address is set to the default value of 0 and the limit is set to 0xFFFFH.
     *
     * When a task switch occurs, the task register is automatically loaded
     * with the segment selector and descriptor for the TSS for the new task.
     * The  contents of the task register are not automatically saved prior to
     * writing the new TSS information into the register.
     *
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

    /* Load TSS from gdt */
    desc = gdt + (TR >> 3);

#ifdef CONFIG_DEBUG_TSS_DESC_BASE
    /*
     * Base address field
     *
     * Defines the location of byte 0 of the segment within the 4-GByte linear
     * address space. The processor puts together the three base address fields
     * to from a single 32-bit value. Segment base addresses should be aligned
     * to 16-byte boundaries. Although 16-byte alignment is not required, this
     * alignment allows programs to maximize performance by aligning code and 
     * data on 16-byte boundaries.
     */
    base = get_base(*desc);
    printk("TSS base addrss: %#x\n", base);
#endif

#ifdef CONFIG_DEBUG_TSS_DESC_LIMIT
    /*
     * Segment limit field
     *
     * Specifies the size of the segment. The processor puts together the 
     * two segment limit fields to form a 20-bit value. The processor 
     * interprets the segment limit in one of two ways, depending on the 
     * setting of the G (granularity) flag:
     *
     * * If the granularity flag is clear, the segment size can range from 
     *   1 byte to 1 Mbyte, in byte increments.
     *
     * * If the granularity flag is set, the segment size can range from 
     *   4KBytes to 4GBytes, in 4-KByte increments.
     */
    limit = get_limit(TR);
    printk("TSS limit: %#x\n", limit);
#endif

#ifdef CONFIG_DEBUG_TSS_DESC_B
    /*
     * B: Busy
     *
     * The busy flag (B) in the type field indicates whether the task is busy.
     * A busy task is currently running or suspended. A type field with a value
     * of 1001B indicates an inactive task; a value of 1011B indicates a busy
     * task. Tasks are not recursive. The processor uses the busy flag to
     * detect an attempt to call a task whose execution has been interrupted.
     * To insure that there is only one busy flag is associated with a task,
     * each TSS should have only one TSS descriptor that points to it.
     */
    if ((desc->b >> 9) & 0x1)
        printk("Task busy: running or suspended\n");
    else
        printk("Task inactive\n");
#endif

#ifdef CONFIG_DEBUG_TSS_DESC_DPL
    /*
     * DPL (Descriptor privilege level) field
     *
     * Specifies the privilege level of the segment. The privilege level can 
     * range from 0 to 3, with 0 being the most privileged level. The DPL is
     * used to control access to the segment.
     */
    DPL = (desc->b >> 13) & 0x3;
    printk("DPL: %#x\n", DPL);
#endif

#ifdef CONFIG_DEBUG_TSS_DESC_P
    /*
     * P (Segment-present) flag
     *
     * Indicates whether the segment is present in memory (set) or not present
     * (clear). If this flag is clear, the processor generates a segment-not-
     * present exception (#NP) when a segment selector that points to the 
     * segment descriptor is loaded into a segment register. Memory management
     * software can use this flag to control which segments are actually loaded
     * into physical memory at a given time. It offers a control in addition to
     * paging for managing virtual memory.
     */
    if (desc->b & 0x8000)
        printk("Segment-Present flag set.\n");
    else
        printk("Segment-Present flag clear.\n");
#endif

#ifdef CONFIG_DEBUG_TSS_DESC_G
    /*
     * G (granularity) flag
     *
     * Determines the scalling of the segment limit field. When the granularity
     * flag is clear, the segment limit is interpreted in byte units; when flag
     * is set, the segment limit is interpreted in 4-KByte units. (This flag 
     * does not affect the granularity of the base address; it is always byte 
     * granlar.) When the granularity flag is set, the twelve least significant
     * bits of an offset are not tested when checking the offset against the
     * segment limit. For example, when the granularity flag is set, a limit of
     * 0 results in valid offsets from 0 to 4095.
     */
    if (desc->b & 0x800000)
        printk("Limit granularity 4-KBytes\n");
    else
        printk("Limit granularity 1-Byte\n");
#endif

    return 0;
}
late_debugcall(tss_descriptor_entence);
#endif

#ifdef CONFIG_DEBUG_SEG_TSS_TR
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
static int __unused TR_entence(void)
{
    unsigned short __unused TR;

#ifdef CONFIG_DEBUG_TSS_TR_STR
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

#ifdef CONFIG_DEBUG_TSS_TR_LTR
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
late_debugcall(TR_entence);
#endif

#ifdef CONFIG_DEBUG_SEG_TSS_CONTEXT
static int __unused tss_context(void)
{
    struct tss_struct __unused *tss;

    /* Load current TSS */
    tss = &current->tss;

    printf("eax: %#x ebx: %#x ecx: %#x edx: %#x esp: %#x ebp: %#x\n"
           "esi: %#x edi: %#x CS: %#x DS: %#x SS: %#x FS: %#x GS: %#x\n",
           (unsigned int)tss->eax, (unsigned int)tss->ebx, 
           (unsigned int)tss->ecx, (unsigned int)tss->edx, 
           (unsigned int)tss->esp, (unsigned int)tss->ebp,
           (unsigned int)tss->esi, (unsigned int)tss->edi, 
           tss->cs, tss->ds, tss->ss, tss->fs, tss->gs);
    return 0;
}
user1_debugcall_sync(tss_context);
#endif

#ifdef CONFIG_DEBUG_TASK_GATE_ESTABLISH
/*
 * Establish a Task Gate.
 *
 * Debug segment selector and segment descriptor list
 *
 * +-------------+--------------------+----------+-----+
 * | Segment Sel | Segment Descriptor | TSS Sel  | DPL |
 * +-------------+--------------------+----------+-----+
 * | 0x0203      | 0x0000e50000000000 | _TSS(0)  | 03  |
 * +-------------+--------------------+----------+-----+
 */

static int __unused establish_debug_task_gate(void)
{
    unsigned short __unused Sel = 0x0203;
    struct desc_struct __unused *desc;

    desc = gdt + (Sel >> 0x3);
    desc->a = _TSS(0) << 16;
    desc->b = 0x0000e500;

    return 0;
}
device_debugcall(establish_debug_task_gate);
#endif

#ifdef CONFIG_DEBUG_TASK_GATE_U2K

static int __unused user_access_kernel(void)
{
    unsigned short __unused CS;
    unsigned short __unused Sel;
    unsigned short __unused CPL;
    unsigned short __unused RPL;
    unsigned short __unused DPL;
    struct desc_struct __unused *desc;

    /* Obtain current CPL */
    __asm__ ("mov %%cs, %0" : "=m" (CS));
    CPL = CS & 0x3;

    /* Prepare segment selector for TSS Gates, we have establish a TSS Gates,
     * and segment selector is 0x0203. 
     */
    Sel = 0x0203;
    /* Obtain the RPL of TSS Gates */
    RPL = Sel & 0x3;

    /* Obtian TSS Gate */
    desc = gdt + (Sel >> 3);
    /* Obtain the DPL of TSS Gate */
    DPL = (desc->b >> 13) & 0x3;

    printf("Sel: %#x TSS Gates: %#08x%08x\n", Sel, desc->b, desc->a);
    printf("CPL: %#x RPL: %#x DPL: %#x\n", CPL, RPL, DPL);

    __asm__ ("ljmp *%0" :: "m" (Sel));
    printk("Hello World\n");

    return 0;
}
user1_debugcall_sync(user_access_kernel);
#endif
