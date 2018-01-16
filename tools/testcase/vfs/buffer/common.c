/*
 * Debug for buffer machanism on VFS
 *
 * (C) 2018.1 BiscuitOS <buddy.zhang@aliyun.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

int debug_vfs_buffer(void)
{
#ifdef CONFIG_DEBUG_BUFFER_FREELIST
    debug_free_list();
#endif

    return 0;
}
