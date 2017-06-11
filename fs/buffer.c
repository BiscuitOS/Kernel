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

static LIST_HEAD(free_list);
static int NR_BUFFERS;
static struct task_struct *buffer_wait;

static struct buffer_head *start_buffer = (struct buffer_head *)&end;
static struct list_head hash_table[NR_HASH];

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
	struct buffer_head *bh = start_buffer;
	void *block;

	/* Skip bios used memory */
	if (buffer_end == BIOS_USED_END)
		block = (void *)(BIOS_USED_START);
	else
		block = (void *)buffer_end;

	while ((block -= BLOCK_SIZE) >= ((void *)(bh + 1))) {
		memset(bh, 0, sizeof(struct buffer_head));
		bh->b_data = (char *)block;
		list_add_tail(&bh->b_list, &free_list);
		bh++;
		NR_BUFFERS++;

		if (block == (void *)(BIOS_USED_END))
			block = (void *)(BIOS_USED_START);
	}

	memset(hash_table, 0, sizeof(struct list_head) * NR_HASH);
}
