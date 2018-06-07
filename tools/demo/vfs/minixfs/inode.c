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
#include <linux/fs.h>
#include <linux/hdreg.h>
#include <linux/mm.h>

#include <sys/stat.h>

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
 */

/*
 * Inode MODE
 *
 *   i_mode is used to store file type and access permission attribute.
 *   And i_mode[15:12] is used to store file type, i_mode[11:9] is used
 *   to hold setting information while it is modified. i_mode[8:0] is used
 *   to hold access permission. e.g. figure. More information see 
 *   'include/sys/stat.h'
 *
 *   15--------12-11------------------9-8---------------------0
 *   | file type | setting information | access permision     |
 *   ----------------------------------------------------------
 *
 *   i_mode[15:12] File Type
 *   ----------------------------------------------------------
 *   | Value      | Describe                                  |
 *   ----------------------------------------------------------
 *   | 000        |                                           |
 *   ---------------------------------------------------------- 
 *   | 001        | FIFO/Named Pipe                           |
 *   ---------------------------------------------------------- 
 *   | 010        | Character Device                          |
 *   ---------------------------------------------------------- 
 *   | 011        |                                           |
 *   ---------------------------------------------------------- 
 *   | 100        | Dirent                                    |
 *   ---------------------------------------------------------- 
 *   | 101        |                                           |
 *   ---------------------------------------------------------- 
 *   | 110        | Block Device                              |
 *   ---------------------------------------------------------- 
 *   | 111        | Regular File                              |
 *   ---------------------------------------------------------- 
 *    
 *   i_mode[11:9] Setting inforamtion
 *   ----------------------------------------------------------
 *   | Value      |                                           |
 *   ----------------------------------------------------------
 *   | 001        | set-user-ID                               |
 *   ----------------------------------------------------------
 *   | 010        | set-group-ID                              |
 *   ----------------------------------------------------------
 *   | 100        | Constrained delete flag (only for Dirent) |
 *   ----------------------------------------------------------
 *
 *   i_mode[8:6] User Access Permission
 *   ----------------------------------------------------------
 *   | Value      |                                           |
 *   ----------------------------------------------------------
 *   | R [8]      | Read permision                            |
 *   ----------------------------------------------------------
 *   | W [7]      | Write permision                           |
 *   ----------------------------------------------------------
 *   | X [6]      | Execute permision                         |
 *   ----------------------------------------------------------
 *
 *   i_mode[5:3] Group Access Permission
 *   ----------------------------------------------------------
 *   | Value      |                                           |
 *   ----------------------------------------------------------
 *   | R [5]      | Read permision                            |
 *   ----------------------------------------------------------
 *   | W [4]      | Write permision                           |
 *   ----------------------------------------------------------
 *   | X [3]      | Execute permision                         |
 *   ----------------------------------------------------------
 *
 *   i_mode[2:0] User Access Permission
 *   ----------------------------------------------------------
 *   | Value      |                                           |
 *   ----------------------------------------------------------
 *   | R [2]      | Read permision                            |
 *   ----------------------------------------------------------
 *   | W [1]      | Write permision                           |
 *   ----------------------------------------------------------
 *   | X [0]      | Execute permision                         |
 *   ----------------------------------------------------------
 *
 */
static void parse_inode_mode(struct m_inode *inode)
{
    int mode = inode->i_mode;

    if (S_ISREG(mode))
        printk("File Type: Regular file\n");
    else if (S_ISDIR(mode))
        printk("File Type: Directory\n");
    else if (S_ISCHR(mode))
        printk("File Type: Character Device\n");
    else if (S_ISBLK(mode))
        printk("File Type: Block Device\n");
    else if (S_ISFIFO(mode))
        printk("File Type: FIFO/Named Pipe\n");
    else if (S_ISSOCK(mode))
        printk("File Type: Socket\n");
    else if (S_ISLNK(mode))
        printk("File Type: Symoble link\n");
    else
        printk("File Type: Unknown type\n");


    /* Setting information upon execution */
    if (mode & S_ISUID)
        printk("Set Info:  Set user id upon execution\n");
    if (mode & S_ISGID)
        printk("Set Info:  Set group id upon execution\n");
    if (mode & S_ISVTX)
        printk("Set Info:  Sticky bit\n");

    /* User Access Permission */
    printk("User  Permission: ");
    if (mode & S_IRUSR)
        printk("R ");
    if (mode & S_IWUSR)
        printk("W ");
    if (mode & S_IXUSR)
        printk("X ");

    /* Group Access Permission */
    printk("\nGroup Permission: ");
    if (mode & S_IRGRP)
        printk("R ");
    if (mode & S_IWGRP)
        printk("W ");
    if (mode & S_IXGRP)
        printk("X ");

    /* World/Other Permission */
    printk("\nOther Permission: ");
    if (mode & S_IROTH)
        printk("R ");
    if (mode & S_IWOTH)
        printk("W ");
    if (mode & S_IXOTH)
        printk("X ");
    printk("\n");
}

/*
 * Parse Inode information from MINIXFS.
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
static void parse_inode_info(struct m_inode *inode)
{
    printk("=============INODE============\n");
    /* Inode mode */
    parse_inode_mode(inode);
    /* UserID */
    printk("UserID:           %#x\n", inode->i_uid);
    /* Length of file */
    printk("Length:           %#x\n", inode->i_size);
    /* Modify time (from 1970.1.1:0)*/
    printk("Modify Time:      %#x\n", inode->i_mtime);
    /* Group ID */
    printk("GroupID:          %#x\n", inode->i_gid);
    /* Links number */
    printk("Link numbers:     %#x\n", inode->i_nlinks);
    printk("==============================\n");
}

/*
 * Load special inode from MINIXFS
 *
 *   Inode locate in Inode block that contain all inode structure,
 *    
 */
static struct m_inode *read_inode(int root, int ino)
{
    struct super_block *sb;
    struct buffer_head *bh;
    struct m_inode *inode = NULL;
    int block;

    /* Obtain super block */
    if(!(sb = get_super(root)))
        panic("trying read super block");

    /* compute the block */
    block  = 2 + sb->s_imap_blocks + sb->s_zmap_blocks;
    block += (ino - 1) / INODES_PER_BLOCK;
    printk("block %#x\n", block);

    /* load logical block from disk */
    if (!(bh = bread(root, block)))
        panic("trying read inode block");

    inode = (struct m_inode *)get_free_page();
    /* Obtain special inode */
    *(struct d_inode *)inode = ((struct d_inode *)bh->b_data)
                               [(ino - 1) % INODES_PER_BLOCK];
    brelse(bh);
    return inode;
}


int debug_vfs_minixfs_inode_userland(void)
{
    if (1) {
        struct m_inode *inode;

        inode = read_inode(ROOT_DEV, ROOT_INO);
        parse_inode_info(inode);
        free_page((unsigned long)inode);
    } else {
        struct m_inode *inode;

        inode = read_inode(ROOT_DEV, ROOT_INO);
        parse_inode_info(inode);
        free_page((unsigned long)inode);
    }
    return 0;
}
