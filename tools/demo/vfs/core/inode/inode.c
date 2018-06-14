/*
 * Inode: Describe a filesystem object file or dirent.
 *
 * (C) 2018.06.07 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/stat.h>

#ifdef CONFIG_MINIX_FS
#include <linux/minix_fs.h>
#endif

#include <demo/debug.h>

/*
 * Hash for inode
 *   Inode use "Separate chaining with list head cells" hash table to 
 *   store inode as follow figure.
 *
 * +-------+-------+----------+-------+------------------+-------+
 * |       |       |          |       |                  |       |
 * | entry | entry | ...      | entry | ...              | entry |
 * |       |       |          |       |                  |       |
 * +-------+-------+----------+-------+------------------+-------+
 *                                |
 *                                |
 *      o-------------------------o
 *      |
 *      |
 *      o-->+-------+   i_hash_prev   +-------+
 *          |       |<----------------|-       |
 *          | inode |                 | inode |
 *          |      -|---------------->|       |  
 *          +-------+   i_hash_next   +-------+
 *
 *
 */
static struct inode_hash_entry {
    struct inode *inode;
    int updating;
} hash_table[NR_IHASH];

static int nr_inodes = 0, nr_free_inodes = 0;
static struct wait_queue *inode_wait = NULL;
static struct inode *first_inode;
static void write_inode(struct inode *inode);

static inline int const hashfn(dev_t dev, unsigned int i)
{
    return (dev ^ i) % NR_IHASH;
}

static inline struct inode_hash_entry * const hash(dev_t dev, int i)
{
    return hash_table + hashfn(dev, i);
}

/*
 * The "new" scheduling parimitives (new as of 0.97 or so) allow this to
 * be done without disabling interrupts (other than in the actual queue
 * updating things: only a couple of 386 instruction). This should be 
 * much better for interrupt latency.
 */
static void __wait_on_inode(struct inode *inode)
{
    struct wait_queue wait = { current, NULL};

    add_wait_queue(&inode->i_wait, &wait);
repeat:
    current->state = TASK_UNINTERRUPTIBLE;
    if (inode->i_lock) {
        schedule();
        goto repeat;
    }
    remove_wait_queue(&inode->i_wait, &wait);
    current->state = TASK_RUNNING;
}

static inline void wait_on_inode(struct inode *inode)
{
    if (inode->i_lock)
        __wait_on_inode(inode);
}

static inline void lock_inode(struct inode *inode)
{
    wait_on_inode(inode);
    inode->i_lock = 1;
}

static inline void unlock_inode(struct inode *inode)
{
    inode->i_lock = 0;
    wake_up(&inode->i_wait);
}

/*
 * Remove inode from hash table
 *
 * +---------+
 * |         |
 * |         |
 * |         |           +-------+  i_hash_next  +-------+
 * +---------+           |      -|-------------->|      -|--> ...
 * |         |           | inode |               | inode |
 * |  Entry  |   inode   |       |<--------------|-      |<-- ...
 * |        -|---------->+-------+  i_hash_prev  +-------+
 * +---------+        
 * |         |
 * |         |
 * |         |
 * +---------+
 *         
 */
static void remove_inode_hash(struct inode *inode)
{
    struct inode_hash_entry *h;

    h = hash(inode->i_dev, inode->i_ino);

    if (h->inode == inode)
        h->inode = inode->i_hash_next;
    if (inode->i_hash_next)
        inode->i_hash_next->i_hash_prev = inode->i_hash_prev;
    if (inode->i_hash_prev)
        inode->i_hash_prev->i_hash_next = inode->i_hash_next;
    inode->i_hash_prev = inode->i_hash_next = NULL;
}

/*
 * remove_inode_free()
 *   Remove inode from free list. The head of free list is "first_inode".
 *   It manage a double linked list to hold all free inodes. The member 
 *   'i_next' and 'i_prev' points the next and prev free inode. If you
 *   want to remove a special inode from free list, you should consider
 *   inode that points 'first_inode', 'i_next' and 'i_prev'.
 *
 *
 *
 *                  +-------+  i_next   +-------+
 *                  |      -|---------->|      -|--> ..
 *                  | inode |           | inode |
 *                  |       |<----------|       |<-- ..
 *   first_inode--->+-------+  i_prev   +-------+
 *
 * 
 */
