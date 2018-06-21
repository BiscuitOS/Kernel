/*
 * Buffer
 *
 * (C) 2018.06.19 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/unistd.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/locks.h>

#include <demo/debug.h>

/*
 * VFS Buffer
 *   VFS Buffer is common data area that store data from HD/Floppy. Buffer
 *   subsystem manage data exchange between memory with Disk, it be more
 *   like a data cache for HD/Floppy data. Buffer subsystem contain 4 
 *   queues that manages mate data 'buffer_head' and data area. The 4
 *   queues are: Hash_table, free_list, unused_list and this_page. 
 *   I'll describe mechanism and how to work for 3 queues.
 *
 *   UNUSED_LIST
 *    The unused_list is a typical single linked list, it manage unused
 *    'buffer_head' structure. When system allocate memory to establish
 *    new 'buffer_head', it will be insert into header of 'unused_list'.
 *    And then Buffer subsystem will invoke 'get_unused_buffer_head()' to 
 *    obtain a unsed and empty 'buffer_head'. When 'unused_list' becomes
 *    empty, the subsystem will invoke 'get_more_buffer_heads()' to 
 *    get more unused 'buffer_head'.
 *    The 'unusd_list' points to first unused 'buffer_head', and the member
 *    of 'unused_list' utilze 'b_next_free' to connect another 'buffer_head'
 *
 *    Note! The member of 'unused_list' doesn't contain data area!
 *
 *    As figure:
 * 
 *    0-------------+-------------+--------------+-------------+-----4k
 *    |             |             |              |             |      |
 *    | buffer_head | buffer_head | ...          | buffer_head | hole |
 *    |     (0)     |     (1)     |              |     (n)     |      |
 *    +-------------+-------------+--------------+-------------+------+
 *                                   |
 *                                   |
 *                                   |
 *                                   |
 *                                   V
 *
 *              +-------------+               +-------------+
 *              |             |               |             |
 *              | buffer_head |  b_next_free  | buffer_head |
 *     NULL<----|-    (0)     |<--------------|-    (1)     |
 *              +-------------+               +-------------+<---unused_list 
 *
 *   FREE_LIST
 *     The 'free_list' is typical double linked list, it manage all free and
 *     in-used 'buffer_head' sturuct. The member of 'free_list' contains 
 *     data zone. The 'free_list' points to first free 'buffer_head', and
 *     Buffer subsystem can invoke 'remove_from_queues()' to obtain a
 *     valid and empty 'buffer_head' that contain data area. The 'free_list'
 *     also manage all in-used 'buffer_head'. When obtain a free 
 *     a 'buffer_head', we should insert it into tail of 'free_list'.
 *     'free_list' utlize 'b_prev_free' and 'b_next_free' to connect another
 *     'buffer_head'.
 *
 *     As Figure:
 *
 *        (In-used)                             (free)
 *     +-------------+     b_prev_free      +-------------+
 *     |             |<---------------------|-            |
 *     | buffer_head |                      | buffer_head |
 *     |             |     b_next_free      |             |
 *     |            -|--------------------->|            -|---> ...
 *     |            -|---------o            |            -|---o
 *     +-------------+         |      o---->+-------------+   |
 *                             |      |                       |
 *                      b_data |    free_list                 |
 *                             |                    b_data    |
 *                             |                 o------------o
 *                             |                 |
 *                             |                 |
 *                             V                 V
 *     0-----------+-----------+-----------+-----+-----------+-----4K
 *     |           |           |           |     |           |      |
 *     | Data Area | Data Area | Data Area | ... | Data Area | hold |
 *     |           |           |           |     |           |      |
 *     +-----------+-----------+-----------+-----+-----------+------+
 *
 *
 *   HASH_TABLE
 *    In order to accelerate to find special block, Buffer subsystem manage
 *    a hast table. The key value depend on device number and block number.
 *    Each member utlize 'b_next' and 'b_prev' to connect another memober.
 *    We can invoke 'remove_from_hash_queue()' to remove a 'buffer_bead'
 *    from hash table and also invoke "insert_into_queues()" to insert
 *    a 'buffer_head' into Hash_table.
 *
 *    As follow:
 *
 *    +---------------+
 *    |               |
 *    +---------------+
 *    | buffer_head * |
 *    +---------------+
 *    |               |
 *    |               |
 *    |               |
 *    |               |      +-------------+   b_prev   +-------------+
 *    |               |      |             |<-----------|-            |
 *    |               |      | buffer_head |            | buffer_head |
 *    +---------------+      |            -|----------->|            -|--> NULL
 *    | buffer_head * |----->+-------------+   b_next   +-------------+
 *    +---------------+
 *    |               |
 *    |               |
 *    |               |
 *    +---------------+
 *
 * 
 *   THIS_PAGE
 *    It's a typical single linked list and used to manage 'buffer_head'
 *    that data area from same page.
 *
 *    As figure
 *
 *             +-------------+ b_this_page  +-------------+ b_this_page
 *             |            -|------------->|            -|-------------> NULL
 *             | buffer_head |              | buffer_head |
 *             |            -|---o          |            -|---o
 *    head---->+-------------+   |          +-------------+   |
 *                               |                            |
 *                               |                            |
 *            b_data             |                            |
 *    o--------------------------o                            |
 *    |                                        b_data         |
 *    |            o------------------------------------------o
 *    |            |
 *    |            |
 *    |            |
 *    V            V          Buffer Data area
 *    0------------+------------+--------------+------------+-----4K
 *    |            |            |              |            |      |
 *    | BLOCK_SIZE | BLOCK_SIZE | ...          | BLOCK_SIZE | hole |
 *    |            |            |              |            |      |
 *    +------------+------------+--------------+------------+------+
 */

