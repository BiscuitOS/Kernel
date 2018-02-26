/*
 * Block device: Hard-disk, Floppy and Ramdisk
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <test/block.h>

/* common block entry */
int debug_block_common(void)
{
#ifdef CONFIG_DEBUG_BLOCK_HD
    debug_block_hd_common();
#endif

#ifdef CONFIG_DEBUG_BLOCK_DISK
    debug_block_disk_common();
#endif
    return 0;
}
