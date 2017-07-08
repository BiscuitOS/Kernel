/*
 * linux/fs/buffer.c
 *
 * (C) 1991 Linus Torvalds
 */
#include <linux/fs.h>
#include <sys/types.h>
#include <linux/list.h>
#include <string.h>
#include <asm/system.h>
#include <linux/sched.h>


extern int end;

/* 640KB-1MB is used by BIOS and Vedio, it is hard coded, sorry */
#define BIOS_USED_START	(640 << KB_SHIFT)
#define BIOS_USED_END	(1 << MB_SHIFT)
#define COPYBLK(from, to) \
__asm__("cld\n\t" \
	"rep\n\t" \
	"movsl\n\t" \
	: : "c" (BLOCK_SIZE/4), "S" (from), "D" (to) \
	)

static int NR_BUFFERS;
static struct task_struct *buffer_wait;
static struct buffer_head *free_list;

static struct buffer_head *start_buffer = (struct buffer_head *)&end;
static struct buffer_head *hash_table[NR_HASH];

#define _hashfn(dev, block) (((unsigned)(dev^block))%NR_HASH)
#define hash(dev, block) hash_table[_hashfn(dev, block)]

static void wait_on_buffer(struct buffer_head *bh)
{
	irq_disable();
	while (bh->b_lock)
		sleep_on(&buffer_wait);
	irq_enable();
}

void brelse(struct buffer_head *bh)
{
	if (!bh)
		return;

	wait_on_buffer(bh);
	if (!(bh->b_count--))
		panic("Trying to free freed buffer.");
	wake_up(&buffer_wait);
}

/* TODO */
static struct buffer_head *getblk(int dev, int block)
{
	struct buffer_head *bh = NULL;

	return bh;
}

struct buffer_head *bread(int dev, int block)
{
	struct buffer_head *bh = getblk(dev, block);

	if (!bh)
		panic("Can't get a valid buffer.");

	if (bh->b_uptodate)
		return bh;

	ll_rw_block(READ, bh);
	wait_on_buffer(bh);
	if (bh->b_uptodate)
		return bh;

	brelse(bh);
	return NULL;
}

/* In BiscuitOS, buffer_end is at 4MB currently */
void buffer_init(long buffer_end)
{
    struct buffer_head *h = start_buffer;
    void *b;
    int i;

    if (buffer_end == 1 << 20)
        b = (void *)(640 * 1024);
    else
        b = (void *)buffer_end;
    while ((b -= BLOCK_SIZE) >= ((void *)(h + 1))) {
        h->b_dev    = 0;
        h->b_dirt   = 0;
        h->b_count  = 0;
        h->b_lock   = 0;
        h->b_uptodate = 0;
        h->b_wait = NULL;
        h->b_next = NULL;
        h->b_prev = NULL;
        h->b_data = (char *)b;
        h->b_prev_free = h - 1;
        h->b_next_free = h + 1;
        h++;
        NR_BUFFERS++;
        if (b == (void *)0x100000)
            b = (void *)0xA0000;
    }
    h--;
    free_list = start_buffer;
    free_list->b_prev_free = h;
    h->b_next_free = free_list;
    for (i = 0; i < NR_HASH; i++)
        hash_table[i] = NULL;
}

int sync_dev(int dev)
{
    int i;
    struct buffer_head *bh;

    bh = start_buffer;
    for (i = 0; i < NR_BUFFERS; i++, bh++) {
        if (bh->b_dev != dev)
            continue;
        wait_on_buffer(bh);
        if (bh->b_dev == dev && bh->b_dirt)
            ll_rw_block(WRITE, bh);
    }
    sync_inodes();
    bh = start_buffer;
    for (i = 0; i < NR_BUFFERS; i++, bh++) {
        if (bh->b_dev != dev)
            continue;
        wait_on_buffer(bh);
        if (bh->b_dev == dev && bh->b_dirt)
            ll_rw_block(WRITE, bh);
    }
    return 0;
}

static struct buffer_head *find_buffer(int dev, int block)
{
    struct buffer_head *tmp;
    
    for (tmp = hash(dev, block); tmp != NULL; tmp = tmp->b_next)
        if (tmp->b_dev == dev && tmp->b_blocknr == block)
            return tmp;
    return NULL;
}

/*
 * Why like this, I hear you say... The reason is race-conditions.
 * As we don't lock buffers (unless we are readint them, that is),
 * something might happen to it while we sleep (ie a read-error 
 * will force it bad). This shouldn't really happen currently, but
 * the code is ready.
 */
struct buffer_head *get_hash_table(int dev, int block)
{
    struct buffer_head *bh;

    for (;;) {
        if (!(bh = find_buffer(dev, block)))
            return NULL;
        bh->b_count++;
        wait_on_buffer(bh);
        if (bh->b_dev == dev && bh->b_blocknr == block)
            return bh;
        bh->b_count--;
    }
}