static void remove_inode_free(struct inode *inode)
{
    if (first_inode == inode)
        first_inode = inode->i_next;
    if (inode->i_next)
        inode->i_next->i_prev = inode->i_prev;
    if (inode->i_prev)
        inode->i_prev->i_next = inode->i_next;
    inode->i_next = inode->i_prev = NULL;
}

/*
 * Insert_inode_free()
 *   Insert inode into free list. The head of free list is "first_inode".
 *   It manage a double linked list to hold all free inodes. The member 
 *   'i_next' and 'i_prev' points the next and prev free inode. If you
 *   want to insert a special inode into free list, you should consider
 *   inode that points 'first_inode', 'i_next' and 'i_prev'.
 *
 *
 *
 *                  +-------+  i_next   +-------+
 *                  |      -|---------->|      -|--> ..
 *                  | inode |           | inode |
 *                  |       |<----------|       |<-- ..
 *   first_inode--->+-------+  i_prev   +-------+
 *
 *
 *                  +-------+ i_next
 *                  |      -|------->
 *           i_prev | inode |
 *         <--------|       |
 *                  +-------+
 * 
 * 
 * 
 *                              i_next
 *        o----------------------------------------------------------o
 *        |                                                          |
 *        |                                                          |
 *        |                                                          |
 *        |        +---------+       +---------+        +---------+  |
 *        o------->|        -|------>|        -|-->...->|        -|--o
 *                 |  inode  |       |  inode  |        |  inode  |
 *                 |         |       |         |        |         |
 *        o--------|-        |<------|-        |<--...<-|-        |<-o
 *        |        +---------+       +---------+        +---------+  |
 *        |        A                                                 |
 *        |        |                                                 |
 *        |        o----first_inode                                  |
 *        |                                                          |
 *        o----------------------------------------------------------o
 *                             i_prev 
 *
 * 
 */
static void insert_inode_free(struct inode *inode)
{
    inode->i_next = first_inode;
    inode->i_prev = first_inode->i_prev;
    inode->i_next->i_prev = inode;
    inode->i_prev->i_next = inode;
    first_inode = inode;
}

/*
 * Insert inode into hash table
 *
 * +---------+
 * |         |
 * |         |
 * |         |           +-------+  i_hash_next  +-------+
 * +---------+           |      -|-------------->|      -|--> ...
 * |         |           | inode |               | inode |
 * |  Entry  |   inode   |       |<--------------|-      |<-- ...
 * |        -|---------->+-------+  i_hash_prev  +-------+
 * +---------+        
 * |         |
 * |         |
 * |         |
 * +---------+
 *         
 */
static __unused void insert_inodes_hash(struct inode *inode) 
{
    struct inode_hash_entry *h;

    h = hash(inode->i_dev, inode->i_ino);

    inode->i_hash_next = h->inode;
    inode->i_hash_prev = NULL;
    if (inode->i_hash_next)
        inode->i_hash_next->i_hash_prev = inode;
    h->inode = inode;
}

/*
 * put_last_free()
 *   Add an non-free inode into tail of free list.
 *
 * 
 *                              i_next
 *        o----------------------------------------------------------o
 *        |                                                          |
 *        |                                                          |
 *        |        (non-free)          (free)             (free)     |
 *        |        +---------+       +---------+        +---------+  |
 *        o------->|        -|------>|        -|-->...->|        -|--o
 *                 |  inode  |       |  inode  |        |  inode  |
 *                 |         |       |         |        |         |
 *        o--------|-        |<------|-        |<--...<-|-        |<-o
 *        |        +---------+       +---------+        +---------+  |
 *        |                          A                               |
 *        |                          |                               |
 *        |        first_inode-------o                               |
 *        |                                                          |
 *        o----------------------------------------------------------o
 *                             i_prev 
 *

 */
