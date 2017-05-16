#include <linux/fs.h>

extern int end;

struct buffer_head *start_buffer = (struct buffer_head *)&end;
struct buffer_head *hash_table[NR_HASH];
static struct buffer_head *free_list;

int NR_BUFFERS = 0;

void buffer_init(long buffer_end)
{
	struct buffer_head *h = start_buffer;
	void *b;
	int i;

	if (buffer_end == 1 << 20)
		b = (void *)(640 * 1024);
	else
		b = (void *) buffer_end;

	while ((b -= BLOCK_SIZE) >= ((void *)(h + 1))) {
		h->b_dev      = 0;
		h->b_dirt     = 0;
		h->b_count    = 0;
		h->b_lock     = 0;
		h->b_uptodate = 0;
		h->b_wait     = NULL;
		h->b_next       = NULL;
		h->b_prev       = NULL;
		h->b_data       = (char *)b;
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
