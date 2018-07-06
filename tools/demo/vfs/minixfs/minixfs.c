/*
 * MINIX Filesystem.
 *
 * (C) 2018.07.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/stat.h>
#include <linux/locks.h>
#include <linux/minix_fs.h>

#include <asm/bitops.h>

#include <demo/debug.h>

/*
 * Basic Definitions on MINIXFS
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

static inline _syscall3(int, open, const char *, file, int, flag, int, mode);
static inline _syscall1(int, close, int, fd);

/* Boot block */
#define BOOT_BLOCK          0x00
/* Super block */
#define SUPER_BLOCK         0x01

/* Bit number for per BLOCK */
#define BIT_PER_BLOCK       (1024 * 8)

#define find_first_zero(addr) ({ \
    int __res; \
    __asm__("cld\n" \
            "1:\tlodsl\n\t" \
            "notl %%eax\n\t" \
            "bsfl %%eax,%%edx\n\t" \
            "jne 2f\n\t" \
            "addl $32,%%ecx\n\t" \
            "cmpl $8192,%%ecx\n\t" \
            "jl 1b\n\t" \
            "xorl %%edx,%%edx\n" \
            "2:\taddl %%edx,%%ecx" \
            : "=c" (__res) : "0" (0), "S" (addr)); \
__res; })

/*
 * Boot block
 *  Boot block is first part on MINIX Filesystem. It consists of 
 *  boot code, Reserved for system. The Boot block contain 1 block.
 *
 *   Assume/Indicate MINIXFS as first hard disk. 
 *   Schem of Partition e.g.
 *
 *   +----------------+--------------------------------------------------+
 *   | 0x1BE          | Bootable Identificiation:                        |
 *   | (Offset 0x00)  | 00: Un-bootable  -- 0x80: Bootable               |
 *   +----------------+--------------------------------------------------+
 *   | 0x1BF - 0x1C1  | 1st partition Original partition                 |
 *   | (Offset 0x01)  | Little-endian                                    |
 *   |                | 0x1BF[7:0]: Original Head                        |
 *   |                | 0x1C0[5:0]: Original Sector                      |
 *   |                | 0x1C0[7:6]: MSB of Original Cylinder             |
 *   |                | 0x1C1[7:0]: LSB of Original Cylinder             |
 *   +----------------+--------------------------------------------------+
 *   | 0x1C2          | Partition Type                                   |
 *   | (Offset 0x4)   | 00H: Un-used                                     |
 *   |                | 06H: FAT16 Basic partition                       |
 *   |                | 0BH: FAT32 Basic partition                       |
 *   |                | 05H: Extend partition                            |
 *   |                | 07H: NTFS partition                              |
 *   |                | 0FH: (LBA mode) Extern partition                 |
 *   +----------------+--------------------------------------------------+
 *   | 0x1C3 - 0x1C5  | 1st partition End CHS                            |
 *   | (Offset 0x5)   | Little-endian                                    |
 *   |                | 0x1C3[7:0]: End Head                             |
 *   |                | 0x1C4[5:0]: End Sector                           |
 *   |                | 0x1C4[7:6]: MSB of End Cylinder                  |
 *   |                | 0x1C5[7:0]: LSB of End Cylinder                  |
 *   +----------------+--------------------------------------------------+
 *   | 0x1C6 - 0x1C9  | Used Sector for 1st partition                    |
 *   | (Offset 0x8)   |                                                  |
 *   +----------------+--------------------------------------------------+
 *   | 0x1CA - 0x1CD  | Total Sector for 1st partition                   |
 *   | (Offset 0xC)   |                                                  |
 *   +----------------+--------------------------------------------------+
 */
static __unused int minixfs_boot_block(struct super_block *sb)
{
    struct buffer_head *bh;
    char *buf;
    int i;

    /* Read Boot block from Disk */
    if (!(bh = bread(sb->s_dev, BOOT_BLOCK, BLOCK_SIZE))) {
        printk(KERN_ERR "Unable to read BOOT block.\n");
        return -EINVAL;
    }

    /* Read boot code */
    buf = (char *)bh->b_data;

    printk("Boot Code:\n");
    for (i = 510; i < BLOCK_SIZE; i++)
        printk("%x ", buf[i]);
    printk("\n");

    brelse(bh);

    return 0;
}

/*
 * Super block
 * 
 *   The superblock for the minix-fs is offset 1024 bytes from the start of
 *   of the disk, this is to leave room for things like LILO and other boot
 *   code. The superblock basically details the size of the the file system.
 *   It contains the number of inodes, number of data zones, space used by
 *   the inode and zone map. The blocksize of the filesystem and a two
 *   character ID number indicating that it is indeed a MINIXFS. The linux
 *   utilize two structure to manage minix-fs superblock. The 'minix_sb'
 *   structure manage meta superblock information on Disk. And the 
 *   'minix_sb_info' structure manages meta superblock information on RAM.
 *
 *     super block <-----> minix_sb_info <-----> minix_sb
 *         RAM                 RAM                 Disk
 *
 *   Superblock on Disk:
 *
 *   +------+------------+--------------+-------------+-----------------+
 *   |      |            |              |             |                 |
 *   | Boot | superblock | inode-bitmap | zone-bitmap | ...             |
 *   |      |            |              |             |                 |
 *   +------+------------+--------------+-------------+-----------------+
 *          A
 *          |
 *          |
 *          |
 *          V
 *   struct minix_sb
 * 
 *                                                     Superblock on RAM   
 *                                                     +----------------+
 *                                                     |                |
 *                                                     |   SuperBlock   |
 *                                                     |                |
 *                                                     +----------------+
 *                                                     |                |
 *                       struct minix_sb_info<-------->|   u.minix_sb   |
 *                                                     |                |
 *                                                     +----------------+
 *
 *
 *
 *
 *
 * Scheme of Super block
 *   Assume Super block is located in 1st partition on HD (that 0x301).
 *
 *   struct minix_super_block:
 *    It indicate the super block for MINIX-FS and store in Disk.
 *
 *   +--------+-------------------------------------------------------+
 *   | Offset | Describe                                              |
 *   +--------+-------------------------------------------------------+
 *   | 0x00   | s_ninodes: Number of inodes (block)                   |
 *   | 0x01   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x02   | s_nzones: Number of data zones (block)                |
 *   | 0x03   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x04   | s_imap_blocks: Number of Inode BitMap (block)         |
 *   | 0x05   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x06   | s_zmap_blocks: Number of Zone BitMap (block)          |
 *   | 0x07   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x08   | s_firstdatazone: The number of first zone with 'data' |
 *   | 0x09   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x0A   | s_log_zone_size:                                      |
 *   |   |    |    Size of data zone =                                |
 *   | 0x0B   |                 (1024 << s_log_zone_size)             |
 *   +--------+-------------------------------------------------------+
 *   | 0x0C   | s_max_size:                                           |
 *   | 0x0D   |   Maximum file size (bytes)                           |
 *   | 0x0E   |                                                       |
 *   | 0x0F   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x10   | s_magic:                                              |
 *   | 0x11   |   The Magic of MINIX-FS                               |
 *   +--------+-------------------------------------------------------+
 *   | 0x12   | Mount state, was it cleanly unmount                   |
 *   | 0x13   |                                                       |
 *   +--------+-------------------------------------------------------+
 * 
 *
 *   struct minix_sb_info:
 *    It indicate the minix-fs basic information that store in RAM.
 *
 *   +--------+-------------------------------------------------------+
 *   | Offset | Describe                                              |
 *   +--------+-------------------------------------------------------+
 *   | 0x00   | s_ninodes: Number of inodes (block)                   |
 *   | 0x03   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x04   | s_nzones: Number of data zones (block)                |
 *   | 0x07   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x08   | s_imap_blocks: Number of Inode BitMap (block)         |
 *   | 0x0B   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x0C   | s_zmap_blocks: Number of Zone BitMap (block)          |
 *   | 0x0F   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x10   | s_firstdatazone: The number of first zone with 'data' |
 *   | 0x13   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x14   | s_log_zone_size:                                      |
 *   |   |    |    Size of data zone =                                |
 *   | 0x17   |                 (1024 << s_log_zone_size)             |
 *   +--------+-------------------------------------------------------+
 *   | 0x18   | s_max_size:                                           |
 *   | 0x19   |   Maximum file size (bytes)                           |
 *   | 0x1A   |                                                       |
 *   | 0x1B   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x1C   | s_imap[8]:                                            |
 *   | 0x1D   |   Points to buffer for Inode Bitmap                   |
 *   | 0x1E   |                                                       |
 *   | 0x1F   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x20   | s_zmap[8]:                                            |
 *   | 0x21   |   Points to buffer for Zone Bitmap                    |
 *   | 0x22   |                                                       |
 *   | 0x23   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x24   | s_dirsize:                                            |
 *   | 0x25   |   Indicate the size of directory                      |
 *   | 0x26   |                                                       |
 *   | 0x27   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x28   | s_namelen:                                            |
 *   | 0x29   |   Indicate the name length on dentry entry            |
 *   | 0x2A   |                                                       |
 *   | 0x2B   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x2C   | s_sbh:                                                |
 *   | 0x2D   |   Point to buffer that contain minix super block      |
 *   | 0x2E   |                                                       |
 *   | 0x2F   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x30   | s_ms:                                                 |
 *   | 0x31   |   Point to 'minix_super_block' structure              |
 *   | 0x32   |                                                       |
 *   | 0x33   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x34   | Mount state, was it cleanly unmount                   |
 *   | 0x35   |                                                       |
 *   +--------+-------------------------------------------------------+
 */
