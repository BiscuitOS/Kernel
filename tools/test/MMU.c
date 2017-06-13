
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sched.h>

#ifdef CONFIG_TESTCODE

/*
 * Test get_free_page
 * Get a free page from last physical memory page.
 * If free page exist, return physical address , or return 0.
 * The size of page is 4KB. kernel will get a free page when invoke
 * get_free_page(), it doesn't mapping physical address into virtual 
 * address. 
 */
void test_get_free_page(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11
	unsigned long __address;
	int i;

	/*
	 * Case 0 
	 * Normal get free page.
	 */
	/* Get 5 free PAGEs */
	for (i = 0; i < 5; i++) {
		__address = get_free_page();
		printk("Get[%d] free page physical address %#x\n", i, __address);
	}
	/* Only test non-free */
	;

	/*
	 * Case 1
	 * Request page big then free pages or PAGING_PAGES.
	 * system will swap page 
	 */
	; /* no rotinue test */
#endif
}

/*
 * Test free_page
 */
void test_free_page(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11
	unsigned long __address;

	/*
	 * case 0 
	 * Normal get free page and free it again. The physical address is same
	 * as two times.
	 */

	/* get a free page */
	__address = get_free_page();
	printk("Allocate free page %#x\n", __address);
	/* free page */
	free_page(__address);

	/* get a free page again */
	__address = 0;
	__address = get_free_page();
	printk("Allocate free page again %#x\n", __address);
	free_page(__address);

	/*
	 * Case 1
	 * Free a page that physical address small than LOW_MEM.
	 * The system will ignore it and kernel area doesn't free.
	 */
	 __address = 0x10; /* assume a phyical address */
	 free_page(__address);
	 printk("Free %#x! it's a joke :)\n", __address);

	 /*
	  * Case 2
	  * Free a page that physical address over HIGH_MEMORY.
	  * It will cause panic!
	  */
	  __address = (17 << 20); /* assume a physical address 0x1100000 */
	  free_page(__address);

	  /*
	   * Case 3 
	   * Free a page that have free
	   */
	   __address = 0;
	   __address = get_free_page();
	   free_page(__address);
	   /* free again! panic! */
	   free_page(__address);
#endif
}

/*
 * Test calc_mem()
 */
void test_calc_mem(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11
	extern void calc_mem(void);

	calc_mem();	
#endif
}

/*
 * Test copy_page_tables()
 */
void test_copy_page_table(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11
	unsigned long old_data_base;
	unsigned long new_data_base = 4 * 0x4000000;
	unsigned long data_limit;

	data_limit = get_limit(0x17);
	old_data_base = get_base(current->ldt[2]);

	copy_page_tables(old_data_base, new_data_base, data_limit);
#endif
}

/*
 * Test kmalloc()
 */
