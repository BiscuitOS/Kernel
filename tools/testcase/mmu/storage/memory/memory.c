/*
 * Main Memory on MMU
 *
 * (C) 2018.06.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/page.h>

#include <test/debug.h>

#ifdef CONFIG_DEBUG_DETECT_MEMORY

#ifdef CONFIG_MEMORY_FROM_BIOS
/*
 * Memory information on BIOS
 *   The address of setup argument is base on empty_zero_page. The 
 *   empty_zero_page holds more information for system setup. And
 *   we can obtain special memory information from it.
 * 
 *   * EXT_MEM_K
 *     Indicate the size for expand main memory. On Linux 1.0, the kernel
 *     always hold lower 1M memory to storage system code and data. And
 *     EXT_MEM_K is memory size that over 1M Byte. We can compute the 
 *     main memory size as follow:
 *       MainMemory = (1 << 20) + (EXT_MEM_K << 10)
 */
extern char empty_zero_page[PAGE_SIZE];
#define PARAM        empty_zero_page
#define EXT_MEM_K    (*(unsigned short *)(PARAM+2))

static int detect_memory_from_BIOS(void)
{
    printk("EXT-Memory(Byte): %#x\n", (EXT_MEM_K<<10));
    printk("MainMemory:       %#x\n", (1<<20)+(EXT_MEM_K<<10));
    return 0;
}
#endif

#ifdef CONFIG_MEMORY_FROM_LD
/*
 * Memory information from ld
 *   The symbol from ld will indicate the kernel section address.
 *   We can use these symbol to indicate the running address for different
 *   section. More ld symbol information see arch/x86/kernel/vmlinux.lds.S
 *
 *   * __executable_start
 *     The symbol indicate the running start address for kernel code
 *     section. It value usually is same with start address of program.
 * 
 *   * etext/_etext/__etext
 *     The symbol indicate the running end address of kernel code. But on
 *     some architecture, the data section is behind of code section, so
 *     this symbol also indicate the running start address of kernel data.
 *
 *   * edata/_edata/__edata
 *     The symbol indicate the running end address for kernel data, But on
 *     common architecture, the bss section is behind of data section, so
 *     this symbol also indicate the running start address of kernel bss.
 *
 *   * end/_end/__end
 *     The symbol indicate the running end address for kernel. 
 *     The kernel usually contain '.text', '.data' and '.bss' section. 
 *     
 *     Kernel .text and .data section
 *     -----------------------------------
 *     |   __executable_start            |
 *     -----------------------------------
 *     |   .code / .text section         |
 *     |                                 |
 *     -----------------------------------
 *     |   etext / _etext / __etext      |
 *     -----------------------------------
 *     |   .data section                 |
 *     |                                 |
 *     -----------------------------------
 *     |   edata / _edata / __edata      |
 *     -----------------------------------
 *     |   .bss section                  |
 *     |                                 |
 *     -----------------------------------
 *     |   end / _end / __end            |
 *     -----------------------------------
 */
static int detect_memory_from_ld(void)
{
    extern char __executable_start[];
    extern char etext[], _etext[], __etext[];
    extern char edata[], _edata[], __edata[];
    extern char end[], __end[];

    printk("Kernel:   %#08x - %#08x\n", 
          (unsigned int)__executable_start, (unsigned int)end);
    printk("Code:     %#08x - %#08x\n", 
          (unsigned int)__executable_start, (unsigned int)etext);
    printk("Data:     %#08x - %#08x\n", 
          (unsigned int)_etext, (unsigned int)edata);
    printk("BSS:      %#08x - %#08x\n", 
          (unsigned int)_edata, (unsigned int)__end);
    printk("__etext:  %#08x\n", (unsigned int)__etext);
    printk("__edata:  %#08x\n", (unsigned int)__edata);

    return 0;
}
#endif

#ifdef CONFIG_MEMORY_FROM_TASK
/*
 * Memory information from task_struct
 *   Task_struct hold current task running inforation that also contain
 *   current task memory information.
 *
 *   * start_code
 *     The current task start address of code section.
 *
 *   * end_code
 *     The current task end address of code section. On common architecture,
 *     the data section is behind of code section, so this value also
 *     indicates the start address of data section.
 *
 *   * end_data
 *     The current task end address of data section.
 *
 *   * start_brk
 *     The current task start address of heap.
 *
 *   * brk
 *     The size of heap for current task.
 *
 *   * start_stack
 *     The start address of stack for current task.
 *
 *   * start_mmap
 *     The start address of mmap for current task.
 */
