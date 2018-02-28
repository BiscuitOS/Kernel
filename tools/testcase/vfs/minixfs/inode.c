/*
 * MINIXFS: inode
 *
 * (C) 2018.2 BiscuitOS <buddy.zhang@aliyun.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/hdreg.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include <test/debug.h>

/*
 * Inode Entry
 *
 *   The inode contains all the important information about a file, except
 *   it name. It contains the file permissions, file type, user, group, size
 *   modification time, number of links, and the location and order of all
 *   the blocks in the file. All values are stored in low byte - high byte 
 *   order. The maximum number of links is 250 (MINIX_LINK_MAX). Notice that
 *   the group number is limited to one byte where <gnu/types.h> defines
 *   it as short.
 *
 *   Now, for the zones. The first seven zone pointers (0-6) are point to
 *   file data. They are two byte numbers which point to a BLOCK on the disk
 *   which contains the file's data. The eighth is a pointer to an indirect
 *   block. This block continues the tradition of the first seven zone 
 *   pointers, and contains 512 (BLOCK_SIZE/2) zone pointers. The ninth zone
 *   pointer in the inode points to a double indirect block. The double
 *   indirect block contains 512 pointers to more indirect blocks, each of
 *   which points to 512 data zones. Each indirect block adds 1k to the file.
 *   It's no big deal, but it;s nice that small files (under 8K) don't have
 *   this overhead. Technically you can make 262M file (7 + 512 + 512 X 512K,
 *   see also s_max_size in the superblock information), but since the Minix
 *   fs uses unsigned shorts for block pointers, it is limited to 64M 
 *   partitions.
 *
 *   To determine the meaning of the mode entry, consult <linux/types.h> which
 *   is included via <sys/types.h>. Below is a list of octal numbers which
 *   can be extracted from the mode entry. This information can also be found
 *   in the stat(2) and chmod(2) man page.
 *
 *   Scheme of Inode
 *
 *   ------------------------------------------------------------------
 *   |  Offset  |  Describe                                           |
 *   ------------------------------------------------------------------
 *   | 0x00     | MODE                                                |
 *   | 0x01     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x02     | UID                                                 |
 *   | 0x03     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x04     | SIZE                                                |
 *   | 0x05     |                                                     |
 *   | 0x06     |                                                     |
 *   | 0x07     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x08     | TIME                                                |
 *   | 0x09     |                                                     |
 *   | 0x0A     |                                                     |
 *   | 0x0B     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x0C     | GID                                                 |
 *   ------------------------------------------------------------------
 *   | 0x0D     | LINKS                                               |
 *   ------------------------------------------------------------------
 *   | 0x0E     | ZONE 0                                              |
 *   | 0x0F     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x10     | ZONE 1                                              |
 *   | 0x11     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x12     | ZONE 2                                              |
 *   | 0x13     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x14     | ZONE 3                                              |
 *   | 0x15     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x16     | ZONE 4                                              |
 *   | 0x17     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x18     | ZONE 5                                              |
 *   | 0x19     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x1A     | ZONE 6                                              |
 *   | 0x1B     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x1C     | ZONE 7                                              |
 *   | 0x1D     |                                                     |
 *   ------------------------------------------------------------------
 *   | 0x1E     | ZONE 8                                              |
 *   | 0x1F     |                                                     |
 *   ------------------------------------------------------------------
 * 
 */

/* MINIXFS ROOT */
#define MINIXFS_DEV     0x301
/* SUPER BLOCK: 2nd block */
#define SUPER_BLOCK     0x01

/* get super block */
static struct super_block *get_super_block(void)
{
    struct super_block *sb;
    struct buffer_head *bh;

    bh = bread(MINIXFS_DEV, SUPER_BLOCK);
    if (!bh) {
        printk("MINIXFS: Unable to obtain SuperBlock\n");
        return NULL;
    }
    sb = (struct super_block *)get_free_page();
    *((struct d_super_block *)sb) = *((struct d_super_block *)bh->b_data);

    brelse(bh);
    if (sb->s_magic != SUPER_MINIX_MAGIC &&
        sb->s_magic != SUPER_MINIX_MAGIC_V1) {
        printk("MINIXFS: Incorrect MAGIC\n");
        free_page((unsigned long)sb);
        return NULL;
    }
    return sb;
}

/* put super block */
static void put_super_block(struct super_block *sb)
{
    free_page((unsigned long)sb);
}

int debug_vfs_minixfs_inode_userland(void)
{
    if (1) {
        put_super_block(get_super_block());
    } else {
        put_super_block(get_super_block());
    }
    return 0;
}