static inline _syscall3(int, open, const char *, file, int, flag, int, mode);
static inline _syscall1(int, close, int, fd);

static struct buffer_head *hash_table[NR_HASH];
static struct buffer_head *free_list = NULL;
static struct buffer_head *unused_list = NULL;
static struct wait_queue  *buffer_wait = NULL;

/*
 * Meta Data
 *  VFS Buffer mange some argument to indicate usage of buffer, we can
 *  do some useful operation according these argument.
 *
 *  nr_buffer
 *   This argument indicate subsystem
 */

static int nr_buffer = 0;
static int nr_buffer_head = 0;
static int buffermems = 0;
static int min_free_pages = 20; /* nr free pages needed before buffer grows */

static int grow_buffers(int pri, int size);

#define _hashfn(dev,block) (((unsigned)(dev^block))%NR_HASH)
#define hash(dev,block)  hash_table[_hashfn(dev,block)]

static struct buffer_head *find_buffer(dev_t dev, int block, int size)
{
    struct buffer_head *tmp;

    for (tmp = hash(dev, block); tmp != NULL; tmp = tmp->b_next)
        if (tmp->b_dev == dev && tmp->b_blocknr == block) {
            if (tmp->b_size == size)
                return tmp;
            else {
                printk(KERN_ERR "VFS: Wrong blocksize on device %d/%d\n",
                             MAJOR(dev), MINOR(dev));
                return NULL;
            }
        }
    return NULL;
}

/*
 * Why like this, I hear you say... The reason is race-conditions.
 * As we don't lock buffers (unless we are readint them, that is),
 * something might happen to it while we sleep (ie a read-error
 * will force it bad). This shouldn't really happen currently, but
 * the code is ready.
 */
static struct buffer_head *get_hash_tables(dev_t dev, int block, int size)
{
    struct buffer_head *bh;

    for (;;) {
        if (!(bh = find_buffer(dev, block, size))) {
            return NULL;
        }
        bh->b_count++;
        wait_on_buffer(bh);
        if (bh->b_dev == dev && bh->b_blocknr == block &&
                bh->b_size == size)
            return bh;
        bh->b_count--;
    }
}

/*
 * VFS buffer manages a hash table to hold all valid buffers. The member
 * 'b_next' of 'buffer_head' structure is used to point next 'buffer_head'
 * that contain same key value. As figure:
 *
 * hash_table
 * 
 * +---------------+
 * |               |
 * +---------------+
 * | buffer_head * |
 * +---------------+
 * |               |
 * |               |
 * |               |
 * |               |      +-------------+   b_prev   +-------------+
 * |               |      |             |<-----------|-            |
 * |               |      | buffer_head |            | buffer_head |
 * +---------------+      |            -|----------->|            -|--> NULL
 * | buffer_head * |----->+-------------+   b_next   +-------------+
 * +---------------+
 * |               |
 * |               |
 * |               |
 * +---------------+
 *
 * remove_from_hash_queue()
 *  Remove a 'buffer_head' from hash_table.
 *
 */