static int detect_memory_from_task_struct(void)
{
    struct task_struct *task = current;

    printk("Code:     %#08x - %#08x\n", 
          (unsigned int)task->start_code, (unsigned int)task->end_code);
    printk("Data:     %#08x - %#08x\n", 
          (unsigned int)task->end_code, (unsigned int)task->end_data);
    printk("Brk:      %#08x - %#08x\n", 
          (unsigned int)task->start_brk, (unsigned int)task->brk);
    printk("Stack:    %#08x\n", (unsigned int)task->start_stack);
    printk("Mmap:     %#08x\n", (unsigned int)task->start_mmap);

    return 0;
}
#endif

#ifdef CONFIG_MEMORY_FROM_GLOBAL_ARGUMENT
/*
 * Memory information from global argument.
 *   Speical argument indicate the system memory information. We can 
 *   utilize these argment to allocate memory or mmap virtual address.
 *
 *   * high_memory
 *     The start address of high memory. On linux 1.0, if the main
 *     memory size is over 16MB, and the high memory is region what
 *     address over 16MB.
 *
 *   * mem_map
 *     The array for physical page map information. The member of
 *     mem_map holds the speical page whether mapped or reserved.
 *     The system define 3 types: MAP_PAGE_RESERVED, MAP_PAGE_USED and
 *     MAP_PAGE_UNUSED.
 *     MAP_PAGE_RESERVED indicate these page reserved for kernel.
 *     MAP_PAGE_USED indicate these page has mapped.
 *     MAP_PAGE_UNUSED indicate these page doesn't map.
 *
 *   * free_page_list
 *     A single list that hold address for first free page. On free page
 *     list, free_page_list point a free page, and first address of free
 *     page hold next free page. As figure.
 *
 *                       4k--------------         4k--------------
 *                         |            |           |            |
 *                         |    PAGE    |           |    PAGE    |
 *                         |            |           |            |
 *                         |            |           |            |
 *                         |            |           |            |
 *                       4 --------------         4 --------------
 *    free_page_list --->  | free_list  | ------>   | free_list  | --> ....
 *                       0 --------------         0 --------------
 *
 *   * nr_free_pages
 *     The value indicates the free pages on current system.
 */
extern unsigned long high_memory;
extern unsigned short *mem_map;
extern unsigned long free_page_list;
extern int nr_free_pages;

static int detect_memory_from_global_argument(void)
{
    printk("HighMemory:      %08x\n", (unsigned int)high_memory);
    printk("NR free page:    %#08x\n", nr_free_pages);
    printk("First free page: %#08x\n", (unsigned int)free_page_list);
    printk("Mem_map[0x200000] is %s\n", mem_map[MAP_NR(0x200000)] == 0 ?
                              "unused" : "used");
    return 0;
}
#endif

static int memory_detect(void)
{
#ifdef CONFIG_MEMORY_FROM_BIOS
    detect_memory_from_BIOS();
#endif
#ifdef CONFIG_MEMORY_FROM_LD
    detect_memory_from_ld();
#endif
#ifdef CONFIG_MEMORY_FROM_TASK
    detect_memory_from_task_struct();
#endif
#ifdef CONFIG_MEMORY_FROM_GLOBAL_ARGUMENT
    detect_memory_from_global_argument();
#endif
    return 0;
}
late_debugcall(memory_detect);
#endif

#ifdef CONFIG_DEBUG_INIT_MEMORY
static unsigned long pseudo_high_memory = 0;
static unsigned short *pseudo_mem_map = NULL;
static unsigned long pseudo_free_page_list = 0;
static int pseudo_nr_free_pages = 0;
extern char etext[];
extern unsigned long pg0[1024];
static unsigned long pseudo_etext = 0x200000 + (unsigned int)etext;