static __unused void put_last_free(struct inode *inode)
{
    remove_inode_free(inode);
    inode->i_prev = first_inode->i_prev;
    inode->i_prev->i_next = inode;
    inode->i_next = first_inode;
    inode->i_next->i_prev = inode;
}

/*
 * Note that we don't want to disturb any wait-queues when we discard
 * an inode.
 *
 * Argghh. Got bitten by a gcc problem with inlining: no way to tell
 * the compiler that the inline asm function 'memset' changes 'inode'
 * I've been searching for the bug for days, and was getting desperate.
 * Finally looked at the assembler output... Grrr.
 *
 * The solution is the weird use of 'volatile'. Ho humm. Have to report
 * it to the gcc lists, and hope we can do this more cleanly some day..
 */
static void clear_inodes(struct inode *inode)
{
    struct wait_queue *wait;

    wait_on_inode(inode);
    remove_inode_hash(inode);
    remove_inode_free(inode);
    wait = ((volatile struct inode *)inode)->i_wait;
    if (inode->i_count)
        nr_free_inodes++;
    memset(inode, 0, sizeof(*inode));
    ((volatile struct inode *)inode)->i_wait = wait;
    insert_inode_free(inode);
}

#ifdef CONFIG_DEBUG_INODE_INIT

static void inode_inits(void)
{
    memset(hash_table, 0, sizeof(hash_table));
    first_inode = NULL;
}

static int debug_inode_init(void)
{
    inode_inits();
    return 0;
}
subsys_debugcall(debug_inode_init);
#endif

/*
 * grow_inode()
 *   This function is used to add new free inode. At first, it will 
 *   obtian a new page for inode. if first_inode points to NULL,
 *   and then first_inode will be pointed to second inode in new page.
 *
 *   0-------+-------+----------------------------+-------+-----4k
 *   |       |       |                            |       |      |
 *   | inode | inode | ...                        | inode | hole |
 *   |       |       |                            |       |      |
 *   +-------+-------+----------------------------+-------+------+
 *           A   |
 *           |   |
 *           |   | i_next/i_prev
 *           |   |
 *           |   V
 *       first_inode
 */
static void grow_inode(void)
{
    struct inode *inode;
    int i;

    /* we only invoke "get_free_page" not "__get_free_page",
     * "get_free_page" will allocate a zero page, but "_get_free_page"
     * only allocate a non-zero page. 
     */
    if (!(inode = (struct inode *) get_free_page(GFP_KERNEL)))
        return;

    i = PAGE_SIZE / sizeof(struct inode);
    nr_inodes += i;
    nr_free_inodes += i;

    if (!first_inode) {
        /* Why ignore first inode from a free page? */
        inode++;
        inode->i_next = inode->i_prev = first_inode = inode, i--;
    }
    for ( ; i; i--)
        insert_inode_free(inode++);
}

/*
 * get_empty_inode()
 *   Get an empty inode from free list or allocate new inode.
 *   Note! Invoke 'get_empty_inode' will get a empty inode, but this
 *   inode is "first_inode", it hold on free inode list! and other code,
 *   put_last_free() will remove it from free inode list and add into
 *   tail of free inode list. 
 */
static __unused struct inode *get_empty_inodes(void)
{
    struct inode *inode, *best;
    int i;

    if (nr_inodes < NR_INODE && nr_free_inodes < (nr_inodes >> 2))
        grow_inode();
repeat:
    inode = first_inode;
    best = NULL;
    for (i = 0; i < nr_inodes; inode = inode->i_next, i++) {
        if (!inode->i_count) {
            if (!best)
                best = inode;
            /* free inode: no dirt and lock */
            if (!inode->i_dirt && !inode->i_lock) {
                best = inode;
                break;
            }
        }
    }
    /* Unable to get free inode, and add new inode into free inode list */
    if (!best || best->i_dirt || best->i_lock)
        if (nr_inodes < NR_INODE) {
            grow_inode();
            goto repeat;
        }
    inode = best;
    if (!inode) {
        printk(KERN_INFO "VFS: No free inodes - contact linus\n");
        sleep_on(&inode_wait);
        goto repeat;
    }
    if (inode->i_lock) {
        wait_on_inode(inode);
        goto repeat;
    }
    if (inode->i_dirt) {
        write_inode(inode);
        goto repeat;
    }
    if (inode->i_count)
        goto repeat;
    clear_inodes(inode);
    inode->i_count = 1;
    inode->i_nlink = 1;
    inode->i_sem.count = 1;
    nr_free_inodes--;
    if (nr_free_inodes < 0) {
        printk(KERN_INFO "VFS: get_empty_inode: bad free inode count.\n");
        nr_free_inodes = 0;
    }
    return inode;
}

