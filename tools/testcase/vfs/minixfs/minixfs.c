/*
 * MINIX file system
 *
 * (C) 2018.2 BiscuitOS <buddy.zhang@aliyun.com>
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
 * MINIXFS physical layout
 *
 * ------------------------------------------------------------------
 * | Boot | Super | Inode | Zone | Data                             |
 * ------------------------------------------------------------------
 *
 *   Boot:  Boot block
 *          Reserved for partition boot code. 1 block
 *   Super: Super block
 *          Infomation about the filesystem. 1 block
 *   Inode: Inode Map
 *          Keep track of used/unused inodes. #Inodes/BLOCK_SIZE
 *   Zone:  Zone Map
 *          Keep track of used/unused inodes. #Data Zones/BLOCK_SIZE
 *   Data:  Data Zone
 *          File/Directory contents.
 *         
 * Definitions
 *
 *   inode: Stores all the information about a file except its name.
 *   block: A unit of size which is determined by the medium or programmer.
 *          For example, most devices use 1024 byte (1k) blocks, including
 *          hard disks and floppy disks (stored in BLOCK_SIZE).
 *   zone:  A zone is the part of the disk where the file data exists.
 *   super block: 
 *          the first block on a disk which contains information about
 *          the type and size of the file system.
 */

/* MINIXFS ROOT DEV: 1st hard-disk */
#define ROOT_DEV 0x300
/* BOOT block: first block on MINIXFS */
#define BOOT_BLOCK     0x00

/*
 * Obtain boot block of MINIXFS
 *
 *   Assume/Indicate MINIXFS as first hard disk. 
 */
static void obtain_boot_block_minixfs(void)
{
    struct buffer_head *bh;
    struct partition *partition[4];

    bh = bread(ROOT_DEV, BOOT_BLOCK);
    if (!bh) {
        printk("MINIXFS: can't obtain boot block.\n");
        return;
    }
    /* varify whether block is boot block */
    if (bh->b_data[0x1FE] != 0x55 || (unsigned char)
        bh->b_data[0x1FF] != 0xAA) {
        printk("MINIXFS: Unable to obtain correct Boot block.\n");
        brelse(bh);
        return;
    }

    /* Obtain Partition Table 0. */
    partition[0] = (struct partition *)&bh->b_data[0x1BE];
    /* Obtain Partition Table 1. */
    partition[1] = (struct partition *)&bh->b_data[0x1CE];
    /* Obtain Partition Table 2. */
    partition[2] = (struct partition *)&bh->b_data[0x1DE];
    /* Obtain Partition Table 3. */
    partition[3] = (struct partition *)&bh->b_data[0x1EE];
    
    brelse(bh);
}


int debug_vfs_minixfs(void)
{
    if (1) {
        obtain_boot_block_minixfs();
    } else {
        obtain_boot_block_minixfs();
    }     
    return 0;
}
