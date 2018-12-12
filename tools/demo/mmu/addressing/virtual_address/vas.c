/*
 * Virtual address
 *
 * (C) 2018.11.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

#ifdef CONFIG_DEBUG_VA_CODE
static int debug_kernel_code(void)
{
    extern char etext[];

    printk(".text: %#08x -- %#08x\n", 0, (unsigned int)etext); 

    return 0;
}
late_debugcall(debug_kernel_code);
#endif