static inline void remove_from_hash_queue(struct buffer_head *bh)
{
    if (bh->b_next)
        bh->b_next->b_prev = bh->b_prev;
    if (bh->b_prev)
        bh->b_prev->b_next = bh->b_next;
    if (hash(bh->b_dev, bh->b_blocknr) == bh)
        hash(bh->b_dev, bh->b_blocknr) = bh->b_next;
    bh->b_next = bh->b_prev = NULL;
}

/*
 * VFS Buffer manage a single linked list to hold all free 'buffer_head'
 * that contain data area. Each free 'buffer_head' utilze 'b_prev_free'
 * and 'b_next_free' to connect another 'buffer_head'. As Figure.
 *
 *
 *
 *             +-------------+     b_prev_free      +-------------+
 *             |             |<---------------------|-            |
 *             | buffer_head |     b_next_free      | buffer_head |
 *             |            -|--------------------->|            -|---> ...
 *             |            -|--------------------->|            -|---> ...
 *             |            -|---o    b_this_page   |            -|---o
 * free_list-->+-------------+   |                  +-------------+   |
 *                               |                                    |
 *                               | b_data                             |
 *                               |                        b_data      |
 *                         o-----o                 o------------------o
 *                         |                       |
 *                         |                       |
 *                         V                       V
 * 0-----------+-----------+-----------+-----------+-----------+-----4K
 * |           |           |           |           |           |      |
 * | Data Area | Data Area | Data Area | ...       | Data Area | hold |
 * |           |           |           |           |           |      |
 * +-----------+-----------+-----------+-----------+-----------+------+
 *
 * remove_from_free_list()
 *  Remove a free 'buffer_head' from free list.
 *
 */
static inline void remove_from_free_list(struct buffer_head *bh)
{
    if (!(bh->b_prev_free) || !(bh->b_next_free))
        panic("VFS: Free block list corrupted");
    bh->b_prev_free->b_next_free = bh->b_next_free;
    bh->b_next_free->b_prev_free = bh->b_prev_free;
    if (free_list == bh)
        free_list = bh->b_next_free;
    bh->b_next_free = bh->b_prev_free = NULL;
}

/*
 * remove_from_queues()
 *  In order to get a valid and free 'buffer_head', we should remove
 *  it from hast_table and free list.
 */
static inline void remove_from_queues(struct buffer_head *bh)
{
    remove_from_hash_queue(bh);
    remove_from_free_list(bh);
}

/*
 * insert_into_queues()
 *   Insert a 'buffer_head' structure into two queues: free_list and 
 *   hash_table. More information see above.
 */
static inline void insert_into_queues(struct buffer_head *bh)
{
    /* put at end of free list */
    bh->b_next_free = free_list;
    bh->b_prev_free = free_list->b_prev_free;
    free_list->b_prev_free->b_next_free = bh;
    free_list->b_prev_free = bh;
    /* put the buffer in new hash-queue if it has a device */
    bh->b_prev = NULL;
    bh->b_next = NULL;
    if (!bh->b_dev)
        return;
    bh->b_next = hash(bh->b_dev, bh->b_blocknr);
    hash(bh->b_dev, bh->b_blocknr) = bh;
    if (bh->b_next)
        bh->b_next->b_prev = bh;
}

/*
 * put_last_free()
 *  Remove buffer_head from free list and then insert it into tail of
 *  free list.
 */
static inline void put_last_free(struct buffer_head *bh)
{
    if (!bh)
        return;
    if (bh == free_list) {
        free_list = bh->b_next_free;
        return;
    }
    remove_from_free_list(bh);
    /* Add to back of free list */
    bh->b_next_free = free_list;
    bh->b_prev_free = free_list->b_prev_free;
    free_list->b_prev_free->b_next_free = bh;
    free_list->b_prev_free = bh;
}

/*
 * Ok, this is getblk, and it isn't very clear, again to hinder
 * race-conditions. Most of the code is seldom used, (ie repeating),
 * so it should be much more efficient than it looks.
 *
 * The algoritm is changed: hopefully better, and an elusive bug removed.
 *
 * 14.02.92: changed it to sync dirty buffers a bit: better performance
 * when the filesystem starts to get full of direct blocks (I hope). 
 */