static __unused int minixfs_super_block(struct super_block *sb)
{
    struct minix_super_block *ms;
    struct buffer_head *bh = NULL;
    int imap_blocks, zmap_blocks;
    int i, block;
    unsigned long firstdatazone;

    /* We have two ways to obtain minix fs super block
     *  1. Read minix fs super block from Disk
     *  2. Obtain minix fs super block from super block.
     */
    ms = sb->u.minix_sb.s_ms;
    if (!ms) {
        if (!(bh = bread(sb->s_dev, SUPER_BLOCK, BLOCK_SIZE))) {
            printk(KERN_ERR "Faild to read super block\n");
            return -EINVAL;
        }
        sb->u.minix_sb.s_ms = ms;
    }

    /*
     * s_magic:
     *  The File system Magic for MINIX-FS V0/V1/V2. To now, minixfs has
     *  public 3 version, and different version has a unique magic.
     * 
     *    MINIX_SUPER_MAGIC     0x137F
     *    MINIX_SUPER_MAGIC2    0x138F
     */
    if (MINIX_SUPER_MAGIC != ms->s_magic && 
                      MINIX_SUPER_MAGIC2 != ms->s_magic)
        panic("Invalid MINIX-FS MAGIC");

    /*
     * s_ninodes:
     *  It indicates the numbe of inode for minix-fs. It's often used to
     *  calculate the block number for Inode BitMap. Each block contains
     *  1024 * 8 bits.
     *
     *   s_imap_blocks = (s_ninodes + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     *
     *  Align with BLOCK_SIZE. 
     * 
     * s_imap_blocks:
     *  It indicates the number of block for Inode-BitMap. Each bit on
     *  Inode-BitMap represents a used/unused inode. If a bit is zero and
     *  indicate the special inode is unused.  
     */
    imap_blocks = (ms->s_ninodes + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
    if (imap_blocks != ms->s_imap_blocks)
        panic("Un-Alignment for s_imap_blocks");

    /*
     * s_nzones:
     *  It indicates the number of zone for minix-fs. It's used to 
     *  manage zone and calculate the block number for Zone BitMap.
     *  Each block contains 1024 * 8 bits.
     *
     *   s_zmap_blocks = (s_nzones + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     *
     *  Align with BLOCK_SIZE.
     *
     * s_zmap_blocks:
     *  It indicates the number of block for Zone-BitMap. Each bit on 
     *  Zone-BitMap represents a used/unused zone. If a bit is zero and
     *  indicates the special zone is unused.
     */
    zmap_blocks = (ms->s_nzones + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
    if (zmap_blocks != ms->s_zmap_blocks)
        panic("Un-Alignment for s_zmap_blocks");

    /*
     * s_firstdatazone:
     *  It indicate the block order for first data zone. It's used to
     *  calculate the block order for special inode. The member 'i_zone'
     *  on "struct minix_inode" contain the block order on data zone.
     *
     *  +------+------------+-----+--------+-------+----------+-------+
     *  |      |            |     |        |       |          |       |
     *  | Boot | Superblock | ... | Zone0  | Zone1 | ........ | Zonen | 
     *  |      |            |     |        |       |          |       |
     *  +------+------------+-----+--------+-------+----------+-------+
     *                            A        A
     *                            |        |
     *                            |        |
     *                            |        |
     *     s_firstdatazone--------o        |
     *                                     |
     *                                     |
     *  +-----------+                      |
     *  |           |                      |
     *  |   minix   |                      |
     *  |   inode   |                      |
     *  |           |                      |
     *  +-----------+    i_zone[0] = m     |
     *  | i_zone[0] |----------------------o
     *  +-----------+
     *  | i_zone[1] |
     *  +-----------+
     *  | i_zone[2] |
     *  +-----------+
     *  | i_zone[3] |
     *  +-----------+
     *  | i_zone[4] |
     *  +-----------+
     *  | i_zone[5] |
     *  +-----------+
     *  | i_zone[6] |
     *  +-----------+
     *  | i_zone[7] |
     *  +-----------+
     *  | i_zone[8] |
     *  +-----------+
     *
     *  So, when we calculate the block order for speical inode as follow:
     *
     *    Blocks(inode) = inode->i_zone[x] + s_firstdatazone
     *
     *  Before first data zone, minix-fs contain boot, superblock, 
     *  Inode-BitMap, Zone-BitMap and Inode block. So we can calculate 
     *  the first data zone as follow:
     *
     *    firstdatazone = 2 + s_imap_blocks + s_zmap_block +
     *        (sizeof(struct minix_inode) * s_ninodes + BLOCK_SIZE - 1) / 
     *                         BLOCK_SIZE;
     */
     firstdatazone = 2 + sb->u.minix_sb.s_imap_blocks + 
                         sb->u.minix_sb.s_zmap_blocks + 
                ((sizeof(struct minix_inode) * sb->u.minix_sb.s_ninodes) +
                          BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (firstdatazone != sb->u.minix_sb.s_firstdatazone)
        panic("Un-Alignment s_firstdatazone");
    /*
     * s_log_zone_size:
     *  It indicate the size of Data zone on minix-fs. On default, the size
     *  of data zone is BLOCK_SIZE which 1024 Bytes. 's_log_zone_size' is a
     *  exponent, we should calculate data zone as follow:
     * 
     *     Size = 1024 << s_log_zone_size
     *
     */
    if (BLOCK_SIZE != (1024 << sb->u.minix_sb.s_log_zone_size))
        panic("Minix-fs utilizes unalign BLOCK_SIZE");

    /*
     * s_max_size:
     *  It indicate the maximum size for file on minix-fs. If size of file
     *  is bigger then 's_max_size', the system will trigger fault.
     */
    printk("Maximum file size: %#08x\n", (unsigned int)ms->s_max_size);

    /*
     * s_imap[8]:
     *  It manage all valid Inode-Bitmap buffer. Each minix-fs superblock
     *  can hold 8 Inode-BitMap.
     * 
     *  |<-00->| <---01---> | <--2--> | <--3--> |    | <--9--> |
     *  +------+------------+---------+---------+----+---------+--------+
     *  |      |            |         |         |    |         |        |
     *  | Boot | SuperBlock | imap[0] | imap[1] | .. | imap[7] | ...... |  
     *  |      |            |         |         |    |         |        |
     *  +------+------------+---------+---------+----+---------+--------+
     *                      A         A              A
     *                      |         |              |
     *                      |         |              |
     *                      |         |              |
     *  +-------------+     |         |              |
     *  |             |     |         |              |
     *  |   minixfs   |     |         |              |
     *  | superblock  |     |         |              |
     *  |             |     |         |              |
     *  +-------------+     |         |              |
     *  |  s_imap[0] -|-----o         |              |
     *  +-------------+               |              |
     *  |  s_imap[1] -|---------------o              |
     *  +-------------+                              |
     *  |  s_imap[2]  |                              |
     *  +-------------+                              |
     *  |  s_imap[3]  |                              |
     *  +-------------+                              |
     *  |  s_imap[4]  |                              |
     *  +-------------+                              |
     *  |  s_imap[5]  |                              |
     *  +-------------+                              |
     *  |  s_imap[6]  |                              |
     *  +-------------+                              |
     *  |  s_imap[7] -|------------------------------o
     *  +-------------+ 
     * 
     *  more information about 'Inode-BitMap' see: minixfs_inode_bitmap 
     */
    block = 2; /* skip boot and superblock block */
    for (i = 0; i < sb->u.minix_sb.s_ms->s_imap_blocks; i++) {
        if (!sb->u.minix_sb.s_imap[i])
            sb->u.minix_sb.s_imap[i] = 
                   bread(sb->s_dev, block, BLOCK_SIZE);
        block++;
    }

    /*
     * s_zmap[8]:
     *  It manage all valid Zone-Bitmap buffer. Each minix-fs superblock
     *  can hold 8 Zone-BitMap.
     * 
     *  |<-00->| <---01---> |    
     *  +------+------------+----+---------+---------+----+---------+----+
     *  |      |            |    |         |         |    |         |    |
     *  | Boot | SuperBlock | .. | zmap[0] | zmap[1] | .. | zmap[7] | .. |  
     *  |      |            |    |         |         |    |         |    |
     *  +------+------------+----+---------+---------+----+---------+----+
     *                           A         A              A
     *                           |         |              |
     *                           |         |              |
     *                           |         |              |
     *  +-------------+          |         |              |
     *  |             |          |         |              |
     *  |   minixfs   |          |         |              |
     *  | superblock  |          |         |              |
     *  |             |          |         |              |
     *  +-------------+          |         |              |
     *  |  s_zmap[0] -|----------o         |              |
     *  +-------------+                    |              |
     *  |  s_zmap[1] -|--------------------o              |
     *  +-------------+                                   |
     *  |  s_zmap[2]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[3]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[4]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[5]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[6]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[7] -|-----------------------------------o
     *  +-------------+ 
     * 
     *  more information about 'Zone-BitMap' see: minix_zone_bitmap
     */
    for (i = 0; i < sb->u.minix_sb.s_ms->s_zmap_blocks; i++) {
        if (!sb->u.minix_sb.s_zmap[i])
            sb->u.minix_sb.s_zmap[i] =
                   bread(sb->s_dev, block, BLOCK_SIZE);
        block++;
    }

    /*
     * s_dirsize:
     *  It indicates the size of minix-direntory. Minix-fs utilize structure
     *  'minix_dir_entry' and define as follow:
     *
     *      struct minix_dir_entry {
     *          unsigned short inode;
     *          char name[0];
     *      }
     *
     *  As above, the 'name' is a empty string array, and 'sizeof' doesn't 
     *  cacluate array size. so the size of 'minix_dir_entry' is 2.
     *  Minix-fs utilize it to manage all direntory which same length for
     *  'name'. And 's_namelen' indicate the size of name string array.
     *  So diretry length as follow:
     *
     *     s_dirsize = 2 + s_namelen
     *
     * s_namelen:
     *  It indicates the length of empty string array.
     *
     *  |<-- s_dirsize -->|<-- s_dirsize -->|     |<-- s_dirsize -->|
     *  |<-2 + s_namelen->|<-2 + s_namelen->|     |<-2 + s_namelen->|
     *  +-----------------+-----------------+-----+-----------------+----+
     *  |                 |                 |     |                 |    |
     *  | minix_dir_entry | minix_dir_entry | ... | minix_dir_entry | .. |
     *  |                 |                 |     |                 |    |
     *  +-----------------+-----------------+-----+-----------------+----+
     *
     */
    if (sb->u.minix_sb.s_dirsize != (2 + sb->u.minix_sb.s_namelen))
        panic("Invalid size for minix_dir_entry");

    /*
     *  On differnet MINIX-FS version, s_namelen and s_dirsize doesn't has 
     *  accordant value.
     *
     *   MINIX_SUPER_MAGIC (0x137F)
     *     s_dirsize = 16
     *     s_namelen = 14
     *   MINIX_SUPER_MAGIC2 (0x138F)
     *     s_dirsize = 32
     *     s_namelen = 30
     */  
    if (sb->u.minix_sb.s_ms->s_magic == MINIX_SUPER_MAGIC) {
        if (sb->u.minix_sb.s_dirsize != 16)
            panic("MINIX_SUPER_MAGIC invalid s_dirsize");
        if (sb->u.minix_sb.s_namelen != 14)
            panic("MINIX_SUPER_MAGIC invalid s_namelen");
    } else if (sb->u.minix_sb.s_ms->s_magic == MINIX_SUPER_MAGIC2) {
        if (sb->u.minix_sb.s_dirsize != 32)
            panic("MINIX_SUPER_MAGIC2 invalid s_dirsize");
        if (sb->u.minix_sb.s_namelen != 30)
            panic("MINIX_SUPER_MAGIC2 invalid s_namelen");
    }

    /*
     * s_sbh:
     *  It hold the superblock buffer. 'b_data' point to data information
     *  about superblock on minixf-fs.
     *
     * s_ms:
     *  It hold structure of 'minix_super_block'. We often confuse two
     *  structure 'minix_super_block' and "minix_sb_info". The 'minix_super_
     *  -block' is used to describe basic super block information on Disk. 
     *  And the 'minix_sb_info' is used to describe superblock information
     *  on RAM.
     */
    if (sb->u.minix_sb.s_ms != 
         (struct minix_super_block *)(sb->u.minix_sb.s_sbh->b_data))
        panic("Invalie minix_super_block and minix_sb_info");

    /*
     * s_mount_state:
     *  It indicate whether minix-fs has beed mounted. If it is 1
     *  means minix-fs isn't mounted.
     */
    if (sb->u.minix_sb.s_mount_state)
        panic("MINIX-FS unmount!");

    if (bh)
        brelse(bh);
    return 0;
}

/*
 * minixfs_inode_bitmap
 * 
 *   Inode-BitMap is used to manage all unused/used inode. Each bit on
 *   represent the usage of inode. The minix-fs utilize the unit of block
 *   to store all BitMap and the 'sb->u.minix_sb.s_imap[]' point all
 *   Inode-BitMap buffer. The 'sb->u.minix_sb.s_ninodes' indicate the 
 *   number of inode and 'sb->u.minix_sb.s_imap_blocks' indicates the
 *   number of Inode-BitMap.
 */
static __unused int minixfs_inode_bitmap(struct super_block *sb)
{
    int ninodes, imap_blocks;
    struct buffer_head *bh;
    int block;
    int i, j, ino;
    struct minix_inode *inode;

    /*
     * The start block of Inode-BitMap is 2 (0: boot, 1: superblock).
     * And block number of Inode-BitMap hold on 's_imap_block'.
     *
     * s_ninodes:
     *  It indicates the numbe of inode for minix-fs. It's often used to
     *  calculate the block number for Inode BitMap. Each block contains
     *  1024 * 8 bits.
     *
     *   s_imap_blocks = (s_ninodes + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     *
     *  Align with BLOCK_SIZE. 
     * 
     * s_imap_blocks:
     *  It indicates the number of block for Inode-BitMap. Each bit on
     *  Inode-BitMap represents a used/unused inode. If a bit is zero and
     *  indicate the special inode is unused.  
     */
     ninodes = sb->u.minix_sb.s_ninodes;
     imap_blocks = (ninodes + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     if (imap_blocks != sb->u.minix_sb.s_imap_blocks)
         panic("Un-Alignment for s_imap_blocks");

    /*
     * s_imap[8]:
     *  It manage all valid Inode-Bitmap buffer. Each minix-fs superblock
     *  can hold 8 Inode-BitMap.
     * 
     *  |<-00->| <---01---> | <--2--> | <--3--> |    | <--9--> |
     *  +------+------------+---------+---------+----+---------+--------+
     *  |      |            |         |         |    |         |        |
     *  | Boot | SuperBlock | imap[0] | imap[1] | .. | imap[7] | ...... |  
     *  |      |            |         |         |    |         |        |
     *  +------+------------+---------+---------+----+---------+--------+
     *                      A         A              A
     *                      |         |              |
     *                      |         |              |
     *                      |         |              |
     *  +-------------+     |         |              |
     *  |             |     |         |              |
     *  |   minixfs   |     |         |              |
     *  | superblock  |     |         |              |
     *  |             |     |         |              |
     *  +-------------+     |         |              |
     *  |  s_imap[0] -|-----o         |              |
     *  +-------------+               |              |
     *  |  s_imap[1] -|---------------o              |
     *  +-------------+                              |
     *  |  s_imap[2]  |                              |
     *  +-------------+                              |
     *  |  s_imap[3]  |                              |
     *  +-------------+                              |
     *  |  s_imap[4]  |                              |
     *  +-------------+                              |
     *  |  s_imap[5]  |                              |
     *  +-------------+                              |
     *  |  s_imap[6]  |                              |
     *  +-------------+                              |
     *  |  s_imap[7] -|------------------------------o
     *  +-------------+ 
     *
     *  Read Inode-BitMap buffer from Disk into superblock.
     */ 
    block = 2; /* skip boot and superblock block */
    for (i = 0; i < sb->u.minix_sb.s_ms->s_imap_blocks; i++) {
        if (!sb->u.minix_sb.s_imap[i])
            sb->u.minix_sb.s_imap[i] =
                   bread(sb->s_dev, block, BLOCK_SIZE);
        block++;
    }

    /*
     * Find a valid inode from Inode-BitMap. A unit of Inode-BitMap contains
     * (1024 * 8) bits. Each bit represent a unused/used inode. We can
     * find a empty inode from 0 to maximum.
     */
    j = 1024 * 8;
    for (i = 0; i < sb->u.minix_sb.s_imap_blocks; i++) {
        if ((bh = sb->u.minix_sb.s_imap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    }
    if (i >= 8 || !bh || j >= 8192)
        panic("Can't find a valid inode!");
    /* Set spical bit to used inode */
    if (set_bit(j, bh->b_data))
        panic("bit already set");


    /*
     * Obtain speical inode on Inode-Table.
     *   Inode-Table is a part of MINIX-fs, it's behind of Zone-BitMap.
     *   So start block of Inode-Table is:
     * 
     *     block = 2 + s_imap_blocks + s_zmap_blocks
     *
     * The 'ino' is index on Inode-Table, it's used to indicate a special
     * inode. The minix-fs utilize structure 'minix_inode' to mange a 
     * exist inode information. The "Inode-Table" is a array that contain
     * all inode for minix-fs. As fllow:
     *
     *  Inode Table
     *
     *  +-------------+-------------+------+-------------+-------------+
     *  |             |             |      |             |             |
     *  | minix_inode | minix_inode | .... | minix_inode | minix_inode |
     *  |             |             |      |             |             |
     *  +-------------+-------------+------+-------------+-------------+
     *
     *  MINIX_INODES_PER_BLOCK:
     *   This macro indicates the number of inode on a BLOCK. So the block
     *   order for 'ino' is:
     *  
     *      blocks = 2 + s_imap_blocks + s_zmap_blocks +
     *                (ino - 1) / MINIX_INODES_PER_BLOCK;
     */
    ino = j + i * 8 * 1024;
    block = 2 + sb->u.minix_sb.s_imap_blocks + 
                sb->u.minix_sb.s_zmap_blocks +
                (ino - 1) / MINIX_INODES_PER_BLOCK;
    /* Read part of inode-table from Disk */
    if (!(bh = bread(sb->s_dev, block, BLOCK_SIZE))) {
        printk(KERN_ERR "Unable to obtain inode: %d\n", ino);
        return -EINVAL;
    }

    /*
     * The minix-fs utilizes 'minix_inode' to manage minix-fs inode that
     * be stored in Inode-table. when obtain Inode-table buffer,
     * we can obtain special inode as follow:
     *
     *    inode = ((struct minix_inode *)(bh->b_data)) + 
     *               (ino - 1) % MINIX_INODES_PER_BLOCK;
     *
     *  more inode information see: 
     */
    inode = ((struct minix_inode *) bh->b_data) +
              (ino - 1) % MINIX_INODES_PER_BLOCK;
    if (!inode) {
        printk(KERN_ERR "Unable to obtain speical inode from disk\n");
        return -EINVAL;
    }

    if (bh)
        brelse(bh);

    return 0;
}

/*
 * Zone-BitMap
 *  It's used to manage all used/unused Data zone. Data zone is used to
 *  store data information. Each bit represents a speical data zone.
 *  The minix-fs utilize 's_zmap_blocks' and 's_nzones' to indicate 
 *  Zone-BitMap size.
 *
 * Data Zone:
 *  It's a unit that used to store data, and MINIX-fs divide into unit
 *  as data zone. The length of zone is baisc on 's_log_zone_size'.
 */
static __unused int minixfs_zone_bitmap(struct super_block *sb)
{
    struct buffer_head *bh;
    int nzones, zmap_blocks;
    int block, firstdatazone;
    int i, j;
    int zonesize, zone_nr;
    char __unused *buf;

    /*
     * s_nzones:
     *  It indicates the number of zone for minix-fs. It's used to 
     *  manage zone and calculate the block number for Zone BitMap.
     *  Each block contains 1024 * 8 bits.
     *
     *   s_zmap_blocks = (s_nzones + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     *
     *  Align with BLOCK_SIZE.
     *
     * s_zmap_blocks:
     *  It indicates the number of block for Zone-BitMap. Each bit on 
     *  Zone-BitMap represents a used/unused zone. If a bit is zero and
     *  indicates the special zone is unused.
     */
    nzones = sb->u.minix_sb.s_nzones;
    zmap_blocks = (nzones + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
    if (zmap_blocks != sb->u.minix_sb.s_zmap_blocks)
        panic("s_zmap_blocks doesn't alignment!");

    /*
     * s_zmap[8]:
     *  It manage all valid Zone-Bitmap buffer. Each minix-fs superblock
     *  can hold 8 Zone-BitMap.
     * 
     *  |<-00->| <---01---> |    
     *  +------+------------+----+---------+---------+----+---------+----+
     *  |      |            |    |         |         |    |         |    |
     *  | Boot | SuperBlock | .. | zmap[0] | zmap[1] | .. | zmap[7] | .. |  
     *  |      |            |    |         |         |    |         |    |
     *  +------+------------+----+---------+---------+----+---------+----+
     *                           A         A              A
     *                           |         |              |
     *                           |         |              |
     *                           |         |              |
     *  +-------------+          |         |              |
     *  |             |          |         |              |
     *  |   minixfs   |          |         |              |
     *  | superblock  |          |         |              |
     *  |             |          |         |              |
     *  +-------------+          |         |              |
     *  |  s_zmap[0] -|----------o         |              |
     *  +-------------+                    |              |
     *  |  s_zmap[1] -|--------------------o              |
     *  +-------------+                                   |
     *  |  s_zmap[2]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[3]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[4]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[5]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[6]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[7] -|-----------------------------------o
     *  +-------------+ 
     * 
     *  The start block of Zone-BitMap is behind of Inode-BitMap. So
     *  Start block is:
     *
     *    block = 2 + sb->u.minix_sb.s_imap_blocks
     */
     block = 2 + sb->u.minix_sb.s_imap_blocks;
     for (i = 0; i < sb->u.minix_sb.s_zmap_blocks; i++) {
         /* Buffer doesn't exist */
         if (!sb->u.minix_sb.s_zmap[i])
             if (!(bh = bread(sb->s_dev, block, BLOCK_SIZE))) {
                 printk(KERN_ERR "Unable obtain Zone-BitMap\n");
                 return -EINVAL;
             }
         block++;
     }

    /*
     * Find first empty data zone from Zone-BitMap, A unit of Zone-BitMap 
     * contains (1024 * 8) bits. Each bit represent a unused/used Zone. 
     * We can find a empty inode from 0 to maximum.
     */
    j = 1024 * 8;
    for (i = 0; i < sb->u.minix_sb.s_zmap_blocks; i++)
        if ((bh = sb->u.minix_sb.s_zmap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < (1024 * 8))
                break;

    if (i > 8 || !bh || j >= 8192)
        panic("Invalid Zone-BitMap");
    if (set_bit(j, bh->b_data))
        panic("bit already set on zone-bitmap");

    /*
     * s_firstdatazone:
     *  It indicate the block order for first data zone. It's used to
     *  calculate the block order for special inode. The member 'i_zone'
     *  on "struct minix_inode" contain the block order on data zone.
     *
     *  +------+------------+-----+--------+-------+----------+-------+
     *  |      |            |     |        |       |          |       |
     *  | Boot | Superblock | ... | Zone0  | Zone1 | ........ | Zonen | 
     *  |      |            |     |        |       |          |       |
     *  +------+------------+-----+--------+-------+----------+-------+
     *                            A        A
     *                            |        |
     *                            |        |
     *                            |        |
     *     s_firstdatazone--------o        |
     *                                     |
     *                                     |
     *  +-----------+                      |
     *  |           |                      |
     *  |   minix   |                      |
     *  |   inode   |                      |
     *  |           |                      |
     *  +-----------+    i_zone[0] = x     |
     *  | i_zone[0] |----------------------o
     *  +-----------+
     *  | i_zone[1] |
     *  +-----------+
     *  | i_zone[2] |
     *  +-----------+
     *  | i_zone[3] |
     *  +-----------+
     *  | i_zone[4] |
     *  +-----------+
     *  | i_zone[5] |
     *  +-----------+
     *  | i_zone[6] |
     *  +-----------+
     *  | i_zone[7] |
     *  +-----------+
     *  | i_zone[8] |
     *  +-----------+
     *
     *  So, when we calculate the block order for speical inode as follow:
     *
     *    Blocks(inode) = inode->i_zone[x] + s_firstdatazone
     *
     *  Before first data zone, minix-fs contain boot, superblock, 
     *  Inode-BitMap, Zone-BitMap and Inode block. So we can calculate 
     *  the first data zone as follow:
     *
     *    firstdatazone = 2 + s_imap_blocks + s_zmap_block +
     *        (sizeof(struct minix_inode) * s_ninodes + BLOCK_SIZE - 1) / 
     *                         BLOCK_SIZE;
     */
    firstdatazone = 2 + sb->u.minix_sb.s_imap_blocks +
                        sb->u.minix_sb.s_zmap_blocks +
              (sizeof(struct minix_inode) * sb->u.minix_sb.s_ninodes +
                          BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (firstdatazone != sb->u.minix_sb.s_firstdatazone)
        panic("Invalid s_firstdatazone on minix-fs");

    /*
     * s_log_zone_size:
     *  It indicate the size of Data zone on minix-fs. On default, the size
     *  of data zone is BLOCK_SIZE which 1024 Bytes. 's_log_zone_size' is a
     *  exponent, we should calculate data zone as follow:
     * 
     *     Size = 1024 << s_log_zone_size
     *
     */
    zonesize = 1024 << sb->u.minix_sb.s_log_zone_size;

    /*
     * Obtain special data zone. The s_firstdatazone points to start block
     * of data zone. So we can obtain spcial data zone as follow:
     *
     *    DataZone = zone_nr + s_firstdatazone - 1
     */
    zone_nr = j + i * 1024 * 8 + sb->u.minix_sb.s_firstdatazone - 1;
    /*
     *  Verify zone nr, Reserved data zone is litter than s_firstdatazone 
     *
     *
     * | <- Zone0 -> | <- Zone1 -> | ....... | <- Zonem -> | 
     * +-------------+-------------+---------+-------------+----------+
     * |             |             |         |             |          |
     * |     Boot    |  Superblock | .....   |  Data Zone  | ....     |
     * |             |             |         |             |          |
     * +-------------+-------------+---------+-------------+----------+
     * | <--------- Reserved Zone ---------> A
     *                                       |
     *                                       |
     *                 s_firstdatazone-------o
     */
    if (zone_nr < sb->u.minix_sb.s_firstdatazone)
        panic("Invalid size for zone nr");
    
    /*
     * Read a data zone from Disk.
     *
     * s_log_zone_size:
     *  It indicate the size of Data zone on minix-fs. On default, the size
     *  of data zone is BLOCK_SIZE which 1024 Bytes. 's_log_zone_size' is a
     *  exponent, we should calculate data zone as follow:
     * 
     *     Size = 1024 << s_log_zone_size
     *
     */
    zonesize = 1024 << sb->u.minix_sb.s_log_zone_size;
    if (zonesize >= BLOCK_SIZE)
        zonesize = BLOCK_SIZE;
    if (!(bh = bread(sb->s_dev, zone_nr, zonesize))) {
        printk(KERN_ERR "Unable to read data zone from disk.\n");
        return -EINVAL;
    }

    /* Get Data area and Read/Write operation */
    buf = (char *)(bh->b_data);

    if (bh)
        brelse(bh);

    return 0;
}

/*
 * Inode Table
 *  This table is used to manage all inode structure. It locates on between 
 *  'Zone-BitMap' and 'Data zone'. MINIX-fs utilize structure 'minix_inode'
 *  to represent a inode that hold meta data for regular file or directory.
 */
static __unused int minixfs_inode_table(struct super_block *sb)
{
    int ninodes, imap_blocks;
    struct buffer_head *bh;
    int block;
    int i, j, ino;
    struct minix_inode *inode;

    /*
     * The start block of Inode-BitMap is 2 (0: boot, 1: superblock).
     * And block number of Inode-BitMap hold on 's_imap_block'.
     *
     * s_ninodes:
     *  It indicates the numbe of inode for minix-fs. It's often used to
     *  calculate the block number for Inode BitMap. Each block contains
     *  1024 * 8 bits.
     *
     *   s_imap_blocks = (s_ninodes + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     *
     *  Align with BLOCK_SIZE. 
     * 
     * s_imap_blocks:
     *  It indicates the number of block for Inode-BitMap. Each bit on
     *  Inode-BitMap represents a used/unused inode. If a bit is zero and
     *  indicate the special inode is unused.  
     */
     ninodes = sb->u.minix_sb.s_ninodes;
     imap_blocks = (ninodes + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     if (imap_blocks != sb->u.minix_sb.s_imap_blocks)
         panic("Un-Alignment for s_imap_blocks");

    /*
     * s_imap[8]:
     *  It manage all valid Inode-Bitmap buffer. Each minix-fs superblock
     *  can hold 8 Inode-BitMap.
     * 
     *  |<-00->| <---01---> | <--2--> | <--3--> |    | <--9--> |
     *  +------+------------+---------+---------+----+---------+--------+
     *  |      |            |         |         |    |         |        |
     *  | Boot | SuperBlock | imap[0] | imap[1] | .. | imap[7] | ...... |  
     *  |      |            |         |         |    |         |        |
     *  +------+------------+---------+---------+----+---------+--------+
     *                      A         A              A
     *                      |         |              |
     *                      |         |              |
     *                      |         |              |
     *  +-------------+     |         |              |
     *  |             |     |         |              |
     *  |   minixfs   |     |         |              |
     *  | superblock  |     |         |              |
     *  |             |     |         |              |
     *  +-------------+     |         |              |
     *  |  s_imap[0] -|-----o         |              |
     *  +-------------+               |              |
     *  |  s_imap[1] -|---------------o              |
     *  +-------------+                              |
     *  |  s_imap[2]  |                              |
     *  +-------------+                              |
     *  |  s_imap[3]  |                              |
     *  +-------------+                              |
     *  |  s_imap[4]  |                              |
     *  +-------------+                              |
     *  |  s_imap[5]  |                              |
     *  +-------------+                              |
     *  |  s_imap[6]  |                              |
     *  +-------------+                              |
     *  |  s_imap[7] -|------------------------------o
     *  +-------------+ 
     *
     *  Read Inode-BitMap buffer from Disk into superblock.
     */ 
    block = 2; /* skip boot and superblock block */
    for (i = 0; i < sb->u.minix_sb.s_ms->s_imap_blocks; i++) {
        if (!sb->u.minix_sb.s_imap[i])
            sb->u.minix_sb.s_imap[i] =
                   bread(sb->s_dev, block, BLOCK_SIZE);
        block++;
    }

    /*
     * Find a valid inode from Inode-BitMap. A unit of Inode-BitMap contains
     * (1024 * 8) bits. Each bit represent a unused/used inode. We can
     * find a empty inode from 0 to maximum.
     */
    j = 1024 * 8;
    for (i = 0; i < sb->u.minix_sb.s_imap_blocks; i++) {
        if ((bh = sb->u.minix_sb.s_imap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    }
    if (i >= 8 || !bh || j >= 8192)
        panic("Can't find a valid inode!");
    /* Set spical bit to used inode */
    if (set_bit(j, bh->b_data))
        panic("bit already set");


    /*
     * Obtain speical inode on Inode-Table.
     *   Inode-Table is a part of MINIX-fs, it's behind of Zone-BitMap.
     *   So start block of Inode-Table is:
     * 
     *     block = 2 + s_imap_blocks + s_zmap_blocks
     *
     * The 'ino' is index on Inode-Table, it's used to indicate a special
     * inode. The minix-fs utilize structure 'minix_inode' to mange a 
     * exist inode information. The "Inode-Table" is a array that contain
     * all inode for minix-fs. As fllow:
     *
     *  Inode Table
     *
     *  +-------------+-------------+------+-------------+-------------+
     *  |             |             |      |             |             |
     *  | minix_inode | minix_inode | .... | minix_inode | minix_inode |
     *  |             |             |      |             |             |
     *  +-------------+-------------+------+-------------+-------------+
     *
     *  MINIX_INODES_PER_BLOCK:
     *   This macro indicates the number of inode on a BLOCK. So the block
     *   order for 'ino' is:
     *  
     *      blocks = 2 + s_imap_blocks + s_zmap_blocks +
     *                (ino - 1) / MINIX_INODES_PER_BLOCK;
     */
    ino = j + i * 8 * 1024;
    block = 2 + sb->u.minix_sb.s_imap_blocks + 
                sb->u.minix_sb.s_zmap_blocks +
                (ino - 1) / MINIX_INODES_PER_BLOCK;
    /* Read part of inode-table from Disk */
    if (!(bh = bread(sb->s_dev, block, BLOCK_SIZE))) {
        printk(KERN_ERR "Unable to obtain inode: %d\n", ino);
        return -EINVAL;
    }

    /*
     * The minix-fs utilizes 'minix_inode' to manage minix-fs inode that
     * be stored in Inode-table. when obtain Inode-table buffer,
     * we can obtain special inode as follow:
     *
     *    inode = ((struct minix_inode *)(bh->b_data)) + 
     *               (ino - 1) % MINIX_INODES_PER_BLOCK;
     *
     *  more inode information see: 
     */
    inode = ((struct minix_inode *) bh->b_data) +
              (ino - 1) % MINIX_INODES_PER_BLOCK;
    if (!inode) {
        printk(KERN_ERR "Unable to obtain speical inode from disk\n");
        return -EINVAL;
    }

    if (bh)
        brelse(bh);

    return 0;
}

/*
 * Data Zone
 *  Minix-fs divides Rootfs into same size unit from start address to 
 *  end address.
 *
 *  | <- Zone0 -> | <- Zone1 -> | ....... | <- Zonem -> | 
 *  +-------------+-------------+---------+-------------+----------+
 *  |             |             |         |             |          |
 *  |     Boot    |  Superblock | .....   |  Data Zone  | ....     |
 *  |             |             |         |             |          |
 *  +-------------+-------------+---------+-------------+----------+
 *  | <--------- Reserved Zone ---------> A
 *                                        |
 *                                        |
 *                  s_firstdatazone-------o
 *
 *  The Data zone is a part of Zone and it only store meta data of file
 *  or directory.
 */
static __unused int minixfs_data_zone(struct super_block *sb)
{
    struct buffer_head *bh;
    int nzones, zmap_blocks;
    int block, firstdatazone;
    int i, j;
    int zonesize, zone_nr;
    char __unused *buf;

    /*
     * s_nzones:
     *  It indicates the number of zone for minix-fs. It's used to 
     *  manage zone and calculate the block number for Zone BitMap.
     *  Each block contains 1024 * 8 bits.
     *
     *   s_zmap_blocks = (s_nzones + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     *
     *  Align with BLOCK_SIZE.
     *
     * s_zmap_blocks:
     *  It indicates the number of block for Zone-BitMap. Each bit on 
     *  Zone-BitMap represents a used/unused zone. If a bit is zero and
     *  indicates the special zone is unused.
     */
    nzones = sb->u.minix_sb.s_nzones;
    zmap_blocks = (nzones + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
    if (zmap_blocks != sb->u.minix_sb.s_zmap_blocks)
        panic("s_zmap_blocks doesn't alignment!");

    /*
     * s_zmap[8]:
     *  It manage all valid Zone-Bitmap buffer. Each minix-fs superblock
     *  can hold 8 Zone-BitMap.
     * 
     *  |<-00->| <---01---> |    
     *  +------+------------+----+---------+---------+----+---------+----+
     *  |      |            |    |         |         |    |         |    |
     *  | Boot | SuperBlock | .. | zmap[0] | zmap[1] | .. | zmap[7] | .. |  
     *  |      |            |    |         |         |    |         |    |
     *  +------+------------+----+---------+---------+----+---------+----+
     *                           A         A              A
     *                           |         |              |
     *                           |         |              |
     *                           |         |              |
     *  +-------------+          |         |              |
     *  |             |          |         |              |
     *  |   minixfs   |          |         |              |
     *  | superblock  |          |         |              |
     *  |             |          |         |              |
     *  +-------------+          |         |              |
     *  |  s_zmap[0] -|----------o         |              |
     *  +-------------+                    |              |
     *  |  s_zmap[1] -|--------------------o              |
     *  +-------------+                                   |
     *  |  s_zmap[2]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[3]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[4]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[5]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[6]  |                                   |
     *  +-------------+                                   |
     *  |  s_zmap[7] -|-----------------------------------o
     *  +-------------+ 
     * 
     *  The start block of Zone-BitMap is behind of Inode-BitMap. So
     *  Start block is:
     *
     *    block = 2 + sb->u.minix_sb.s_imap_blocks
     */
     block = 2 + sb->u.minix_sb.s_imap_blocks;
     for (i = 0; i < sb->u.minix_sb.s_zmap_blocks; i++) {
         /* Buffer doesn't exist */
         if (!sb->u.minix_sb.s_zmap[i])
             if (!(bh = bread(sb->s_dev, block, BLOCK_SIZE))) {
                 printk(KERN_ERR "Unable obtain Zone-BitMap\n");
                 return -EINVAL;
             }
         block++;
     }

    /*
     * Find first empty data zone from Zone-BitMap, A unit of Zone-BitMap 
     * contains (1024 * 8) bits. Each bit represent a unused/used Zone. 
     * We can find a empty inode from 0 to maximum.
     */
    j = 1024 * 8;
    for (i = 0; i < sb->u.minix_sb.s_zmap_blocks; i++)
        if ((bh = sb->u.minix_sb.s_zmap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < (1024 * 8))
                break;

    if (i > 8 || !bh || j >= 8192)
        panic("Invalid Zone-BitMap");
    if (set_bit(j, bh->b_data))
        panic("bit already set on zone-bitmap");

    /*
     * s_firstdatazone:
     *  It indicate the block order for first data zone. It's used to
     *  calculate the block order for special inode. The member 'i_zone'
     *  on "struct minix_inode" contain the block order on data zone.
     *
     *  +------+------------+-----+--------+-------+----------+-------+
     *  |      |            |     |        |       |          |       |
     *  | Boot | Superblock | ... | Zone0  | Zone1 | ........ | Zonen | 
     *  |      |            |     |        |       |          |       |
     *  +------+------------+-----+--------+-------+----------+-------+
     *                            A        A
     *                            |        |
     *                            |        |
     *                            |        |
     *     s_firstdatazone--------o        |
     *                                     |
     *                                     |
     *  +-----------+                      |
     *  |           |                      |
     *  |   minix   |                      |
     *  |   inode   |                      |
     *  |           |                      |
     *  +-----------+    i_zone[0] = x     |
     *  | i_zone[0] |----------------------o
     *  +-----------+
     *  | i_zone[1] |
     *  +-----------+
     *  | i_zone[2] |
     *  +-----------+
     *  | i_zone[3] |
     *  +-----------+
     *  | i_zone[4] |
     *  +-----------+
     *  | i_zone[5] |
     *  +-----------+
     *  | i_zone[6] |
     *  +-----------+
     *  | i_zone[7] |
     *  +-----------+
     *  | i_zone[8] |
     *  +-----------+
     *
     *  So, when we calculate the block order for speical inode as follow:
     *
     *    Blocks(inode) = inode->i_zone[x] + s_firstdatazone
     *
     *  Before first data zone, minix-fs contain boot, superblock, 
     *  Inode-BitMap, Zone-BitMap and Inode block. So we can calculate 
     *  the first data zone as follow:
     *
     *    firstdatazone = 2 + s_imap_blocks + s_zmap_block +
     *        (sizeof(struct minix_inode) * s_ninodes + BLOCK_SIZE - 1) / 
     *                         BLOCK_SIZE;
     */
    firstdatazone = 2 + sb->u.minix_sb.s_imap_blocks +
                        sb->u.minix_sb.s_zmap_blocks +
              (sizeof(struct minix_inode) * sb->u.minix_sb.s_ninodes +
                          BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (firstdatazone != sb->u.minix_sb.s_firstdatazone)
        panic("Invalid s_firstdatazone on minix-fs");

    /*
     * s_log_zone_size:
     *  It indicate the size of Data zone on minix-fs. On default, the size
     *  of data zone is BLOCK_SIZE which 1024 Bytes. 's_log_zone_size' is a
     *  exponent, we should calculate data zone as follow:
     * 
     *     Size = 1024 << s_log_zone_size
     *
     */
    zonesize = 1024 << sb->u.minix_sb.s_log_zone_size;

    /*
     * Obtain special data zone. The s_firstdatazone points to start block
     * of data zone. So we can obtain spcial data zone as follow:
     *
     *    DataZone = zone_nr + s_firstdatazone - 1
     */
    zone_nr = j + i * 1024 * 8 + sb->u.minix_sb.s_firstdatazone - 1;
    /*
     *  Verify zone nr, Reserved data zone is litter than s_firstdatazone 
     *
     *
     * | <- Zone0 -> | <- Zone1 -> | ....... | <- Zonem -> | 
     * +-------------+-------------+---------+-------------+----------+
     * |             |             |         |             |          |
     * |     Boot    |  Superblock | .....   |  Data Zone  | ....     |
     * |             |             |         |             |          |
     * +-------------+-------------+---------+-------------+----------+
     * | <--------- Reserved Zone ---------> A
     *                                       |
     *                                       |
     *                 s_firstdatazone-------o
     */
    if (zone_nr < sb->u.minix_sb.s_firstdatazone)
        panic("Invalid size for zone nr");
    
    /*
     * Read a data zone from Disk.
     *
     * s_log_zone_size:
     *  It indicate the size of Data zone on minix-fs. On default, the size
     *  of data zone is BLOCK_SIZE which 1024 Bytes. 's_log_zone_size' is a
     *  exponent, we should calculate data zone as follow:
     * 
     *     Size = 1024 << s_log_zone_size
     *
     */
    zonesize = 1024 << sb->u.minix_sb.s_log_zone_size;
    if (zonesize >= BLOCK_SIZE)
        zonesize = BLOCK_SIZE;
    if (!(bh = bread(sb->s_dev, zone_nr, zonesize))) {
        printk(KERN_ERR "Unable to read data zone from disk.\n");
        return -EINVAL;
    }

    /* Get Data area and Read/Write operation */
    buf = (char *)(bh->b_data);

    if (bh)
        brelse(bh);

    return 0;
}

/*
 * MINIX Filesystem physical layout
 *
 *  +------+-------------+-----------+----------+-------------+--------+
 *  |      |             |           |          |             |        |
 *  | Boot | Super Block | Inode Map | Zone Map | Inode Table |  Zone  |
 *  |      |             |           |          |             |        |
 *  +------+-------------+-----------+----------+-------------+--------+
 *
 *  MINIX FS consists of 5 parts. Detail as follow:
 *
 *  1) Boot block
 *     Boot block reserved for partition boot code. Total 1 block.
 *  2) Super Block
 *     Super block contains information about the MINIX filesystem.
 *     Total 1 block.    
 *  3) Inode Map
 *     Inode Map is a BitMap that Keeps track of used/unused inodes. Each 
 *     bit represents a used/unused inode.
 *  4) Zone Map
 *     Zone Map is a BitMap that Keeps track of used/unused data zone.
 *     Each bit represents a used/unsed inode.
 *  5) Zone
 *     The 'Zone' consists of a data area that contain Direntory/File
 *     contents.
 *    
 */
static int minix_layout(struct inode *inode, struct super_block *sb)
{
    struct buffer_head *bh = NULL;
    struct minix_super_block *ms;

    if (!sb) {
        /* Read super block from Disk */
        if (!(bh = bread(inode->i_dev, SUPER_BLOCK, BLOCK_SIZE))) {
            printk(KERN_ERR "Unable obtain minix fs super block.\n");
            return -EINVAL;
        }

        ms = (struct minix_super_block *)bh->b_data;
        sb->u.minix_sb.s_ms = ms;
        sb->s_dev = inode->i_dev;
    }

#ifdef CONFIG_DEBUG_BOOT_BLOCK
    /* Parse Boot Block on MINIX-FS */
    minixfs_boot_block(sb);
#endif

#ifdef CONFIG_DEBUG_SUPER_BLOCK
    /* Parse Super Block on MINIX-FS */
    minixfs_super_block(sb);
#endif

#ifdef CONFIG_DEBUG_INODE_BITMAP
    /* Parse Inode-BitMap on MINIX-FS */
    minixfs_inode_bitmap(sb);
#endif

#ifdef CONFIG_DEBUG_ZONE_BITMAP
    /* Parse Zone-BitMap on MINIX-FS */
    minixfs_zone_bitmap(sb);
#endif

#ifdef CONFIG_DEBUG_INODE_TABLE
    /* Parse Inode-Table on MINIX-FS */
    minixfs_inode_table(sb);
#endif

#ifdef CONFIG_DEBUG_DATA_ZONE
    /* Parse Data Zone on MINIX-FS */
    minixfs_data_zone(sb);
#endif

    if (bh)
        brelse(bh);

    return 0;
}

/*
 * Minix-Inode
 *
 *   struct minix_inode:
 *    It indicate the inode for MINIX-FS and store in Disk.
 *
 *   +--------+-------------------------------------------------------+
 *   | Offset | Describe                                              |
 *   +--------+-------------------------------------------------------+
 *   | 0x00   | i_mode: mode for file or directory                    |
 *   | 0x01   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x02   | i_uid: user access permission                         |
 *   | 0x03   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x04   | i_size: The size of file/directory                    |
 *   | 0x05   |                                                       |
 *   | 0x06   |                                                       |
 *   | 0x07   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x08   | i_time: modify/access time                            |
 *   | 0x09   |                                                       |
 *   | 0x0A   |                                                       |
 *   | 0x0B   |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x0C   | i_gid: group access permission                        |
 *   |        |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x0D   | i_nlinks:                                             |
 *   |        |                                                       |
 *   +--------+-------------------------------------------------------+
 *   | 0x0E   | i_zone[0]:                                            |
 *   | 0x0F   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 *   | 0x10   | i_zone[1]:                                            |
 *   | 0x11   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 *   | 0x12   | i_zone[2]:                                            |
 *   | 0x13   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 *   | 0x14   | i_zone[3]:                                            |
 *   | 0x15   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 *   | 0x16   | i_zone[4]:                                            |
 *   | 0x17   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 *   | 0x18   | i_zone[5]:                                            |
 *   | 0x19   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 *   | 0x1A   | i_zone[6]:                                            |
 *   | 0x1B   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 *   | 0x1C   | i_zone[7]:                                            |
 *   | 0x1D   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 *   | 0x1E   | i_zone[8]:                                            |
 *   | 0x1F   |   block order of data zone                            |
 *   +--------+-------------------------------------------------------+
 */
static __unused int minixfs_minix_inode(struct minix_inode *inode,
                           struct super_block *sb)
{
    unsigned short *block_array;
    struct buffer_head *bh = NULL, *bh1 = NULL, *bh2 = NULL;
    unsigned short block_order;
    int blocks;
    int offset;
    char *buffer;

    /*
     * i_mode:
     *  It's used to indicate inode base attribute, such as inode type:
     *  file, director or character device and so on. <linux/stat.h> defind
     *  a serial of macro to determine inode attribute. As follow:
     * 
     *  +-----------+-------------------------------------------------+
     *  |   Macro   | Describe                                        |
     *  +-----------+-------------------------------------------------+
     *  |  S_ISLIK  | Verify whether inode is a soft-/hard-link file  |
     *  +-----------+-------------------------------------------------+
     *  |  S_ISREG  | Verify whether inode is a regular file          |
     *  +-----------+-------------------------------------------------+
     *  |  S_ISDIR  | Verify whether inode is a directory             |
     *  +-----------+-------------------------------------------------+
     *  |  S_ISCHR  | Verify whether inode is a character device      |
     *  +-----------+-------------------------------------------------+
     *  |  S_ISBLK  | Verify whether inode is a block device          |
     *  +-----------+-------------------------------------------------+
     *  |  S_ISFIFO | Verify whether inode is a FIFO                  |
     *  +-----------+-------------------------------------------------+
     *  |  S_ISSOCK | Verify whether inode is a Socket                |
     *  +-----------+-------------------------------------------------+
     * 
     *  The 'i_mode' also contains access permission of minix_inode.
     *  <linux/stat.h> define a serial of macro to indicate access
     *  permission of inode, as follow:
     *
     *  +-----------+-------------------------------------------------+
     *  |   Macro   | Describe                                        |
     *  +-----------+-------------------------------------------------+
     *  |  S_IRUSR  | User can read inode                             |
     *  +-----------+-------------------------------------------------+
     *  |  S_IWUSR  | User can write inode                            |
     *  +-----------+-------------------------------------------------+
     *  |  S_IXUSR  | User can execute inode                          |
     *  +-----------+-------------------------------------------------+
     *  |  S_IRGRP  | Group can read inode                            |
     *  +-----------+-------------------------------------------------+
     *  |  S_IWGRP  | Group can write inode                           |
     *  +-----------+-------------------------------------------------+
     *  |  S_IXGRP  | Group can execute inode                         |
     *  +-----------+-------------------------------------------------+
     *  |  S_IROTH  | Other can read inode                            |
     *  +-----------+-------------------------------------------------+
     *  |  S_IWOTH  | Other can write inode                           |
     *  +-----------+-------------------------------------------------+
     *  |  S_IXOTH  | Other can execute inode                         |
     *  +-----------+-------------------------------------------------+
     *
     *  When execute 'ls' on userland terminal, we can obtain some permission
     *  information as follow:
     *
     *    $root> ls 
     *    $root> -rwxrwxr-x  3 buddy buddy     4096 May 17 15:10 rc
     *
     *  As above example, we analyse each bit:
     * 
     *    -rwxrwxr-x
     *    |||||||||| 
     *    |||||||||o----- Other execute permission
     *    ||||||||o------ Other write permission
     *    |||||||o------- Other read permission
     *    |||||||
     *    ||||||o-------- Group execute permission
     *    |||||o--------- Group write permission
     *    ||||o---------- Group read permission
     *    ||||
     *    |||o----------- User execute permiision
     *    ||o------------ User write permiison
     *    |o------------- User read permission
     *    |
     *    o-------------- Inode type: - regular file
     *                                d directory
     *                                c character device
     *                                s socket
     */
    if (S_ISREG(inode->i_mode))
        printk("Detect a regular file\n");

    /*
     * i_size:
     *  It indicates the size of inode. According to 'i_size', we can 
     *  calculate the block number that inode maintain. As we known, 
     *  The size of block is 'BLOCK_SIZE', as follow:
     *    
     *      block = (i_size + BLOCK_SIZE - 1) / BLOCK_SIZE
     *
     *  Note! 'i_size' isn't big then 's_maxsize' on superblock.
     */
    if (inode->i_size > sb->u.minix_sb.s_max_size)
        panic("Invalid size big than maxsize on superblock");
    blocks = (inode->i_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    /*
     * i_time:
     *  It maintain the last modify/write time. And the value of 'i_time'
     *  is calculate from 1970's.
     */
    inode->i_time = CURRENT_TIME;

    /*
     * i_zone[]
     *  It's used to manage block order that point to data zone for inode.
     *
     *  minix_inode
     *  
     *  +-------------+
     *  |             |
     *  | Other parts |       +-------------+
     *  |             |       |             |
     *  +-------------+       | buffer_head |
     *  |  i_zone[0] -|------>|             |
     *  +-------------+       +-------------+
     *  |  i_zone[1]  |
     *  +-------------+                           +-------------+
     *  |  i_zone[2]  |                           |             |
     *  +-------------+                           | buffer_head |
     *  |  i_zone[3]  |       +----------+        |             |
     *  +-------------+       |  block0 -|------->+-------------+
     *  |  i_zone[4]  |       +----------+
     *  +-------------+       |  block1  |
     *  |  i_zone[5]  |       +----------+
     *  +-------------+       |  ......  |
     *  |  i_zone[6]  |       +----------+
     *  +-------------+       |  blockn  |        +-------------+
     *  |  i_zone[7] -|------>+----------+        |             |
     *  +-------------+                           | buffer_head |
     *  |  i_zone[8] -|---o                       |             |
     *  +-------------+   |                       +-------------+
     *                    |                                     A
     *                    |                                     |
     *                    |                                     |
     *                    |                     +----------+    |
     *                    |                     |  block0 -|----o
     *                    |                     +----------+
     *                    |                     |  block1  |
     *                    |                     +----------+
     *                    |                     |  ......  |
     *                    |                     +----------+
     *                    o-->+----------+      |  blockn  |
     *                        |  block0 -|----->+----------+
     *                        +----------+
     *                        |  block1  |
     *                        +----------+
     *                        |  ......  |
     *                        +----------+
     *                        |  blockn  |
     *                        +----------+
     *
     * The member 'i_zone' is used to points to a speical block or 
     * block-array. What is block and block-array?
     * 
     *  1) block:
     *     This is buffer_head that manage a special buffer. And block
     *     is file buffer between VFS and Disk.
     *
     *  2) block-array
     *     This is array that maintains a series of block-order. And 
     *     block-order also points to a buffer_head. The size of block-array
     *     is BLOCK_SIZE that total 1024. The type of 'block-order' is
     *     'unsigned short', and then each block-array can manage 512 
     *     'block-order'.
     *
     *       NUMBER(block-order) = BLOCK_SIZE / sizeof(unsigned short)
     *
     * The 'minix_inode' has 3 types for 'i_zone'. Each type can maintain
     * a series of block. as follow:
     *
     *  1) i_zone[0] to i_zone[6]
     *     For this type, 'i_zone' directly point a block. total 7 blocks.
     *
     *  2) i_zone[7]
     *     For this type, 'i_zone[7]' points to a block-array, and block-
     *     array contain 512 'block-order', and each 'block-order' points
     *     to a speical buffer_head. total 512 block.
     *
     *  3) i_zone[8]
     *     For this type, 'i_zone[8]' contain 2 times indirect access,
     *     at first, 'i_zone[8]' points to a block-array that contains
     *     512 'block-order', and each 'block-order' also points to a
     *     'block-array'. On second 'block-array', 'block_array' has
     *     manage 512 'block-order', each 'block-order' also points to
     *     a speical buffer. So, the number of buffer that 'i_zone[8]'
     *     managed is:
     *       
     *        blocknr = 512 * 512 
     *
     * So, the number of 'i_zone' managed is:
     * 
     *        blocknr = 7 + 512 + 512 * 512
     */
    offset = 0;
    blocks = offset / BLOCK_SIZE;
    if (blocks <= 7) {
        /*
         * directly access
         * 
         * +-------------+
         * |             |
         * | minix_inode |
         * |             |              +-------------+
         * +-------------+              |             |
         * |             |              | buffer_head |
         * +-------------+  block_order |             |
         * |  i_zone[1] -|------------->+-------------+
         * +-------------+
         * |             |
         * +-------------+
         */
        block_order = inode->i_zone[blocks];
    } else if (blocks > 7 && blocks <= (7 + 512)) {
        /*
         * One indirectly access
         *
         *
         *                                                  +---------------+
         *                                                  |               |
         * +-------------+                block_array       | buffer_header |
         * |             |              +-------------+     |               |
         * | minix_inode |              | block_order-|---->+---------------+
         * |             |              +-------------+
         * +-------------+              | ..........  |
         * |             |              +-------------+
         * +-------------+  block_order | block_order |
         * |  i_zone[7] -|------------->+-------------+
         * +-------------+
         * |             |
         * +-------------+
         *
         */
        block_order = inode->i_zone[7];
        /* indirect access */
        if (!(bh1 = bread(sb->s_dev, block_order, BLOCK_SIZE))) {
            printk(KERN_INFO "Can't read block from Disk on 1st indirect\n");
            return -EINVAL;
        }
        block_array = (unsigned short *)bh1->b_data;
        block_order = block_array[(blocks - 7) % 512];
    } else if (blocks > (7 + 512)) {
        /*
         * Twice indirectly access
         *
         *
         *
         *
         *                                          block_array
         *                                        +-------------+
         *                                        | block_order |
         *                                        +-------------+
         *                                        |   ....      |
         * +-------------+      block_array       +-------------+
         * |             |    +-------------+     | block_order-|---------o
         * | minix_inode |    | block_order-|---->+-------------+         |
         * |             |    +-------------+                             |
         * +-------------+    | ..........  |                             |
         * |             |    +-------------+                             |
         * +-------------+    | block_order |                             |
         * |  i_zone[8] -|--->+-------------+                             |
         * +-------------+                        +-------------+<--------o
         *                                        |             |
         *                                        | buffer_head |
         *                                        |             |    
         *                                        +-------------+
         *          
         *          
         */
        unsigned short orig_blocks = blocks;

        /* 1st indirect access */
        block_order = inode->i_zone[8];
        if (!(bh1 = bread(sb->s_dev, block_order, BLOCK_SIZE))) {
            printk("1st indirect failed\n");
            return -EINVAL;
        }
        block_array = (unsigned short *)bh1->b_data;
        blocks -= 7 + 512;
        block_order = block_array[blocks / 512];
        /* 2th indirect access */
        if (!(bh2 = bread(sb->s_dev, block_order, BLOCK_SIZE))) {
            printk("2th indirect failed\n");
            return -EINVAL;
        }
        block_array = (unsigned short *)bh2->b_data;
        block_order = block_array[(orig_blocks - 7 - 512) % 512];
    }
    if (!(bh = bread(sb->s_dev, block_order, BLOCK_SIZE))) {
        printk(KERN_ERR "Unable read buffer from disk\n");
        return -EINVAL;
    }
    /* Read buffer */
    buffer = (char *)bh->b_data;
    offset %= BLOCK_SIZE;

    printk("Speical: %c\n", buffer[offset]);

    if (bh)
        brelse(bh);
    if (bh1)
        brelse(bh1);
    if (bh2)
        brelse(bh2);
    return 0;
}

/*
 * minix-fs inode
 *  The Minix-fs utilize structure 'minix_inode' to manage file and 
 *  directory. The 'minix_inode' store in Disk. So linux 1.0 utlize
 *  3 layer to manage all inode. 
 *  1) VFS inode
 *     VFS inode is common inode structure that indicates file/directory
 *     information. And VFS inode contain speical inode information on
 *     member 'u' which declare as union to hold special fs inode.
 *
 *  2) Special inode on RAM
 *     These inode manage speical fileystem inode information that
 *     store in RAM not Disk. And this inode is used to acculate to
 *     access inode information.
 *
 *  3) Speical inode on Disk
 *     These inode manage speical filesystem inode information that
 *     store in Disk. And it is uniqure to indecate a file or directory.
 *
 *  So, the relationship of these inode as follow:
 *
 *                      
 *                     struct inode 
 *                     +---------------+
 *                     |               |
 *                     |               |
 *                     |   VFS inode   |
 *                     |               |
 *                     |               |
 *                     +---------------+   (On-RAM)
 *                     |               |
 *                     |               |
 *                     | Speicl inode  |
 *                     |               |
 *                     |               |
 *                     +---------------+
 *                     A 
 *                     | struct minix_inode_info
 *                     |         A
 *                     |         |
 *                     |         V
 *                     | struct minix_inode     
 *                     |      
 *                     V   (On-Disk) 
 *     +------+-----+-------------+----+-------------+-----------+
 *     |      |     |             |    |             |           |
 *     | Boot | ... | minix_inode | .. | minix_inode | data zone | 
 *     |      |     |             |    |             |           |
 *     +------+-----+-------------+----+-------------+-----------+
 *
 */
static int minix_inode(struct inode *inode)
{
    struct minix_inode *raw_inode;
    struct super_block *sb;
    struct buffer_head *bh;
    int ino, block;

    /*
     * Read minix inode from Inode-Table on minix-fs. Inode-Table is behind
     * of Zone-BitMap, so we can obtain inode-table block order as follow:
     *  
     *   block = 2 + s_imap_blocks + s_zmap_blocks
     *
     * And then, Inode-table maintain all minix_inode on these area. We can
     * search speical inode by ino means inode order. The 'ino' hold on 
     * VFS inode named 'i_ino'. On MINIX-FS, the number of minix_inode which
     * each block managed is MINIX_INODES_PER_BLOCK. So in order to find
     * special minix_inode on Inode-table as follow:
     *
     *   minix_inode = block + (ino - 1) / MINIX_INODES_PER_BLOCK 
     */
    ino = inode->i_ino;
    sb = inode->i_sb;
    block = 2 + sb->u.minix_sb.s_imap_blocks +
                sb->u.minix_sb.s_zmap_blocks +
            (ino - 1) / MINIX_INODES_PER_BLOCK;
    printk("Block %d\n", block);
    if (!(bh = bread(inode->i_dev, block, BLOCK_SIZE))) {
        printk(KERN_ERR "Major problem, unable to read inode from disk\n");
        return -EINVAL;
    }
    raw_inode = ((struct minix_inode *)bh->b_data) +
                  (ino - 1) % MINIX_INODES_PER_BLOCK;

#ifdef CONFIG_DEBUG_MINIX_INODE
    minixfs_minix_inode(raw_inode, sb);
#endif

    if (bh)
        brelse(bh);
    return 0;
}

asmlinkage int sys_demo_minixfs(int fd)
{
    struct inode *inode;    
    struct file *filp;
    struct super_block *sb;

    /* get file descriptor from current task */
    filp = current->filp[fd];
    if (!filp) {
        printk("Unable to open minixfs inode.\n");
        return -EINVAL;
    }

    /* get special inode */
    inode = filp->f_inode;
    if (!inode) {
        printk(KERN_ERR "Invalid inode.\n");
        return -EINVAL;
    }

    /* get special super block */
    sb = inode->i_sb;
    if (!sb) {
        printk(KERN_ERR "Invalid super block.\n");
        return -EINVAL;
    }

    /* Parse MINIX Filsystem layout */
    minix_layout(inode, sb);
    /* MINIX-FS inode */
    minix_inode(inode);
 
    return 0;
}

/* System call entry */
inline _syscall1(int, demo_minixfs, int, fd);

static int debug_minixfs(void)
{
    int fd;

    fd = open("/etc/rc", O_RDONLY, 0);
    if (fd < 0) {
        printk("Unable to read /etc/rc\n");
        return -1;
    }

    demo_minixfs(fd);

    close(fd);

    return 0;
}
user1_debugcall_sync(debug_minixfs);
