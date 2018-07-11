/*
 * POSIX Open on MINIX-FS
 *
 * (C) 2018.07.11 BiscuitOS <buddy.zhang@aliyun.com>
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

#define clear_block(addr) \
    __asm__("cld\n\t" \
            "rep\n\t" \
            "stosl" \
            : \
            : "a" (0), "c" (BLOCK_SIZE/4), "D" ((long)(addr)))

#define find_first_zero(addr) ({ \
    int __res; \
    __asm__("cld\n" \
            "1:\tlodsl\n\t" \
            "notl %%eax\n\t" \
            "bsfl %%eax, %%edx\n\t" \
            "jne 2f\n\t" \
            "addl $32, %%ecx\n\t" \
            "cmpl $8192,%%ecx\n\t" \
            "jl 1b\n\t" \
            "xorl %%edx,%%edx\n" \
            "2:\taddl %%edx,%%ecx" \
            :"=c" (__res):"0" (0), "S" (addr)); \
    __res;})


#define ACC_MODE(x) ("\000\004\002\006"[(x)&O_ACCMODE])

/*
 * Allocate new block to inode.
 *  On MINIX-FS, a new block is a data zone. In order to allocate new
 *  data zone, we should find a empty zone from Zone-BitMap. 
 *
 *  Note! Zone-BitMap is used to manage all data zone, data zone is belong 
 *  to zone on MINIX-FS. The MINIX-FS divide filesystem into a unit, and 
 *  the size of unit is BLOCK_SIZE. The zone contains Reserved zone and
 *  data zone.
 *
 *  1) Reserved Zone
 *     Reserved zone is used to store MINIX-FS information, such as boot
 *     block, super block, zone-bitmap, inode-bitmap and inode-table.
 *  2) Data Zone
 *     Data zone is used to store data of inode.
 * 
 *  Note! The Zone-Bitmap is used to manage all data zone not contain 
 *  Reserved zone. And 's_firstdatazone' points to first data zone.
 */