#define BADNESS(bh)   (((bh)->b_dirt<<1)+(bh)->b_lock)
static struct buffer_head *getblks(dev_t dev, int block, int size)
{
    struct buffer_head *bh, *tmp;
    int buffers;
    static int grow_size = 0;

repeat:
    bh = get_hash_tables(dev, block, size);
    if (bh) {
        if (bh->b_uptodate && !bh->b_dirt)
            put_last_free(bh);
        return bh;
    }
    grow_size -= size;
    if (nr_free_pages > min_free_pages && grow_size <= 0) {
        if (grow_buffers(GFP_BUFFER, size))
            grow_size = PAGE_SIZE;
    }
    buffers = nr_buffer;
    bh = NULL;

    for (tmp = free_list; buffers-- > 0; tmp = tmp->b_next_free) {
        if (tmp->b_count || tmp->b_size != size)
            continue;
        if (mem_map[MAP_NR((unsigned long) tmp->b_data)] != 1)
            continue;
        if (!bh || BADNESS(tmp) < BADNESS(bh)) {
            bh = tmp;
            if (!BADNESS(tmp)) {
                break;
            }
        }
    }

    if (!bh) {
        if (nr_free_pages > 5)
            if (grow_buffers(GFP_BUFFER, size))
                goto repeat;
        if (!grow_buffers(GFP_ATOMIC, size))
            sleep_on(&buffer_wait);
        goto repeat;
    }

    if (bh->b_count || bh->b_size != size)
        goto repeat;
    if (bh->b_dirt) {
        panic("sync buffer");
    }
/* NOTE!! While we slept waiting for this block, somebody else might */
/* already have added "this" block to the cache, check it */
    if (find_buffer(dev, block, size))
        goto repeat;
/* OK, FINALLY we know that this buffer is the only one of its kind, */
/* and that it's unused (b_count=0), unlocked (b_lock=0), and clean */
    bh->b_count = 1;
    bh->b_dirt = 0;
    bh->b_uptodate = 0;
    bh->b_req = 0;
    remove_from_queues(bh);
    bh->b_dev = dev;
    bh->b_blocknr = block;
    insert_into_queues(bh);
    return bh;
}

/*
 * get_more_buffer_heads()
 *  Extablish a free buffer_head list when 'unused_list' is empty. The
 *  free buffer_head list is a typical singal linked list, and 
 *  'unused_list' points to first valid buffer_head. The function will
 *  affect the number of all valid free buffer_head. If we want to 
 *  get a unused buffer_head, the 'unused_list' is best choice.
 *  
 *  0-------------+-------------+--------------+-------------+-----4k
 *  |             |             |              |             |      |
 *  | buffer_head | buffer_head | ...          | buffer_head | hole |
 *  |     (0)     |     (1)     |              |     (n)     |      |
 *  +-------------+-------------+--------------+-------------+------+
 *                                   |
 *                                   |
 *                                   |
 *                                   |
 *                                   V
 *
 *              +-------------+               +-------------+
 *              |             |               |             |
 *              | buffer_head |  b_next_free  | buffer_head |
 *     NULL<----|-    (0)     |<--------------|-    (1)     |
 *              +-------------+               +-------------+<---unused_list
 */
static void get_more_buffer_heads(void)
{
    int i;
    struct buffer_head *bh;

    if (unused_list)
        return;

    if (!(bh = (struct buffer_head *) get_free_page(GFP_BUFFER)))
        return;

    for (nr_buffer_head += i = PAGE_SIZE / sizeof(*bh); i > 0; i--) {
        bh->b_next_free = unused_list;    /* only make link */
        unused_list = bh++;
    }
}

/*
 * get_unused_buffer_head()
 *  Obtain an unused buffer_head from 'unused_list'.
 */
static struct buffer_head *get_unused_buffer_head(void)
{
    struct buffer_head *bh;

    /* Get more valid buffer_head from system */
    get_more_buffer_heads();
    if (!unused_list)
        return NULL;
    bh = unused_list;
    unused_list = bh->b_next_free;
    bh->b_next_free = NULL;
    bh->b_data = NULL;
    bh->b_size = 0;
    bh->b_req = 0;
    return bh;
}