#ifdef CONFIG_MINIX_FS
/*
 * read_inode_minix()
 *  Read inode structure from minix rootfs. detail see tools/demo/vfs/minix.
 */
static __unused void read_inode_minix(struct inode *inode)
{
    struct buffer_head *bh;
    struct minix_inode *raw_inode;
    int block, ino;

    ino = inode->i_ino;
    inode->i_op = NULL;
    inode->i_mode = 0;
    if (!ino || ino >= inode->i_sb->u.minix_sb.s_ninodes) {
        printk(KERN_ERR "Bad inode number on dev 0x%04x: %d "
               "is out of range\n", inode->i_dev, ino);
        return;
    }
    block = 2 + inode->i_sb->u.minix_sb.s_imap_blocks +
                inode->i_sb->u.minix_sb.s_zmap_blocks +
                (ino - 1) / MINIX_INODES_PER_BLOCK;
    if (!(bh = bread(inode->i_dev, block, BLOCK_SIZE))) {
        printk(KERN_ERR "Major problem: unable to read inode from dev "
                        "%#04x\n", inode->i_dev);
        return;
    }
    raw_inode = ((struct minix_inode *)bh->b_data) +
                (ino - 1) % MINIX_INODES_PER_BLOCK;
    inode->i_mode = raw_inode->i_mode;
    inode->i_uid  = raw_inode->i_uid;
    inode->i_gid  = raw_inode->i_gid;
    inode->i_nlink = raw_inode->i_nlinks;
    inode->i_size  = raw_inode->i_size;
    inode->i_mtime = inode->i_atime = inode->i_ctime = raw_inode->i_time;
    inode->i_blocks = inode->i_blksize = 0;
    if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode))
        inode->i_rdev = raw_inode->i_zone[0];
    else {
        for (block = 0; block < 9; block++)
            inode->u.minix_i.i_data[block] = raw_inode->i_zone[block];
    }
    brelse(bh);
    if (S_ISREG(inode->i_mode))
        inode->i_op = &minix_file_inode_operations;
    else if (S_ISDIR(inode->i_mode))
        inode->i_op = &minix_dir_inode_operations;
    else if (S_ISLNK(inode->i_mode))
        inode->i_op = &minix_symlink_inode_operations;
    else if (S_ISCHR(inode->i_mode))
        inode->i_op = &chrdev_inode_operations;
    else if (S_ISBLK(inode->i_mode))
        inode->i_op = &blkdev_inode_operations;
    else if (S_ISFIFO(inode->i_mode))
        init_fifo(inode);
}

static __unused void put_inode_minix(struct inode *inode)
{
    if (inode->i_nlink)
        return;
    inode->i_size = 0;
    minix_truncate(inode);
    minix_free_inode(inode);
}

static struct buffer_head *minix_update_inode(struct inode *inode)
{
    struct buffer_head *bh;
    struct minix_inode *raw_inode;
    int ino, block;