static int minix_new_blocks(struct super_block *sb)
{
    struct buffer_head *bh;
    int i, j;

    if (!sb) {
        printk("trying to get new block from nonexistent device\n");
        return 0;
    }
repeat:
    /* Each buffer contain a data unit, the size of unit is BLOCK_SIZE,
     * The BLOCK_SIZE is 1KByte, so that contains:
     *
     *     1024 * 8 bit => 8192 bit.
     */
    j = 8192;
    for (i = 0; i < 8; i++)
        if ((bh = sb->u.minix_sb.s_zmap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    if (i >= 0 || !bh || j >= 8192)
        return 0;
    if (set_bit(j, bh->b_data)) {
        printk("new_block: bit already set\n");
        goto repeat;
    }
    bh->b_dirt = 1;
    /*
     * +------+------------+-----+-------------+----------------------+
     * |      |            |     |             |                      |
     * | Boot | Superblock | ... | Inode-Table | Data zone            |
     * |      |            |     |             |                      |
     * +------+------------+-----+-------------+----------------------+
     *                                         A
     *                                         |
     *                                         |
     *                                         |
     *                  s_firstdatazone--------o
     *
     */
    j += i * 8192 + sb->u.minix_sb.s_firstdatazone - 1;
    if (j < sb->u.minix_sb.s_firstdatazone ||
                  j >= sb->u.minix_sb.s_nzones)
        return 0;
    if (!(bh = getblk(sb->s_dev, j, BLOCK_SIZE))) {
        printk("new_block: cannot get block\n");
        return 0;
    }
    /* Clear contents of buffer */
    clear_block(bh->b_data);
    bh->b_uptodate = 1;
    bh->b_dirt = 1;
    brelse(bh);
    return j;
}

static void minix_free_blocks(struct super_block *sb, int block)
{
    struct buffer_head *bh;
    unsigned int bit, zone;

    if (!sb) {
        printk("trying to free block on nonexistent device\n");
        return;
    }
    if (block < sb->u.minix_sb.s_firstdatazone ||
              block >= sb->u.minix_sb.s_nzones) {
        printk("trying to free block not in datazone\n");
        return;
    }
    bh = get_hash_table(sb->s_dev, block, BLOCK_SIZE);
    if (bh)
        bh->b_dirt = 0;
    brelse(bh);
    zone = block - sb->u.minix_sb.s_firstdatazone + 1;
    bit = zone & 8192;
    zone >>= 13;
    bh = sb->u.minix_sb.s_zmap[zone];
    if (!bh) {
        printk("minix_free_block: nonexistent bitmap buffer\n");
        return;
    }
    if (!clear_bit(bit, bh->b_data))
        printk("free_block (%04x:%d): bit already cleard\n", 
                                    sb->s_dev, block);
    bh->b_dirt = 1;
    return;
}

/*
 * On MINIX-FS directly access buffer
 *  The inode manages all block nr on 'inode->u.minix_i.i_data[x]'.
 *  and then read data from disk to buffer via 'bread'.
 *
 *  +-----------+
 *  |           |
 *  |   inode   |
 *  |           |                       +-----------------+
 *  +-----------+                       |                 |
 *  | u.minix_i |                       |   buffer_head   |
 *  |           |                       |                 |
 *  | i_data[0]-|-------> block_nr----->+-----------------+
 *  |           |
 *  | ....      |
 *  |           |
 *  |i_data[15] |
 *  +-----------+
 *
 */
static struct buffer_head *inode_getblks(struct inode *inode, int nr,
                      int create)
{
    int tmp;
    unsigned short *p;
    struct buffer_head *result;

    p = inode->u.minix_i.i_data + nr;
repeat:
    tmp = *p; /* block nr on MINIX-FS data zone */
    if (tmp) {
        result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
        if (tmp == *p)
            return result;
        brelse(result);
        goto repeat;
    }
    if (!create)
        return NULL;
    /* Allocate new block to minix_inode */
    tmp = minix_new_blocks(inode->i_sb);
    if (!tmp)
        return 0;
    result = getblk(inode->i_dev, tmp, BLOCK_SIZE);
    if (*p) {
        minix_free_blocks(inode->i_sb, tmp);
        brelse(result);
        goto repeat;
    }
    *p = tmp;
    inode->i_ctime = CURRENT_TIME;
    inode->i_dirt  = 1;
    return result;
}

/*
 * block_getblks
 *  Obtain block nr from a block. For this way, MINIX-FS will one or two
 *  indirectly access special block nr, as figure:
 *
 * One indirectly access
 *
 *
 *                                                  +-------------+
 *                                                  |             |
 * +-------------+                block_array       | buffer_head |
 * |             |              +-------------+     |             |
 * | minix_inode |              | block_order-|---->+-------------+
 * |             |              +-------------+
 * +-------------+              | ..........  |
 * |             |              +-------------+
 * +-------------+  block_order | block_order |
 * |  i_zone[7] -|------------->+-------------+
 * +-------------+
 * |             |
 * +-------------+
 *
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
 * 
 */
static struct buffer_head *block_getblks(struct inode * inode,
     struct buffer_head *bh, int nr, int create)
{
    int tmp;
    unsigned short *p;
    struct buffer_head *result;

    if (!bh)
        return NULL;
    if (!bh->b_uptodate) {
        ll_rw_block(READ, 1, &bh);
        wait_on_buffer(bh);
        if (!bh->b_uptodate) {
            brelse(bh);
            return NULL;
        }
    }
    p = nr + (unsigned short *)bh->b_data;
repeat:
    tmp = *p;
    if (tmp) {
        result = getblk(bh->b_dev, tmp, BLOCK_SIZE);
        if (tmp == *p) {
            brelse(bh);
            return result;
        }
        brelse(result);
        goto repeat;
    }
    if (!create) {
        brelse(bh);
        return NULL;
    }
    tmp = minix_new_blocks(inode->i_sb);
    if (!tmp) {
        brelse(bh);
        return NULL;
    }
    result = getblk(bh->b_dev, tmp, BLOCK_SIZE);
    if (*p) {
        minix_free_blocks(inode->i_sb, tmp);
        brelse(result);
        goto repeat;
    }
    *p = tmp;
    bh->b_dirt = 1;
    brelse(bh);
    return result;
}

/*
 * Read Inode buffer
 *  On MINIX-FS, each inode manage a series of data zones. Minix-inode
 *  contains 3 type data zone.
 *   
 *    1) Directly access
 *       The block nr is smaller then 7, inode can access buffer directly.
 *
 *    2) One indirect access
 *       The block nr is 7 to 512, and inode should access buffer indirectly 
 *       by once.
 *
 *    3) Two indirect access
 *       The block nr is big then 512, and inode should access buffer 
 *       indirectly by twice.
 * 
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
 */
static struct buffer_head *minix_getblks(struct inode *inode, int block,
         int create)
{
    struct buffer_head *bh;

    if (block < 0) {
        printk("minix_getblk: block < 0");
        return 0;
    }
    if (block >= 7 + 512 + 512 * 512) {
        printk("minix_getblk: block > big\n");
        return NULL;
    }
    /* Directly Access */
    if (block < 7)
        return inode_getblks(inode, block, create);
    /* One indirect access */
    block -= 7;
    if (block < 512) {
        /* 7th block contains alll one indirect access block nr */
        bh = inode_getblks(inode, 7, create);
        return block_getblks(inode, bh, block, create);
    }
    /* Two indirect access */
    block -= 512;
    bh = inode_getblks(inode, 8, create);
    /* 8th block contains all one indirect access block nr */
    bh = block_getblks(inode, bh, block >> 9, create);
    /* Another block contains all two indirect access block nr */
    return block_getblks(inode, bh, block & 511, create);
}

/*
 * bread_minix()
 *  Read a BLOCK from minixfs.
 */
static struct buffer_head *bread_minix(struct inode *inode, int block,
                        int create)
{
    struct buffer_head *bh;

    bh = minix_getblks(inode, block, create);
    if (!bh || bh->b_uptodate)
        return bh;
    ll_rw_block(READ, 1, &bh);
    wait_on_buffer(bh);
    if (bh->b_uptodate)
        return bh;
    brelse(bh);
    return NULL;
}

static inline int namecompare(int len, int maxlen, 
            const char *name, const char *buffer)
{
    /*
     * If name matchs buffer, and 'buffer[len]' must be zero!
     */
    if (len >= maxlen || !buffer[len]) {
        unsigned char same;

        __asm__ ("repe ; cmpsb ; setz %0"
                 : "=q" (same)
                 : "S" ((long) name), "D" ((long) buffer), "c" (len));
        return same;
    }
    return 0;
}

/*
 * ok, we cannot use strncmp, as the name is not in our data space.
 * Thus we'll have to use 'minix_match'. No big problem. Match also makes
 * some sanity tests.
 *
 * NOTE! unlike strncmp, minix_match returns 1 for success, 0 for failure.
 */
static int minix_match(int len, const char *name,
          struct buffer_head *bh, unsigned long *offset,
          struct minix_sb_info *info)
{
    /*
     * The 'minix_dir_entry' is used to manage a entry for special inode.
     * It's defined in 'include/linux/minix_fs.h', as follow:
     * 
     *   struct minix_dir_entry {
     *       unsigned short inode;
     *       char name[0];
     *   }
     *
     * The 'inode' points a special inode structure, and 'name' is a empty
     * array but it is start address of name. 'name' is used to point a
     * data zera that cotinuely connect after 'name'. 
     *
     *
     * | <------------- s_namelen -------------> |
     * 0-------2---------------------------------+
     * |       |                                 |
     * | inode | name (unknow length)            |
     * |       |                                 |
     * +-------+---------------------------------+
     *         A
     *         |
     *         |
     *         o---- start address of name string
     * 
     * sizeof(struct minix_dir_entry) = 2
     */
    struct minix_dir_entry *de;

    de = (struct minix_dir_entry *)(bh->b_data + *offset);
    /* Points next minix_dir_entry */
    *offset += info->s_dirsize;
    if (!de->inode || len > info->s_namelen)
        return 0;
    /* "" means "." ---> so paths like "/usr/lib//libc.a" work */
    if (!len && (de->name[0] == '.') && (de->name[1] == '\0'))
        return 1;
    return namecompare(len, info->s_namelen, name, de->name);
}

/*
 * minix_find_entry()
 *  finds an entry in the specified directory with the wanted name. It
 *  returns the cache buffer in which the entry was found, and the entry
 *  itself (as a parameter - res_dir). It does NOT read the inode of the
 *  entry - you'll have to do that yourself if you want to.
 */
static struct buffer_head *find_entry_minix(struct inode *dir,
      const char *name, int namelen, struct minix_dir_entry **res_dir)
{
    unsigned long block, offset;
    struct buffer_head *bh;
    struct minix_sb_info *info;

    *res_dir = NULL;
    /* verify direntory and superblock whether exist? */
    if (!dir || !dir->i_sb)
        return NULL;
    /* Obtain minix_sb from super block */
    info = &dir->i_sb->u.minix_sb;
    /*
     * On MINIX-FS, minix_sb->s_namelen indicate naximum for 
     * name. If the length of name is big than 's_namelen' and doesn't
     * support NAME_TRUNCATE feature, the 'find_entry_minix' will return 
     * Null. 
     */
    if (namelen > info->s_namelen) {
#ifdef NO_TRUNCATE
        return NULL;
#else
        namelen = info->s_namelen;
#endif
    }
    bh = NULL;
    block = offset = 0;
    while (block * BLOCK_SIZE + offset < dir->i_size) {
        if (!bh) {
            /* Read data block for inode */
            bh = bread_minix(dir, block, 0);
            if (!bh) {
                block++;
                continue;
            }
        }
        /*
         * The 'minix_bread' read special block from MINIX Filesystem.
         * A special 'buffer_head' manage a buffer and 'b_data' points to
         * data zera of buffer. The data zera of direntry inode contains
         * a lot of 'minix_dir_entry' that describes the member information.
         *
         * 
         * +-------------------------------------+
         * |                                     |
         * |         struct buffer_head          |
         * |                                     |
         * +-------------------------------------+ 
         *                   |
         *                   |
         *                   |
         *     b_data        |
         * o-----------------o
         * |
         * |
         * |
         * V                 
         * 0-----------------+-----------------+----+-----------------+
         * |                 |                 |    |                 |
         * | minix_dir_entry | minix_dir_entry | .. | minix_dir_entry |
         * |                 |                 |    |                 |
         * +-----------------+-----------------+----+-----------------+
         * A <-- offset0 --> | <-- offset1 --> |    | <-- offsetn --> |
         * |
         * |
         * o-----res_dir
         *
         */
        *res_dir = (struct minix_dir_entry *)(bh->b_data + offset);
        if (minix_match(namelen, name, bh, &offset, info))
            return bh;
        /* Continue search in current block */
        if (offset < bh->b_size)
            continue;
        /* Search on next block */
        brelse(bh);
        bh = NULL;
        offset = 0;
        block++;
    }
    brelse(bh);
    *res_dir = NULL;
    return NULL;
}

/*
 * Search subdir on a directory
 *  MINIX-FS utilizes "minix_dir_entry" to describe a minix-directory. If
 *  the mode of inode is a direcotry and it will contain "minix_dir_entry"
 *  on data zone as follow:
 *
 *
 *  +-----------+
 *  |           |            +--------------+--------+----+
 *  |   inode   |            |              |        |    |
 *  |           |            |  buffer_head | b_data | .. |
 *  +-----------+            |              |        |    |
 *  | i_data[0] |----------->+--------------+--------+----+
 *  +-----------+                                |
 *  | ....      |                                | 
 *  +-----------+                                |
 *  | i_data[16]|                                |
 *  +-----------+                                |
 *                                               |
 *                                               V
 *                       +-----------------+-----+-----------------+------+
 *                       |                 |     |                 |      |
 *                       | minix_dir_entry | ... | minix_dir_entry | hole |
 *                       |                 |     |                 |      |
 *                       +-----------------+-----+-----------------+------+
 *
 *
 *
 */
int lookup_minix(struct inode *dir, const char *name, 
                    int len, struct inode **result)
{
    int ino;
    struct minix_dir_entry *de;
    struct buffer_head *bh;

    *result = NULL;
    if (!dir)
        return -ENOENT;
    /* The mode of inode must be directory */
    if (!S_ISDIR(dir->i_mode)) {
        iput(dir);
        return -ENOENT;
    }
    /* find enty on dirent inode */
    if (!(bh = find_entry_minix(dir, name, len, &de))) {
        iput(dir);
        return -ENOENT;
    }
    ino = de->inode;
    brelse(bh);
    /* Find special inode */
    if (!(*result = iget(dir->i_sb, ino))) {
        iput(dir);
        return -EACCES;
    }
    iput(dir);
    return 0;
}

/*
 *     minix_add_entry()
 *
 * adds a file entry to the specified directory, returning a possible
 * error value if it fails.
 *
 * NOTE!! The inode part of 'de' is left at 0 - which means you
 * may not sleep between calling this and putting something into
 * the entry, as someone else might have used it while you slept.
 */
static int minix_add_entry(struct inode *dir, const char *name,
      int namelen, struct buffer_head **res_buf,
      struct minix_dir_entry **res_dir)
{
    int i;
    unsigned long block, offset;
    struct buffer_head *bh;
    struct minix_dir_entry *de;
    struct minix_sb_info *info;

    *res_buf = NULL;
    *res_dir = NULL;
    if (!dir || !dir->i_sb)
        return -ENOENT;
    /* MINIX-FS super block information */
    info = &dir->i_sb->u.minix_sb;
    if (namelen > info->s_namelen) {
#ifdef NO_TRUNCATE
        return -ENAMETOOLONG;
#else
        namelen = info->s_namelen;
#endif
    }
    if (!namelen)
        return -ENOENT;
    bh = NULL;
    block = offset = 0;
    while (1) {
        if (!bh) {
            bh = bread_minix(dir, block, 1);
            if (!bh)
                return -ENOSPC;
        }
        de = (struct minix_dir_entry *)(bh->b_data + offset);
        offset += info->s_dirsize;
        /* Verify whether inode is over dir */
        if (block * bh->b_size + offset > dir->i_size) {
            de->inode = 0;
            dir->i_size = block * bh->b_size + offset;
            dir->i_dirt = 1;
        }
        if (de->inode) {
            /* Same name on direntory */
            if (namecompare(namelen, info->s_namelen, name, de->name)) {
                brelse(bh);
                return -EEXIST;
            }
        } else {
            /* Find a empty minix_dir_entry and initialize it. */
            dir->i_mtime = dir->i_ctime = CURRENT_TIME;
            for (i = 0; i < info->s_namelen; i++)
                de->name[i] = (i < namelen) ? name[i] : 0;
            bh->b_dirt = 1;
            *res_dir = de;
            break;
        }
        if (offset < bh->b_size)
            continue;
        brelse(bh);
        bh = NULL;
        offset = 0;
        block++;
    }
    *res_buf = bh;
    return 0;
}

/*
 * new_inode_minix
 *  Create a minix_inode.
 */
static struct inode *new_inode_minix(const struct inode *dir)
{
    struct super_block *sb;
    struct inode *inode;
    struct buffer_head *bh;
    int i, j;

    /* Allocate a new inode from VFS */
    if (!dir || !(inode = get_empty_inode()))
        return NULL;
    sb = dir->i_sb;
    inode->i_sb = sb;
    inode->i_flags = inode->i_sb->s_flags;
    /*
     * Allocate a minix-inode from MINIX-FS. The Inode-BitMap is used to 
     * track a minix_inode whether used or unused. The new_inde_minix()
     * serch a zero bit on Inode-BitMap. and then 'Inode-Table' manages
     * all minix-inode, MINIX-FS utilize ino that from Inode-BitMap to 
     * obtain special minix_inode.
     *
     * +------+------------+--------------+----+-------------+-----------+ 
     * |      |            |              |    |             |           |
     * | Boot | Superblock | Inode-BitMap | .. | Inode-Table | Data Zone |
     * |      |            |              |    |             |           |
     * +------+------------+--------------+----+-------------+-----------+ 
     * 
     * The Inode-BitMap is comprised of a lot of Zone, on MINIX-FS, the size
     * of Zone is BLOCK_SIZE, it is 1KByte. So the number of inode for a Zone
     * is:
     *       number = 1024 * 8 bit = 8192 bit
     * So, each zone can represents 8192 minix-inode. And then we can find 
     * a zero bit as empty 'inode nr'. We can find a minix-inode in 
     * 'inode nr' on Inode-Table.
     * 
     */
    j = 8192;
    for (i = 0; i < 8; i++)
        if ((bh = inode->i_sb->u.minix_sb.s_imap[i]) != NULL)
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    if (!bh || j >= 8192) {
        iput(inode);
        return NULL;
    }
    /* Mark as used minix-inode */
    if (set_bit(j, bh->b_data)) {  /* shouldn't happen */
        printk("new_inode: bit already set");
        iput(inode);
        return NULL;
    }
    bh->b_dirt = 1;
    j += i * 8192;
    if (!j || j >= inode->i_sb->u.minix_sb.s_ninodes) {
        iput(inode);
        return NULL;
    }
    inode->i_count = 1;
    inode->i_nlink = 1;
    inode->i_dev = sb->s_dev;
    inode->i_uid = current->euid;
    inode->i_gid = (dir->i_mode & S_ISGID) ? dir->i_gid : current->egid;
    inode->i_dirt = 1;
    inode->i_ino  = j; /* setup inode nr */
    inode->i_mtime = inode->i_atime = inode->i_ctime = CURRENT_TIME;
    inode->i_op = NULL;
    inode->i_blocks = inode->i_blksize = 0;
    /* Insert inode into Inode Hash_Table */
    insert_inode_hash(inode);
    return inode;
}

/*
 * create_minix
 *  Create a new inode and add into current directory.
 */
int create_minix(struct inode *dir, const char *name,
                    int len, int mode, struct inode **result)
{
    int error;
    struct inode *inode;
    struct buffer_head *bh;
    struct minix_dir_entry *de;

    *result = NULL;
    if (!dir)
        return -ENOENT;
    /* Obtain a inode that contain minix-inode information */
    inode = new_inode_minix(dir);
    if (!inode) {
        iput(dir);
        return -ENOSPC;
    }
    inode->i_op = &minix_file_inode_operations;
    inode->i_mode = mode;
    inode->i_dirt = 1;
    /* Insert inode into minix directory */
    error = minix_add_entry(dir, name, len, &bh, &de);
    if (error) {
        inode->i_nlink--;
        inode->i_dirt = 1;
        iput(inode);
        iput(dir);
        return error;
    }
    de->inode = inode->i_ino;
    bh->b_dirt = 1;
    brelse(bh);
    iput(dir);
    *result = inode;
    return 0;
}
