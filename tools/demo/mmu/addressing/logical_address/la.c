/*
 * LA: logical address
 *
 * (C) 2018.10.18 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <demo/debug.h>

static int la_entence(void)
{
#ifdef CONFIG_DEBUG_LA_KERNEL
    printk("HGl\n");
#elif defined (CONFIG_DEBUG_LA_USERLAND)
    printf("Hello World\n");
#endif
    return 0;
}

#ifdef CONFIG_DEBUG_LA_KERNEL
device_debugcall(la_entence);
#elif defined (CONFIG_DEBUG_LA_USERLAND)
user1_debugcall_sync(la_entence);
#endif