    ino = inode->i_ino;
    if (!ino || ino >= inode->i_sb->u.minix_sb.s_ninodes) {
        printk("Bad inode number on dev 0x%04x: %d is out of range\n",
             inode->i_dev, ino);
        inode->i_dirt = 0;
        return 0;
    }
    block = 2 + inode->i_sb->u.minix_sb.s_imap_blocks + 
                inode->i_sb->u.minix_sb.s_zmap_blocks +
                (ino - 1) / MINIX_INODES_PER_BLOCK;
    if (!(bh = bread(inode->i_dev, block, BLOCK_SIZE))) {
        printk(KERN_ERR "unable to read i-inode block\n");
        inode->i_dirt = 0;
        return 0;
    }
    raw_inode = ((struct minix_inode *)bh->b_data) +
                (ino - 1) % MINIX_INODES_PER_BLOCK;
    raw_inode->i_mode = inode->i_mode;
    raw_inode->i_uid  = inode->i_uid;
    raw_inode->i_gid  = inode->i_gid;
    raw_inode->i_nlinks = inode->i_nlink;
    raw_inode->i_size = inode->i_size;
    raw_inode->i_time = inode->i_mtime;
    if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode))
        raw_inode->i_zone[0] = inode->i_rdev;
    else for (block = 0; block < 9; block++)
        raw_inode->i_zone[block] = inode->u.minix_i.i_data[block];
    inode->i_dirt = 0;
    bh->b_dirt = 1;
    return bh;
}

static __unused void write_inode_minix(struct inode *inode)
{
    struct buffer_head *bh;

    bh = minix_update_inode(inode);
    brelse(bh);
}

#endif

#ifdef CONFIG_DEBUG_INODE_READ_SPECIAL
static void read_inode_rootfs(struct inode *inode)
{
#ifdef CONFIG_MINIX_FS
    read_inode_minix(inode);
#endif
}
#endif

#ifdef CONFIG_DEBUG_INODE_IPUT_ROOTFS
static void put_inode_rootfs(struct inode *inode)
{
#ifdef CONFIG_MINIX_FS
    put_inode_minix(inode);
#endif
}

#endif

/*
 * read_inode()
 *   Read special inode information from rootfs.
 */
static __unused void read_inode(struct inode *inode)
{
    lock_inode(inode);
#ifdef CONFIG_DEBUG_INODE_READ_SPECIAL
    read_inode_rootfs(inode);
#else
    if (inode->i_sb && inode->i_sb->s_op && inode->i_sb->s_op->read_inode)
        inode->i_sb->s_op->read_inode(inode);
#endif
    unlock_inode(inode);
}

#ifdef CONFIG_DEBUG_INODE_WRITE_SPECIAL
static void write_inode_rootfs(struct inode *inode)
{
#ifdef CONFIG_MINIX_FS
    write_inode_minix(inode);
#endif
    inode->i_dirt = 0;
}
#endif

/*
 * write_inode()
 *   Write inode data into rootfs.
 */
static void write_inode(struct inode *inode)
{
    if (!inode->i_dirt)
        return;
    wait_on_inode(inode);
    if (!inode->i_dirt)
        return;
    if (!inode->i_sb || !inode->i_sb->s_op ||
              !inode->i_sb->s_op->write_inode) {
        inode->i_dirt = 0;
        return;
    }
    inode->i_lock = 1;
#ifdef CONFIG_DEBUG_INODE_WRITE_SPECIAL
    write_inode_rootfs(inode);
#else
    inode->i_sb->s_op->write_inode(inode);
#endif
    unlock_inode(inode);
}

#ifdef CONFIG_DEBUG_INODE_IGET
/*
 * debug_iget()
 *   Debug main routine for iget(). We can invoke iget() to get
 *   a valid inode structure. At first, iget() will search a valid
 *   inode from INODE hash table, if it found and verify whether it is
 *   valid, if it valid and return it directly. If not, iget() will
 *   invoke 'get_empty_inode' to allocate a new page to manage new 
 *   inode set.
 */
static struct inode *debug_iget(int nr, int crossmntp)
{
    struct super_block *sb = current->pwd->i_sb;
    struct inode_hash_entry *h;
    struct inode *inode;
    struct inode *empty = NULL;
    static struct wait_queue *update_wait = NULL;

    if (!sb)
        panic("VFS: iget with sb==NULL");
    h = hash(sb->s_dev, nr);
repeat:
    for (inode = h->inode; inode; inode = inode->i_hash_next)
        if (inode->i_dev == sb->s_dev && inode->i_ino == nr)
            goto found_it;

