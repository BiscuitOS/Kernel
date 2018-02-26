/*
 * Disk: Sector, Cyliner, Head, Track, Partition
 *
 * (C) 2018.02 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <test/debug.h>

/* common disk entry */
int debug_block_disk_common(void)
{
#ifdef CONFIG_DEBUG_PARTITION_MBR
    debug_block_disk_MBR_common();    
#endif

#ifdef CONFIG_DEBUG_DISK_CHS
    debug_block_disk_CHS_common();
#endif

    return 0;
}
