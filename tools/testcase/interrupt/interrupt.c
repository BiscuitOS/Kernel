/*
 * Testcase for Interrupt
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <test/testcase.h>

#include <linux/kernel.h>
#include <asm/system.h>

/*
 * get current interrupt status.
 * @return: 0 is Interrupt disable
 *          1 is Interrupt enable
 */
int get_interrupt_status(void)
{
     unsigned long _val = 0;

     __asm__ ("pushfl; popl %0": "=g" (_val));
     return (_val & 0x200) ? 1 : 0;
}

int interrupt_main(void)
{
    printk("Testcase: Interrupt descriptor table.\n");

    /* Empty interrupt */
#ifdef CONFIG_TESTCASE_IDTNULL
    trigger_interrupt_null();
#endif

    /* trigger interrupt 0 */
#ifdef CONFIG_TESTCASE_IDT0
    trigger_interrupt0();
#endif

    /* trigger interrupt 1 */
#ifdef CONFIG_TESTCASE_IDT1
    trigger_interrupt1();
#endif

    /* trigger interrupt 2 */
#ifdef CONFIG_TESTCASE_IDT2
    trigger_interrupt2();
#endif

    /* trigger interrupt 3 */
#ifdef CONFIG_TESTCASE_IDT3
    trigger_interrupt3();
#endif

    /* trigger interrupt 4 */
#ifdef CONFIG_TESTCASE_IDT4
    trigger_interrupt4();
#endif

    /* trigger interrupt 5 */
#ifdef CONFIG_TESTCASE_IDT5
    trigger_interrupt5();
#endif

    /* trigger interrupt 6 */
#ifdef CONFIG_TESTCASE_IDT6
    trigger_interrupt6();
#endif

    /* trigger interrupt 7 */
#ifdef CONFIG_TESTCASE_IDT7
    trigger_interrupt7();
#endif

    /* trigger interrupt 8 */
#ifdef CONFIG_TESTCASE_IDT8
    trigger_interrupt8();
#endif

    /* trigger interrupt 9 */
#ifdef CONFIG_TESTCASE_IDT9
    trigger_interrupt9();
#endif

    /* trigger interrupt 10 */
#ifdef CONFIG_TESTCASE_IDT10
    trigger_interrupt10();
#endif

    /* trigger interrupt 11 */
#ifdef CONFIG_TESTCASE_IDT11
    trigger_interrupt11();
#endif

    /* trigger interrupt 12 */
#ifdef CONFIG_TESTCASE_IDT12
    trigger_interrupt12();
#endif

    /* trigger interrupt 13 */
#ifdef CONFIG_TESTCASE_IDT13
    trigger_interrupt13();
#endif

    /* trigger interrupt 14 */
#ifdef CONFIG_TESTCASE_IDT14
    trigger_interrupt14();
#endif

    /* trigger interrupt 15 */
#ifdef CONFIG_TESTCASE_IDT15
    trigger_interrupt15();
#endif

    /* trigger interrupt 16 */
#ifdef CONFIG_TESTCASE_IDT16
    trigger_interrupt16();
#endif

    /* trigger interrupt 17 */
#ifdef CONFIG_TESTCASE_IDT17
    trigger_interrupt17();
#endif

    /* trigger interrupt 32 */
#ifdef CONFIG_TESTCASE_IDT32
    trigger_interrupt32();
#endif

    /* trigger interrupt 39 */
#ifdef CONFIG_TESTCASE_IDT39
    trigger_interrupt39();
#endif

    /* trigger interrupt 45 */
#ifdef CONFIG_TESTCASE_IDT45
    trigger_interrupt45();
#endif

    /* trigger interrupt 46 */
#ifdef CONFIG_TESTCASE_IDT46
    trigger_interrupt46();
#endif

    /* trigger interrupt 128 */
#ifdef CONFIG_TESTCASE_IDT128
    trigger_interrupt128();
#endif

    return 0;
}