/*
 * See fs/inode.c for the weird use of volatile.
 * Clear buffer_head structure and insert 'unused_list'.
 */
static void put_unused_buffer_head(struct buffer_head *bh)
{
    struct wait_queue *wait;

    wait = ((volatile struct buffer_head *) bh)->b_wait;
    memset((void *)bh, 0, sizeof(*bh));
    ((volatile struct buffer_head *)bh)->b_wait = wait;
    bh->b_next_free = unused_list;
    unused_list = bh;
}

/*
 * Create the appropriate buffers when given a page for data area and
 * the size of each buffer.. Use the bh->b_this_page linked list to
 * follow the buffer created. Return NULL if unable to create more
 * buffer.
 *
 *
 *
 *          +-------------+ b_this_page  +-------------+ b_this_page
 *          |            -|------------->|            -|-------------> NULL
 *          | buffer_head |              | buffer_head |
 *          |            -|---o          |            -|---o
 * head---->+-------------+   |          +-------------+   |
 *                            |                            |
 *                            |                            |
 *         b_data             |                            |
 * o--------------------------o                            |
 * |                                        b_data         |
 * |            o------------------------------------------o
 * |            |
 * |            |
 * |            |
 * V            V          Buffer Data area
 * 0------------+------------+--------------+------------+-----4K
 * |            |            |              |            |      |
 * | BLOCK_SIZE | BLOCK_SIZE | ...          | BLOCK_SIZE | hole |
 * |            |            |              |            |      |
 * +------------+------------+--------------+------------+------+
 */
static struct buffer_head *create_buffers(unsigned long page,
                                unsigned long size)
{
    struct buffer_head *bh, *head;
    unsigned long offset;

    head = NULL;
    offset = PAGE_SIZE;
    while ((offset -= size) < PAGE_SIZE) {
        bh = get_unused_buffer_head();
        if (!bh)
            goto no_grow;
        bh->b_this_page = head;
        head = bh;
        bh->b_data = (char *)(page + offset);
        bh->b_size = size;
    }
    return head;
/*
 * In case anything failed, we just free everything we got.
 */
no_grow:
    bh = head;
    while (bh) {
        head = bh;
        bh = bh->b_this_page;
        put_unused_buffer_head(head);
    }
    return NULL;
}

/*
 * Try to increase the number of buffers available: the size argument
 * is used to determine what kind of buffers we want.
 *
 * The VFS utlize a single linked list to manage all free buffer. And free
 * buffer must contain 'buffer_head' and data area. The 'unused_list' only
 * manage unused 'buffer_head' structure that doesn't contain data area.
 * 'grow_buffers()' will establish a free buffer list and 'free_list' points
 * to first free 'buffer_head'. As figure:
 *
 *             +-------------+     b_prev_free      +-------------+
 *             |             |<---------------------|-            |
 *             | buffer_head |     b_next_free      | buffer_head |
 *             |            -|--------------------->|            -|---> ...
 *             |            -|--------------------->|            -|---> ...
 *             |            -|---o    b_this_page   |            -|---o
 * free_list-->+-------------+   |                  +-------------+   |
 *                               |                                    |
 *                               | b_data                             |
 *                               |                        b_data      |
 *                         o-----o                 o------------------o
 *                         |                       |
 *                         |                       |
 *                         V                       V
 * 0-----------+-----------+-----------+-----------+-----------+-----4K
 * |           |           |           |           |           |      |
 * | Data Area | Data Area | Data Area | ...       | Data Area | hold |
 * |           |           |           |           |           |      |
 * +-----------+-----------+-----------+-----------+-----------+------+
 *
 *
 */
static int grow_buffers(int pri, int size)
{
    unsigned long page;
    struct buffer_head *bh, *tmp;

    if ((size & 511) || (size > PAGE_SIZE)) {
        printk("VFS: grow_buffers: size = %d\n", size);
        return 0;
    }
    if (!(page = __get_free_page(pri)))
        return 0;
    bh = create_buffers(page, size);
    if (!bh) {
        free_page(page);
        return 0;
    }
    tmp = bh;
    while (1) {
        if (free_list) {
            tmp->b_next_free = free_list;
            tmp->b_prev_free = free_list->b_prev_free;
            free_list->b_prev_free->b_next_free = tmp;
            free_list->b_prev_free = tmp;
        } else {
            tmp->b_prev_free = tmp;
            tmp->b_next_free = tmp;
        }
        free_list = tmp;
        ++nr_buffer;
        if (tmp->b_this_page)
            tmp = tmp->b_this_page;
        else
            break;
    }
    tmp->b_this_page = bh;
    buffermems += PAGE_SIZE;
    return 1;
}

