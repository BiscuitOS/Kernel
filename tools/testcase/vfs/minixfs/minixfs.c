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
#include <linux/mm.h>
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
#define ROOT_DEV       0x300
/* MINIXFS SUPER DEV: 1st partition on disk */
#define SUPER_DEV      0x301
/* BOOT block: first block on MINIXFS */
#define BOOT_BLOCK     0x00
/* SUPER block: second block on MINIXFS */
#define SUPER_BLOCK    0x01
/* IMAP block: Inode Map block */
#define IMAP_BLOCK     0x02

/*
 * Obtain boot block of MINIXFS
 *
 *   Assume/Indicate MINIXFS as first hard disk. 
 *   Schem of Partition e.g.
 *
 *   .-------------------------------------------------------------------.
 *   | 0x1BE          | Bootable Identificiation:                        |
 *   | (Offset 0x00)  | 00: Un-bootable  -- 0x80: Bootable               |
 *   ---------------------------------------------------------------------
 *   | 0x1BF - 0x1C1  | 1st partition Original partition                 |
 *   | (Offset 0x01)  | Little-endian                                    |
 *   |                | 0x1BF[7:0]: Original Head                        |
 *   |                | 0x1C0[5:0]: Original Sector                      |
 *   |                | 0x1C0[7:6]: MSB of Original Cylinder             |
 *   |                | 0x1C1[7:0]: LSB of Original Cylinder             |
 *   ---------------------------------------------------------------------
 *   | 0x1C2          | Partition Type                                   |
 *   | (Offset 0x4)   | 00H: Un-used                                     |
 *   |                | 06H: FAT16 Basic partition                       |
 *   |                | 0BH: FAT32 Basic partition                       |
 *   |                | 05H: Extend partition                            |
 *   |                | 07H: NTFS partition                              |
 *   |                | 0FH: (LBA mode) Extern partition                 |
 *   ---------------------------------------------------------------------
 *   | 0x1C3 - 0x1C5  | 1st partition End CHS                            |
 *   | (Offset 0x5)   | Little-endian                                    |
 *   |                | 0x1C3[7:0]: End Head                             |
 *   |                | 0x1C4[5:0]: End Sector                           |
 *   |                | 0x1C4[7:6]: MSB of End Cylinder                  |
 *   |                | 0x1C5[7:0]: LSB of End Cylinder                  |
 *   ---------------------------------------------------------------------
 *   | 0x1C6 - 0x1C9  | Used Sector for 1st partition                    |
 *   | (Offset 0x8)   |                                                  |
 *   ---------------------------------------------------------------------
 *   | 0x1CA - 0x1CD  | Total Sector for 1st partition                   |
 *   | (Offset 0xC)   |                                                  |
 *   .-------------------------------------------------------------------.
 */
static void obtain_boot_block_minixfs(void)
{
    struct buffer_head *bh;
    struct partition *partition[4], *p;

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
    
    p = partition[0];
    printk("===== Partition Table =====\n");
    if (p->boot_ind)
        printk("Partition is Bootable\n");
    else
        printk("Partition is Un-Bootable\n");
    printk("Partition type:    %#x\n", p->sys_ind);
    printk("Original Head:     %#x\n", p->head);
    printk("Original Cylinder: %#x\n",
          ((p->sector & 0xC0) << 0x2) + p->cyl);
    printk("Original Sector:   %#x\n", p->sector & 0x3F);
    printk("End Head:          %#x\n", p->end_head);
    printk("End Cylinder:      %#x\n",
          ((p->end_sector & 0xC0) << 0x2) + p->end_cyl );
    printk("End Sector:        %#x\n", (p->end_sector) & 0x3F);
    printk("Used sectors:      %#x\n", p->end_sector);
    printk("Total sector:      %#x\n", p->nr_sects);
    printk("==========================\n");    

    brelse(bh);
}

/*
 * Obtain Super block for MINIXFS
 *
 *   The superblock for the minix fs is offset 1024 bytes from the start of
 *   of the disk, this is to leave room for things like LILO and other boot
 *   code. The superblock basically details the size of the the file system.
 *   It contains the number of inodes, number of data zones, space used by
 *   the inode and zone map. The blocksize of the filesystem and a two
 *   character ID number indicating that it is indeed a MINIXFS.
 *
 * Scheme of Super block
 *   Assume Super block is located in 1st partition on HD (that 0x301).
 *
 *   ------------------------------------------------------------------
 *   | Offset | Describe                                              |
 *   ------------------------------------------------------------------
 *   | 0x00   | Number of inodes                                      |
 *   | 0x01   |                                                       |
 *   ------------------------------------------------------------------
 *   | 0x02   | Number of data zones                                  |
 *   | 0x03   |                                                       |
 *   ------------------------------------------------------------------
 *   | 0x04   | Space used by inode map (block)                       |
 *   | 0x05   |                                                       |
 *   ------------------------------------------------------------------
 *   | 0x06   | Space used by zone map (block)                        |
 *   | 0x07   |                                                       |
 *   ------------------------------------------------------------------
 *   | 0x08   | First zone with "file" data                           |
 *   | 0x09   |                                                       |
 *   ------------------------------------------------------------------
 *   | 0x0A   | Size of data zone =                                   |
 *   | 0x0B   |                 (1024 << s_log_zone_size)             |
 *   ------------------------------------------------------------------
 *   | 0x0C   | Maximum file size (bytes)                             |
 *   | 0x0D   |                                                       |
 *   | 0x0E   |                                                       |
 *   | 0x0F   |                                                       |
 *   ------------------------------------------------------------------
 *   | 0x10   | Minix 14/30 ID number                                 |
 *   | 0x11   |                                                       |
 *   ------------------------------------------------------------------
 *   | 0x12   | Mount state, was it cleanly unmount                   |
 *   |        |                                                       |
 *   ------------------------------------------------------------------
 */
