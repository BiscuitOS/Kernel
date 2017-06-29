/*
 * Test schedule sub-system
 * Maintainer: Buddy <buddy.zhang@aliyun.com>
 *
 * Copyright (C) 2017 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/sched.h>
#include <linux/kernel.h>

#ifdef CONFIG_TESTCODE

/*
 * Get tss and ldt entry
 */
void test_tss_ldt_entry(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11
    /*
     * Each task contains a TSS and LDT.
     * The selector of first task is 4 in GDT. So
     * NULL | CS | DS | SYSCALL | TSS0 | LDT0 | TSS1 | LDT1 | TSS2 | LDT2
     * The length of each describe is 8 bytes. So 
     * TSSn = (n) << 4 + FIRST_TSS_ENTRY << 3
     * LDTn = (n) << 4 + FIRST_LDT_ENTRY << 3
     */
    int i;

    for (i = 0; i < 3; i++)
        printk("Task[%d] TSS %#x LDT %#x\n", i,
                     _TSS(i), _LDT(i));	

#endif
}

/*
 * Empty segment descriptor
 *  1. GDT[0] is empty segment descriptor. 
 *  2. If DS, ES, GS, FS loads it, The system will not dump any kernel 
 *     panic, but if system access a segment via these segment register, 
 *     the system will generate a kernel error.
 *  3. If SS and CS load empty segment descriptor, the system will generate
 *     kernel panic.
 */
void test_empty_segment_descriptor(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11

#define TESTCASE0     1
#define TESTCASE1     0
#define TESTCASE2     0
#define TESTCASE3     0

#if TESTCASE0
    /*
     * Loading empty segment descriptor to ES, GS, DS, FS.
     * If system only loads empty segment descriptor on above segment register,
     * Any error doesn't generate. 
     */
     __asm__("pushl %%eax\n\t"
             "push %%ds\n\t"
             "push %%es\n\t"
             "push %%gs\n\t"
             "push %%fs\n\t"
             "movl $0, %%eax\n\t"
             "mov %%ax, %%ds\n\t"
             "mov %%ax, %%es\n\t"
             "mov %%ax, %%gs\n\t"
             "mov %%ax, %%fs\n\t"
             "pop %%fs\n\t"
             "pop %%gs\n\t"
             "pop %%es\n\t"
             "pop %%ds\n\t"
             "popl %%eax" :);
    printk("Loading Empty segment descriptor into ES,DS,GS,FS\n");
#elif TESTCASE1
    /*
     * Loading empty segment descriptor to CS.
     * This operation will generate a kernel panic.
     */
     __asm__("pushl %%eax\n\t"
             "push %%cs\n\t"
             "movl $0, %%eax\n\t"
             "mov %%ax, %%cs" :);
     printk("Loading Empty segment descriptor into CS\n");
#elif TESTCASE2
    /*
     * Loading empty segment descriptor to SS.
     * This operation will generates a kernel painc.
     */
    __asm__("pushl %%eax\n\t"
            "push %%ss\n\t"
            "movl $0, %%eax\n\t"
            "mov %%ax, %%ss\n\t"
            "pop %%ss\n\t"
            "popl %%eax" :);
    printk("Loading Empty segment descriptor into SS\n");
#elif TESTCASE3
    /*
     * Loading empty segment descriptor to DS,ES,GS,FS.
     * And then access segment via those segment register.
     * This operation will generate a kernel panic.
     */ 
    __asm__("pushl %%eax\n\t"
            "push %%es\n\t"
            "push %%ds\n\t"
            "push %%gs\n\t"
            "push %%fs\n\t"
            "movl $0, %%eax\n\t"
            "mov %%ax, %%es\n\t"
            "mov %%ax, %%ds\n\t"
            "mov %%ax, %%fs\n\t"
            "mov %%ax, %%gs\n\t" :);
    printk("Loading Empty segment descriptor and access ES,GS,DS,FS\n");
#endif
#undef TESTCASE0     
#undef TESTCASE1     
#undef TESTCASE2     
#undef TESTCASE3     

#endif
}

/*
 * Segment type check
 */
void test_segment_type(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11

#define TESTCASE0   0

#if TESTCASE0
    /*
     * Case0:
     * When loading a segment selector into CS register, system will check
     * segment type. And CS only loading selector of CODE segment.
     * So, we loading a another segment into CS.
     */
    __asm__("pushl %%eax\n\t"
            "push %%ds\n\t"
            "movl $0x10, %%eax\n\t"
            "mov %%ax, %%ds\n\t"
            "push %%ds\n\t"
            "pushl %%eax" :);   
#endif


 
#endif
}

#endif
