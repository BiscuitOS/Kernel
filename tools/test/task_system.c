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

#endif
