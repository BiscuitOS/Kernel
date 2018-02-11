/*
 * Common HD usage
 *
 * (C) 2018.02 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hdreg.h>

#include <test/debug.h>

/*
 * Read Hard-Disk partition Table
 */
static void Obtain_HD0_partition_table(void)
{
    /* First HD is 0x300 */
    int dev = 0x300, i;
    struct buffer_head *bh;
    struct partition *p;

    bh = HD_bread(dev, 0);
    if (bh->b_data[510] != 0x55 || (unsigned char)
        bh->b_data[511] != 0xAA) {
        printk("Bad partition table on drive %d\n", dev);
        panic("");
    }

    p = 0x1BE + (void *)bh->b_data;
    for (i = 1; i < 5; i++, p++) {
        hd2[i].start_sect = p->start_sect;
        hd2[i].nr_sects   = p->nr_sects;
    }
    brelse(bh);
}


int debug_block_usage_common(void)
{
    if (1) {
        Obtain_HD0_partition_table();
    } else {
        Obtain_HD0_partition_table();
    }
    return 0;
}