/*
 * brelse()
 *  Release special buffer_head refreence. this function will not relase
 *  a buffer but only decrease reference.
 */
static void brelses(struct buffer_head *bh)
{
    if (!bh)
        return;
    wait_on_buffer(bh);
    if (bh->b_count) {
        if (--bh->b_count)
            return;
        wake_up(&buffer_wait);
        return;
    }
    printk("VFS: brelse: Trying to free free buffer\n");
}

/*
 * bread() reads a specified block and returns the buffer that contains
 * it. It returns NULL if the block was unreadable.
 */
static __unused struct buffer_head *breads(dev_t dev, int block, int size)
{
    struct buffer_head *bh;

    if (!(bh = getblks(dev, block, size))) {
        printk("VFS: bread: Read error on device %d/%d\n",
                 MAJOR(dev), MINOR(dev));
        return NULL;
    }
    if (bh->b_uptodate)
        return bh;
    ll_rw_block(READ, 1, &bh);
    wait_on_buffer(bh);
    if (bh->b_uptodate)
        return bh;
    brelses(bh);
    return NULL;
}

#ifdef CONFIG_DEBUG_BUFFER_INIT
/*
 * This initializes the initial buffer free list. nr_buffers is set
 * to one less the actual number of buffers, as a sop to backwards
 * compatibility --- the old code did this (I think unintentionally,
 * but I'm not sure), and program in the ps package expect it.
 *                                           - TYT 8/30/92
 */
static int debug_buffer_init(void)
{
    int i;

    if (high_memory >= 4 * 1024 * 1024)
        min_free_pages = 200;
    else
        min_free_pages = 20;
    for (i = 0; i < NR_HASH; i++)
        hash_table[i] = NULL;
    free_list = 0;
    grow_buffers(GFP_KERNEL, BLOCK_SIZE);
    if (!free_list)
        panic("VFS: Unable to initialize buffer free list");
    return 0;
}
late_debugcall(debug_buffer_init);
#endif

/* The entry for systemcall */
asmlinkage int sys_vfs_buffer(int fd)
{
    struct file *filp;
    struct inode *inode;
    struct buffer_head *bh = NULL;
    unsigned short *p __unused;

    filp = current->filp[fd];
    if (!filp) {
        printk("Task error on file descriptor table.\n");
        return -EINVAL;
    }
    inode = filp->f_inode;
    if (!inode) {
        printk("File descriptor error\n");
        return -EINVAL;
    }
    inode->i_count++;

#ifdef CONFIG_DEBUG_BUFFER_GET
    bh = getblks(inode->i_dev, 0, BLOCK_SIZE);
#endif
    if (bh) {
        printk("Obtain 0-->block: %#08x\n", (unsigned int)bh);
#ifdef CONFIG_DEBUG_BUFFER_BRELSE
        brelses(bh);
#else
        brelse(bh);
#endif
    }
#ifdef CONFIG_DEBUG_BUFFER_BREAD
#ifdef CONFIG_MINIX_FS
    p = inode->u.minix_i.i_data + 0;
#endif
    bh = breads(inode->i_dev, *p, BLOCK_SIZE);
    if (bh) {
        char *data;
        int i;

        data = (char *)bh->b_data;
        for (i = 0; i < 100; i++)
            printk("%c ", data[i]);

        brelses(bh);
    }
#endif

    iput(inode);
    return 0;
}

/* Common systemcall entry */
inline _syscall1(int, vfs_buffer, int, fd);

/* userland code */
static int debug_buffer(void)
{
    int fd;

    fd = open("/etc/rc", O_RDONLY, 0);
    if (fd < 0) {
        printf("Unable open '/etc/rc'\n");
        return -1;
    }
    vfs_buffer(fd);
    close(fd);

    return 0;
}
user1_debugcall_sync(debug_buffer);
