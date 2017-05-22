/*
 * Memory manage
 * Maintainer: Buddy <buddy.zhang@aliyun.com>
 *
 * Copyright (C) 2017 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * These are not to be changed without changing head.s etc.
 * Current version only support litter than 16Mb.
 */
#define LOW_MEM        0x100000
#define PAGING_MEMORY  (15 * 1024 * 1024)
#define PAGING_PAGES   (PAGING_MEMORY >> 12)
#define MAP_NR(addr)   (((addr) - LOW_MEM) >> 12)
#define USED 100

static long HIGH_MEMORY = 0;

static unsigned char mem_map[PAGING_PAGES] = { 0, };

/*
 * Initialize memory, build mapping array.
 */
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
