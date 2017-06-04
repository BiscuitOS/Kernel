
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
	unsigned long __address;
	unsigned int len = 34;
	int i;

	/*
	 * Case 0
	 * Normal kmalloc routine.
	 */
	for (i = 0; i < 5; i++) {
		__address = (unsigned long)kmalloc(len);
		printk("Allocate memory from memory() %#x\n", __address);
	}
#endif
}

/*
 * Test kfree()
 */
void test_kfree(void)
{
#ifdef CONFIG_TESTCODE_LINUX0_11
	unsigned long __address;
	int len = 34;
	int i;

	/*
	 * Case 0
	 * Normal free routine.
	 */
	for (i = 0; i < 5; i++) {
		__address = (unsigned long)kmalloc(len);
		/* Physical address all same */
		printk("Allocate memory from memory() %#x\n", __address);
		kfree(__address);
	}
	
#endif
}
#endif