static void obtain_superblock_minixfs(void)
{
    struct buffer_head *bh;
    struct super_block *sb;

    /* The superblock is located in 2nd block on MINIXFS */
    bh = bread(SUPER_DEV, SUPER_BLOCK);
    if (!bh) {
        printk("MINIXFS: unable to obtain superblock.\n");
        return;
    }

    /* Allocate memory to super block structure. */
    sb = (struct super_block *)get_free_page();
    /* Obtain super block structure */
    *((struct d_super_block *)sb) = *((struct d_super_block *)bh->b_data);
    brelse(bh);
    /* Check MINIXFS MAGIC */
    if (sb->s_magic != SUPER_MINIX_MAGIC &&
        sb->s_magic != SUPER_MINIX_MAGIC_V1) {
        printk("MINIXFS: Incorrect MAGIC!\n");
        brelse(bh);
        return;
    }
    printk("=============super block==============\n");
    printk("Ninodes:     %#x\n", sb->s_ninodes);
    printk("Nzones:      %#x\n", sb->s_nzones);
    printk("Imap:        %#x\n", sb->s_imap_blocks);
    printk("Zmap:        %#x\n", sb->s_zmap_blocks);
    printk("Firstzone:   %#x\n", sb->s_firstdatazone);
    printk("Zone Size:   %#x\n", sb->s_log_zone_size);
    printk("MAX Size:    %#x\n", sb->s_max_size);
    printk("MAGIC:       %#x\n", sb->s_magic);
    printk("======================================\n");
    free_page((unsigned long)sb);
}

/*
 * Inode Map
 *
 *   Inode Bitmap describe the usage of inode which indicates special inode
 *   whether is used. Each bit represent one inode. For one block that 
 *   contains 1024 Byte (8192 bits), A block will describe useage of 8192
 *   inodes. The same as logical block Bitmap, beause the find function will
 *   return 0 when empty inode is be found, the first bit of the first byte
 *   for Inode Bitmap and corresponding inode is not used, and it will be set
 *   when system establish. So, the first Inode Bitmap block only contains
 *   the state of 8191 inodes.
 */
static void parse_inode_map(void)
{
    struct super_block *sb;
    struct buffer_head *bh;
    int i, j;
    char *map;

    /* Obtain supber block */
    sb = (struct super_block *)get_free_page();
    bh = bread(SUPER_DEV, SUPER_BLOCK);
    if (!bh) {
        printk("MINIXFS: unable to obtain Superblock\n");
        free_page((unsigned long)sb);
        return;
    }
    *((struct d_super_block *)sb) = *((struct d_super_block *)bh->b_data);
    brelse(bh);

    /* clear s_imap */
    for (i = 0; i < I_MAP_SLOTS; i++)
        sb->s_imap[i] = NULL;
    
    /* Obtain Imap from bread */
    for (i = 0; i < sb->s_imap_blocks; i++)
        sb->s_imap[i] = bread(SUPER_DEV, IMAP_BLOCK + i);

    /* Dump first inode bit map */
    map = (char *)sb->s_imap[0]->b_data;
    /* Dump all inode bit map */
    printk("===============Inode BitMap==============\n");
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 8; j++)
            printk("%#2x ", (unsigned char)map[i * 10 + j]);
        printk("\n");
    }
    printk("=========================================\n");
    free_page((unsigned long)sb);
}

/*
 * Zone map
 *
 */
static void parse_zone_map(void)
{
    struct super_block *sb;
    struct buffer_head *bh;

    /* Obtain super block */
    bh = bread(SUPER_DEV, SUPER_BLOCK);
    if (!bh) {
        printk("MINIXFS: Unable to obtain Superblock.\n");
    }
}

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
 */

int debug_vfs_minixfs_userland(void)
{
    if (1) {
        parse_inode_map();
    } else {
        parse_inode_map();
        obtain_superblock_minixfs();
        obtain_boot_block_minixfs();
    }     
    return 0;
}