/*
 * pseudo_mem_init
 *   Initialize special memory region. We assume memory region is from
 *   0x200000 to 0xA00000, and start_low_mem is 0x286000 that is lowest
 *   address what can be used for Kernel/User. The region 0x300000 to
 *   0xA00000 is safe memory that can be mmapped and used for Kernel/User.
 *
 *   -----------------------------------------------------------------
 *   |        |          |          |          |                     |
 *   | Kernel | Reserved | Reserved | MAP AREA | High Memory         |
 *   | .text  | < 640 KB |  Buffer  |          |                     |
 *   | .data  |          |  Video   |          |                     |
 *   -----------------------------------------------------------------
 *
 */
static void pseudo_mem_init(unsigned long start_low_mem,
         unsigned long start_mem, unsigned long end_mem)
{
    int codepages = 0;
    int reservedpages = 0;
    int datapages = 0;
    unsigned long tmp;
    unsigned short *p;

    cli();
    end_mem &= PAGE_MASK;
    pseudo_high_memory = end_mem;
    start_mem +=  0x0000000f;
    start_mem &= ~0x0000000f;
    tmp = MAP_NR(end_mem);
    pseudo_mem_map = (unsigned short *)start_mem;
    p = pseudo_mem_map + tmp;
    start_mem = (unsigned long)p;
    while (p > pseudo_mem_map)
        *--p = MAP_PAGE_RESERVED;
    start_low_mem = PAGE_ALIGN(start_low_mem);
    start_mem = PAGE_ALIGN(start_mem);
    while (start_low_mem < 0x2A0000) { /* Range 640KB */
        pseudo_mem_map[MAP_NR(start_low_mem)] = 0;
        start_low_mem += PAGE_SIZE;
    }
    while (start_mem < end_mem) {
        pseudo_mem_map[MAP_NR(start_mem)] = 0;
        start_mem += PAGE_SIZE;
    }
    pseudo_free_page_list = 0;
    pseudo_nr_free_pages = 0;
    for (tmp = 0x200000; tmp < end_mem; tmp += PAGE_SIZE) {
        if (pseudo_mem_map[MAP_NR(tmp)]) {
            if (tmp >= 0x2A0000 && tmp < 0x300000)
                reservedpages++;
            else if (tmp < (unsigned long)pseudo_etext)
                codepages++;
            else
                datapages++;
            continue;
        }
        *(unsigned long *)tmp = pseudo_free_page_list;
        pseudo_free_page_list = tmp;
        pseudo_nr_free_pages++;
    }
    tmp = pseudo_nr_free_pages << PAGE_SHIFT;
    printk("Memory: %luk/%luk available (%dk kernel code, %dk "
           "reserved, %dk data)\n",
           tmp >> 10,
           end_mem >> 10,
           codepages << (PAGE_SHIFT - 10),
           reservedpages << (PAGE_SHIFT - 10),
           datapages << (PAGE_SHIFT - 10));
    /* test if the WP bit is honoured in supervisor mode */
    wp_works_ok = -1;
    pg0[0] = PAGE_READONLY;
    /* Refresh TLB */
    invalidate();
    __asm__ __volatile__ ("movb 0, %%al ; movb %%al, 0" : : : "ax", "memory");
    pg0[0] = 0;
    invalidate();
    if (wp_works_ok < 0)
        wp_works_ok = 0;
    return;
}

/*
 * pseudo memory initialize
 *   This function used to describe how to initialize special memory
 *   region. We assume the memory region from 2MB to 10MB, and start
 *   address that can used is 3MB. The region 2MB to 3MB contain kernel
 *   code, data and buffer and so on.
 */
static int pseudo_memory_init(void)
{
    /*
     * low_memory_start
     *   The end address of kernel that contain .text, .data and .bss
     *   section. We aassume 'low_memory_start' is 0x286000 
     *   (0x286000 = 0x200000 + 0x86000)
     */
    unsigned long low_memory_start = 0x286000;
    /*
     * memory_start
     *   The start address of memory what can be used. Kernel/User can use
     *   this memory region that over 'memort_start'. We assume the value 
     *   is 0x300000.
     */
    unsigned long memory_start = 0x300000;
    /*
     * memory_end
     *   The last address of memory what can be used. Kernel/User can't
     *   user memory address that over 'memory_end'. We assume the value is
     *   0xA00000.
     */
    unsigned long memory_end = 0xA00000;

    pseudo_mem_init(low_memory_start, memory_start, memory_end);
    return 0;
}
late_debugcall(pseudo_memory_init);
#endif
