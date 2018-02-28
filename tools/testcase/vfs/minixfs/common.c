/*
 * MINIX file system
 *
 * (C) 2018.1 BiscuitOS <buddy.zhang@aliyun.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_vfs_minixfs_common_userland(void)
{
#ifdef CONFIG_DEBUG_MINIXFS_BLOCK
    debug_vfs_minixfs_userland();
#endif

    return 0;
}
