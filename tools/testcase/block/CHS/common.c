/*
 * Disk: Sector, Cyliner, Head, Track
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
int debug_block_CHS_common(void)
{
    debug_block_disk_CHS_common();
    return 0;
}
