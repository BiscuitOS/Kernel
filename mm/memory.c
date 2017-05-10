
/*
 * These are not to be changed without changing head.s etc.
 */
#define LOW_MEM        0x100000
#define PAGING_MEMORY  (15 * 1024 * 1024)
#define PAGING_PAGES   (PAGING_MEMORY >> 12)
#define MAP_NR(addr)   (((addr) - LOW_MEM) >> 12)
#define USED 100

static long HIGH_MEMORY = 0;

static unsigned char mem_map[PAGING_PAGES] = {0,};

void mem_init(long start_mem, long end_mem)
{
	int i;

	HIGH_MEMORY = end_mem;
	for (i = 0; i < PAGING_PAGES; i++)
		mem_map[i] = USED;
	i = MAP_NR(start_mem);
	end_mem -= start_mem;
	end_mem >>= 12;
	while (end_mem-- > 0)
		mem_map[i++] = 0;
}