void test_kmalloc(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11

#define DEBUG_KMALLOC_CASE0 0 
#define DEBUG_KMALLOC_CASE1 0
#define DEBUG_KMALLOC_CASE2 0
#define DEBUG_KMALLOC_CASE3 0
#define DEBUG_KMALLOC_CASE4 1

#if DEBUG_KMALLOC_CASE0

	/*
	 * Case 0
	 * Normal kmalloc routine.
	 */
	unsigned long __address;
	unsigned int len = 34;

	__address = (unsigned long)kmalloc(len);
	printk("Allocate memory from kmalloc() %#x\n", __address);
	/* non-kfree */
#elif DEBUG_KMALLOC_CASE1
	
	/*
	 * Case 1
	 * Request size larger than Max bucket size.
	 */
	unsigned long __address;
	unsigned long len = 4097; /* MAX is 4096 */

	__address = (unsigned long)kmalloc(len);
	printk("Allocate memory from kmalloc() %#x\n", __address);
#elif DEBUG_KMALLOC_CASE2

	/*
	 * Case 2
	 * Kmalloc can't get memory from get_free_page() - OOM
	 * OOM in init_bucket_desc()
	 */
	unsigned long __address;
	unsigned long max_paging_pages = ((15 << 20) >> 12);
	unsigned long len = 32;
	int i;

	/* waste all free pages */
	for (i = 0; i < max_paging_pages; i++) 
		__address = get_free_page();
	
	__address = (unsigned long)kmalloc(len);
	printk("Allocate memory from kmalloc() %#x\n", __address);
#elif DEBUG_KMALLOC_CASE3

	/*
	 * Case 3
	 * Kmalloc can't get free memory, 'freeptr' and 'page' can't get 
	 * free page on struct bucket_desc.
	 */
	unsigned long __address;
	unsigned long max_paging_pages = ((11 << 20) >> 12); /* 11M for mapping */
	unsigned long len = 32;
	unsigned long last_pages = 224; /* statistics */
	int i;

	for (i = 0; i < max_paging_pages + last_pages; i++)
		__address = get_free_page();

	/* Only reserved a free page for init_bucket_desc() */
	free_page(__address);

	/* failed request memory for 'freeptr' and 'page' on struct bucket_desc */
	__address = (unsigned long)kmalloc(len);
	printk("Allocate memory from kmalloc() %#x\n", __address);
#elif DEBUG_KMALLOC_CASE4

	/* 
	 * Case 4
	 * Utilize hole memory
	 */
	unsigned long __address0, __address1;
	unsigned long size = 24;

	/* allocate memory */
	__address0 = (unsigned long)kmalloc(size);
	__address1 = (unsigned long)kmalloc(size);
	printk("Area[%#x - %#x] HOLE [%#x - %#x]\n", __address0, __address1,
			(unsigned long)((char *)__address0 + size), __address1);
	__address0 = (unsigned long)((char *)__address0 + size);
	/* Utilize hole area */
	*((char *)__address0) = 'A';
	*((char *)__address0 + 1) = 'B';
	*((char *)__address0 + 2) = 'C';
	*((char *)__address0 + 3) = '\0';
	printk("Hole area %s\n", __address0);
#endif
#endif
}

/*
 * Test kfree()
 */
void test_kfree(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11
#define DEBUG_KFREE_CASE0    0
#define DEBUG_KFREE_CASE1    0
#define DEBUG_KFREE_CASE2    0
#define DEBUG_KFREE_CASE3    1
#define DEBUG_KFREE_CASE4    0

#if DEBUG_KFREE_CASE0
	/*
	 * Case 0
	 * Normal free routine.
	 */
	unsigned long __address;
	int len = 34;
	int i;

	for (i = 0; i < 5; i++) {
		__address = (unsigned long)kmalloc(len);
		/* Physical address all same */
		printk("Allocate memory from memory() %#x\n", __address);
		kfree(__address);
	}
#elif DEBUG_KFREE_CASE1
	/*
	 * Case 1
	 * unknown address.
	 */
	unsigned long __address;
	int len = 34;

	__address = (unsigned long)kmalloc(len);
	__address += PAGE_SIZE * 4;
	printk("Alter allocated address is %#x\n", __address);
	kfree(__address);
#elif DEBUG_KFREE_CASE2
	/*
	 * Case 2
	 * Free size overflow bucket size.
	 */
	unsigned long __address;
	int len = 34;

	__address = (unsigned long)kmalloc(len);
	printk("Allocate memory from memory() %#x\n", __address);
	free_s((void *)__address, len * 2);
#elif DEBUG_KFREE_CASE3
	/*
	 * Case 3
	 * Invalid free size.
	 * Max bucket size is 4096.
	 */
	unsigned long __address;
	int len = 4096;

	__address = (unsigned long)kmalloc(len);
	printk("Allocate memory from memory() %#x\n", __address);
	free_s((void *)__address, 4097);
#elif DEBUG_KFREE_CASE4
	/*
	 * Case 4
	 * Malloc bucket chanis corrupted.
	 */
#endif
	
#endif
}
#endif
