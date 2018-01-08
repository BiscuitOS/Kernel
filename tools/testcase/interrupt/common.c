/*
 * Testcase for Interrupt
 *
 * (C) 2018.1 BiscuitOS <buddy.zhang@aliyun.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_interrupt_common(void)
{
#ifdef CONFIG_DEBUG_INT_USAGE
    interrupt_useage_common();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT0
    common_interrupt0();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT1
    common_interrupt1();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT2
    common_interrupt2();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT3
    common_interrupt3();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT4
    common_interrupt4();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT5
    common_interrupt5();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT6
    common_interrupt6();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT7
    common_interrupt7();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT8
    common_interrupt8();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT9
    common_interrupt9();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT10
    common_interrupt10();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT11
    common_interrupt11();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT12
    common_interrupt12();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT13
    common_interrupt13();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT14
    common_interrupt14();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT16
    common_interrupt16();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT32
    common_interrupt32();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT39
    common_interrupt39();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT45
    common_interrupt45();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT46
    common_interrupt46();
#endif

#ifdef CONFIG_DEBUG_INTERRUPT128
    common_interrupt128();
#endif

    return 0;
}
