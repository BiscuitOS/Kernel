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

/*
 * Boot block
 *  Boot block is first part on MINIX Filesystem. It consists of 
 *  boot code, Reserved for system. The Boot block contain 1 block.
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
 *   | 0x14   | s_long_zone_size:                                     |
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
static int minixfs_super_block(struct super_block *sb)
{
    struct minix_super_block *ms;
    struct buffer_head *bh = NULL;
    int imap_blocks;

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
     * s_ninodes:
     *  It indicates the numbe of inode for minix-fs. It often used to
     *  calculate the block number for Inode BitMap. Each block contain
     *  1024 * 8 bits.
     *
     *   s_imap_blocks = (s_ninodes + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
     *
     *  Align with BLOCK_SIZE. 
     */
    imap_blocks = (ms->s_ninodes + BIT_PER_BLOCK - 1) / BIT_PER_BLOCK;
    if (imap_blocks != ms->s_imap_blocks) {
        panic("Un-Alignment for s_imap_blocks");
    }
   
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
    struct buffer_head *bh;
    struct minix_super_block *ms;

    /* Read super block from Disk */
    if (!(bh = bread(inode->i_dev, SUPER_BLOCK, BLOCK_SIZE))) {
        printk(KERN_ERR "Unable obtain minix fs super block.\n");
        return -EINVAL;
    }

    ms = (struct minix_super_block *)bh->b_data;
    sb->u.minix_sb.s_ms = ms;
    sb->s_dev = inode->i_dev;

#ifdef CONFIG_DEBUG_BOOT_BLOCK
    /* Parse Boot Block on MINIXFS */
    minixfs_boot_block(sb);
#endif

#ifdef CONFIG_DEBUG_SUPER_BLOCK
    /* Parse Super Block on MINIXFS */
    minixfs_super_block(sb);
#endif
    return 0;
}

asmlinkage int sys_demo_minixfs(int fd)
{
    struct inode *inode;    
    struct file *filp;
    struct super_block sb;

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

    /* Parse MINIX Filsystem layout */
    minix_layout(inode, &sb);
 
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
