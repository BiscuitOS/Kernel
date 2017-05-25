/*
 *  linux/mm/memory.c
 *
 *  (C) 1991  Linus Torvalds
 */

#include <linux/kernel.h>
/*
 * These are not to be changed without changing head.s etc.
 * Current version only support litter than 16Mb.
 */
#define LOW_MEM        0x100000
#define PAGING_MEMORY  (15 * 1024 * 1024)
#define PAGING_PAGES   (PAGING_MEMORY >> 12)
#define MAP_NR(addr)   (((addr) - LOW_MEM) >> 12)
#define USED 100

#define invalidate() \
	__asm__("movl %%eax, %%cr3" :: "a" (0))

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

/*
 * Get physical address of first (actully last :-) free page, and mark it
 * used. If no free pages left, return 0.
 */
unsigned long get_free_page(void)
{
	register unsigned long __res asm("ax");

	__asm__("std ; repne ; scasb\n\t"
			"jne 1f\n\t"
			"movb $1, 1(%%edi)\n\t"
			"sall $12, %%ecx\n\t"
			"addl %2, %%ecx\n\t"
			"movl %%ecx, %%edx\n\t"
			"movl $1024, %%ecx\n\t"
			"leal 4092(%%edx), %%edi\n\t"
			"rep ; stosl\n\t"
			" movl %%edx, %%eax\n"
			"1:cld"
			: "=a" (__res)
			: "0" (0), "i" (LOW_MEM), "c" (PAGING_PAGES),
			  "D" (mem_map + PAGING_PAGES - 1)
			  );
	return __res;
}

/*
 * Free a page of memory at physical address 'addr'. Used by
 * 'free_page_tables()'
 */
void free_page(unsigned long addr)
{
	if (addr < LOW_MEM)
		return;
	if (addr >= HIGH_MEMORY)
		panic("trying to free nonexistent page");
	addr -= LOW_MEM;
	addr >>= 12;
	if (mem_map[addr]--)
		return;
	mem_map[addr] = 0;
	panic("trying to free free page");
}

/*
 * Well, here is one of the most complicated function in mm. It
 * copies a range of linerar address by copying only the pages.;-
 * Let's hope this is bug-free, 'cause this one I don't want to debug :-)
 *
 * Note! We don't copy just any chunks of memory - addresses have to
 * be divisible by 4Mb (one page-directory entry), as this makes the
 * function easier. It's used only by fork anyway.
 *
 * Note 2!! When from==0 we are copying kernel space for the first
 * fork(). Then we DONT want to copy a full page-directory entry, as
 * that would lead to some serious memory waste - we just copy the
 * first 160 pages - 640kB. Even that is more than we need, but it
 * doesn't take any more memory - we don't copy-on-write in the low
 * 1 Mb-range, so the pages can be shared with the kernel. Thus the
 * special case for nr=xxxx.
 */
int copy_page_tables(unsigned long from, unsigned long to, long size)
{
	unsigned long *from_page_table;
	unsigned long *to_page_table;
	unsigned long this_page;
	unsigned long *from_dir, *to_dir;
	unsigned long nr;

	if ((from & 0x3fffff) || (to & 0x3fffff))
		panic("copy_page_tables called with wrong alignment");
	from_dir = (unsigned long *) ((from >> 20) & 0xFFC);  /* _pf_dir = 0 */
	to_dir = (unsigned long *) ((to >> 20) & 0xFFC);
	size = ((unsigned) (size + 0x3fffff)) >> 22;
	for ( ; size-- > 0; from_dir++, to_dir++) {
		if (1 & *to_dir)
			panic("copy_page_tables: already exist");
		if (!(1 & *from_dir))
			continue;
		from_page_table = (unsigned long *) (0xfffff000 & *from_dir);
		if (!(to_page_table = (unsigned long *) get_free_page()))
			return -1; /* Out of memory, see freeing */
		*to_dir = ((unsigned long) to_page_table) | 7;
		nr = (from == 0) ? 0xA0 : 1024;
		for ( ; nr-- > 0; from_page_table++, to_page_table++) {
			this_page = *from_page_table;
			if (!(1 & this_page))
				continue;
			this_page &= ~2;
			*to_page_table = this_page;
			if (this_page > LOW_MEM) {
				*from_page_table = this_page;
				this_page -= LOW_MEM;
				this_page >>= 12;
				mem_map[this_page]++;
			}
		}
	}
	invalidate();
	return 0;
}

/*
 * This function frees a continuos block of page talbes, as needed
 * by 'exit()'. As does copy_page_tables(). This handles only 4Mb blocks.
 */
int free_page_tables(unsigned long from, unsigned long size)
{
	unsigned long *pg_table;
	unsigned long *dir, nr;

	if (from & 0x3fffff)
		panic("free_page_tables called whit wrong alignment");
	if (!from)
		panic("Trying to free up swapper memory space");
	size = (size + 0x3fffff) >> 22;
	dir = (unsigned long *) ((from >> 20) & 0xffc); /* _gp_dir = 0 */
	for ( ; size-- > 0; dir++) {
		if (!(1 & *dir))
			continue;
		pg_table = (unsigned long *) (0xfffff000 & *dir);
		for (nr = 0; nr < 1024 ; nr++) {
			if (1 & *pg_table)
				free_page(0xfffff000 & *pg_table);
			*pg_table = 0;
			pg_table++;
		}
		free_page(0xfffff000 & *dir);
		*dir = 0;
	}
	invalidate();
	return 0;
}