    if (!empty) {
        h->updating++;
        empty = get_empty_inodes();
        if (!--h->updating)
            wake_up(&update_wait);
        if (empty)
            goto repeat;
        return (NULL);
    }
    inode = empty;
    inode->i_sb = sb;
    inode->i_dev = sb->s_dev;
    inode->i_ino = nr;
    inode->i_flags = sb->s_flags;
    /* 
     * 'put_last_free()' remove inode from free inode list and then add
     * inode into tail of free inode list. 
     */
    put_last_free(inode);
    insert_inodes_hash(inode);
    read_inode(inode);
    goto return_it;

found_it:
    /* Obtain inode from HASH table */
    if (!inode->i_count)
        nr_free_inodes--;
    inode->i_count++;
    wait_on_inode(inode);
    if (inode->i_dev != sb->s_dev || inode->i_ino != nr) {
        printk(KERN_ERR "Whee.. inode changed from under us. Tell Linus\n");
        iput(inode);
        goto repeat;
    }
    if (crossmntp && inode->i_mount) {
        struct inode *tmp = inode->i_mount;

        tmp->i_count++;
        iput(inode);
        inode = tmp;
        wait_on_inode(inode);
    }
    if (empty)
        iput(empty);
    
return_it:
    while (h->updating)
        sleep_on(&update_wait);
    return inode;
}
#endif

#ifdef CONFIG_DEBUG_INODE_IPUT

void debug_iput(struct inode *inode)
{
    if (!inode)
        return;
    wait_on_inode(inode);
    if (!inode->i_count) {
        printk(KERN_ERR "VFS: iput: trying to free free inode\n");
        printk(KERN_ERR "VFS: device %d/%d, inode %lu, mode=0%07o\n",
                MAJOR(inode->i_rdev), MINOR(inode->i_rdev),
                inode->i_ino, inode->i_mode);
        return;
    }
    if (inode->i_pipe)
        wake_up_interruptible(&PIPE_WAIT(*inode));

#ifdef CONFIG_DEBUG_INODE_WRITE_SPECIAL
    /* force sync */

    inode->i_count = 0;
    inode->i_dirt = 1;
#endif

repeat:
    if (inode->i_count > 1) {
        /* More userful operation. */
        inode->i_count--;
        return;
    }
    wake_up(&inode_wait);
    if (inode->i_pipe) {
        unsigned long page = (unsigned long) PIPE_BASE(*inode);

        PIPE_BASE(*inode) = NULL;
        free_page(page);
    }
#ifdef CONFIG_DEBUG_INODE_IPUT_ROOTFS
    put_inode_rootfs(inode);
    if (!inode->i_nlink)
        return;
#else
    if (inode->i_sb && inode->i_sb->s_op && inode->i_sb->s_op->put_inode) {
        inode->i_sb->s_op->put_inode(inode);
        if (!inode->i_nlink)
            return;
    }
#endif
    if (inode->i_dirt) {
        write_inode(inode);
        goto repeat;
    }
    inode->i_count--;
    nr_free_inodes++;
    return;
}
#endif

#ifdef CONFIG_DEBUG_INODE_ATTR
static void parse_inode_attr(struct inode *inode)
{
}
#endif

/* The entry for systemcall */
asmlinkage int sys_vfs_inode(const char *filename)
{
    char *tmp;
    int error;
    struct inode *inode;

    error = getname(filename, &tmp);
    if (error)
        return error;
    error = open_namei(filename, O_RDWR, 0, &inode, NULL);
    if (error)
        return error;
    inode->i_count++;
#ifdef CONFIG_DEBUG_INODE_IGET
    inode = debug_iget(inode->i_ino, 1);
#else
    inode = iget(inode->i_sb, inode->i_ino);
#endif

#ifdef CONFIG_DEBUG_INODE_ATTR
    parse_inode_attr(inode);
#endif

#ifdef CONFIG_DEBUG_INODE_IPUT
    debug_iput(inode);
#else
    iput(inode);
#endif

#ifdef CONFIG_DEBUG_INODE_IGET_HASH
    inode = debug_iget(inode->i_ino, 1);
    iput(inode);
#endif

    putname(tmp);
    return error;
}

static int debug_inode(void)
{
    vfs_inode("/etc/BiscuitOS.rc");
    return 0;
}
user1_debugcall(debug_inode);
