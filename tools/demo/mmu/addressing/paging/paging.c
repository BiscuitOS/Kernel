/*
 * Paging mechanism
 *
 * (C) 2018.11.10 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/sched.h>
#include <linux/page.h>
#include <linux/malloc.h>
#include <linux/unistd.h>

#include <demo/debug.h>

#ifdef CONFIG_DEBUG_PAGING_MODE
static int __unused paging_mode(void)
{
    unsigned int __unused mode;
    unsigned int __unused CR0;
    unsigned int __unused CR4;
    unsigned int __unused IA32_EFER;

    /* Obtain CR0 Register */
    __asm__ ("mov %%cr0, %0" : "=r" (CR0));

    /* Obtain CR4 Register */
    __asm__ ("mov %%cr4, %0" : "=r" (CR4));

    /* Obtain IA32_EFER, not support IA32e */
    IA32_EFER = 0;

#ifdef CONFIG_DEBUG_PAGING_MODE_DETECT
    /*
     * 32-bit Paging mode
     *   If CR0.PG = 1 and CR4.PAE = 0, 32-bit paging is used.
     */
    if (((CR0 >> 31) & 0x1) && !((CR4 >> 5) & 0x1)) {
        printk("32-bit Paging Modes.\n");
    } else if (((CR0 >> 31) & 0x1) && ((CR4 >> 5) & 0x1) && 
              !((IA32_EFER >> 8) & 0x1)) {
        /*
         * PAE Paing mode
         *   If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 0, PAE paging is
         *   used.
         */
        printk("PAE Paging Mode.\n");
    } else if (((CR0 >> 31) & 0x1) && ((CR4 >> 5) & 0x1) &&
              ((IA32_EFER >> 8) & 0x1)) {
        /*
         * 4-level Paging mode.
         *   If CR0.PG = 1, CR4.PAE = 1, and IA32_EFER.LME = 1, 4-level paging
         *   is used.
         */
        printk("4-level Paging Mode.\n");
    } else
        printk("Unknow Paging Mode.\n");

#endif

    return 0;
}
late_debugcall(paging_mode);
#endif

#ifdef CONFIG_DEBUG_PDE_BITMAP_4M
/*
 * Format of a 32-bit Page-Directory Entry that Maps a 4-MByte Page.
 */
static int __unused pde_4M_bitmap(unsigned long pde)
{
    return 0;
}
#endif

#ifdef CONFIG_DEBUG_PDE_BITMAP_4K
/*
 * Format of a 32-bit Page-Directory Entry References a Page Table.
 */
static int __unused pde_4K_bitmap(unsigned long pde)
{
#ifdef CONFIG_DEBUG_PDE_4KBP_P
    /*
     * P flag (bit 0 on PDEs)
     *   Present; must be 1 to reference a page table.
     */
    if ((pde >> 0) & 0x1)
        printk("Reference a 4-KByte Page table.\n");
    else
        printk("Reference a non-exist 4-KByte Page table.\n");
#endif

#ifdef CONFIG_DEBUG_PDE_4KBP_RW
    /*
     * R/W flag (bit 1 on PDEs)
     *   Read/write. If 0, writes may not be allowed to the 4-KByte region
     *   controlled by this entry.
     */
    if ((pde >> 1) & 0x1)
        printk("Read and write are allowed to the 4-KByte page.\n");
    else
        printk("Write may not be allowed to the 4-KByte page.\n");
#endif

#ifdef CONFIG_DEBUG_PDE_4KBP_US
    /*
     * U/S flag (bit 2 on PDEs)
     *   User/Supervisor. If 0, user-mode accesses are not allowed to the 
     *   4-KByte region controlled by this entry.
     */
    if ((pde >> 2) & 0x1)
        printk("User-mode accesses are allowed to the 4-KByte page.\n");
    else
        printk("Supervisor-mode accesses are allowed to the 4-KByte page.\n");
#endif

#ifdef CONFIG_DEBUG_PDE_4KBP_PWT
    /*
     * PWT flag (bit 3 on PDEs)
     *   Page-level write-through. Indirectly determines the memory type used
     *   to access the page table referenced by this entry.
     */
    if ((pde >> 3) & 0x1)
        printk("Enable Page-level write-through.\n");
    else
        printk("Disable Page-level write-through.\n");
#endif

#ifdef CONFIG_DEBUG_PDE_4KBP_PCD
    /*
     * PCD flag (bit 4 on PDEs)
     *   Page-level cache disable. Indirectly determines the memory type used
     *   to access the page table referenced by this entry.
     */
    if ((pde >> 4) & 0x1)
        printk("Disable Page-level cache.\n");
    else
        printk("Enable Page-level cache.\n");
#endif

#ifdef CONFIG_DEBUG_PDE_4KBP_A
    /*
     * A flag (bit 5 on PDEs)
     *   Accessed. Indicates whether this entry has been used for linear-
     *   address translation.
     */
    if ((pde >> 5) & 0x1)
        printk("PDEs has been used for linear-address translation.\n");
    else
        printk("PDEs hasn't been used for linear-address translation.\n");
#endif

#ifdef CONFIG_DEBUG_PDE_4KBP_PS
    /*
     * PS flag (bit 7 on PDEs)
     *   If CR4.PSE = 1, must be 0 (otherwise, this entry maps a 4-MByte page)
     *   Otherwise, ignored.
     */
    printk("PDEs on 4-KByte Page ignored this bit.\n");
#endif

#ifdef CONFIG_DEBUG_PDE_4KBP_AD
    /*
     * Physical address of 4-KByte alignment page table referenced by this 
     * entry.
     */
    printk("Physical address: %#x\n", (unsigned int)(pde >> 12));
#endif

    return 0;
};
#endif

#ifdef CONFIG_DEBUG_PTE_BITMAP
static int __unused pte_bitmap(unsigned long pte)
{
#ifdef CONFIG_DEBUG_PTE_BP_P
    /*
     * P flag (bit 0 on PTE)
     *   Present, must be 1 to map a 4-KByte page.
     */
    if (pte & 0x1)
        printk("4-KByte Page Present.\n");
    else
        printk("4-KByte Page not Present.\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_RW
    /*
     * R/W flag (bit 1 on PTE)
     *   Read/Write. If 0, writes may not be allowed to the 4-KByte page 
     *   referenced by this entry.
     */
    if ((pte >> 1) & 0x1)
        printk("Read and write to 4-KByte Page.\n");
    else
        printk("Write not be allowed to the 4-KByte Page.\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_US
    /*
     * U/S flag (bit 2 on PTE)
     *   User/Supervisor. If 0, user-mode accesses are not allowed to the 
     *   4-KByte page referenced by this entry.
     */
    if ((pte >> 2) & 0x1)
        printk("User-mode accesses are allowed to the 4-KByte page.\n");
    else
        printk("User-mode accesses are not allowed to the 4-KByte page.\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_PWT
    /*
     * PWT flag (bit 3 on PTE)
     *   Page-level write-through. Indirectly determines the memory type used
     *   to access the 4-KByte page referenced by this entry.
     */
    if ((pte >> 3) & 0x1)
        printk("Enable page-level write-through.\n");
    else
        printk("Disable page-level write-through.\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_PCD
    /*
     * PCD flag (bit 4 on PTE)
     *   Page-level cache disable. Indirectly determines the memory type used
     *   to access the 4-KByte page referenced by this entry.
     */
    if ((pte >> 4) & 0x1)
        printk("Disable page-level cache.\n");
    else
        printk("Enable page-level cache.\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_A
    /*
     * A flag (bit 5 on PTE)
     *   Accessed. Indicates whether software has accessed the 4-KByte page
     *   referenced by this entry.
     */
    if ((pte >> 5) & 0x1)
        printk("4-KByte Page has accessed.\n");
    else
        printk("4-KByte Page hasn't accessed.\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_D
    /*
     * D flag (bit 6 on PTE)
     *   Dirty. Indicates whether software has written to the 4-KByte page
     *   referenced by this entry.
     */
    if ((pte >> 6) & 0x1)
        printk("4-KByte has dirty.\n");
    else
        printk("4-KByte hasn't dirty.\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_PAT
    /*
     * PAT flag (bit 7 on PTE)
     *   If the PAT is supported, indirectly determines the memory type used
     *   to access the 4-KByte page referenced by this entry; Otherwise, 
     *   reserved.
     */
    if ((pte >> 7) & 0x1)
        printk("Support PAT on PTE.\n");
    else
        printk("Reserved.\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_G
    /*
     * G flag (bit 8 on PTE)
     *   Global. If CR4.PGE = 1, determines whether the translation is global.
     *   ignore otherwise.
     */
    if ((pte >> 8) & 0x1)
        printk("Translation is global.\n");
    else
        printk("Ignored\n");
#endif

#ifdef CONFIG_DEBUG_PTE_BP_AD
    /*
     * Physical address field (bits 12 through 31)
     *   Physical address of the 4-KByte page referenced by this entry.
     */
    printk("Page base physical address: %#x\n", 
                            (unsigned int)((pte >> 12) & 0xFFFFF) << 12);
#endif

    return 0;
}
#endif

static int __unused paging_32bit(unsigned long linear)
{
    unsigned long __unused *pgdir;
    unsigned long __unused *pde;
    unsigned long __unused *pte;
    unsigned char __unused *page;
    unsigned long __unused *PDE;
    unsigned long __unused *PTE;
    unsigned long __unused *PAGE;
    unsigned long __unused CR3;
    unsigned long __unused CR4;
    unsigned long __unused CR0;
    unsigned long __unused *physical;

    pgdir = (unsigned long *)current->tss.cr3;

    /*
     * A 4-KByte naturally alignment page directory is located at the physical
     * address specified in bit 31:12 of CR3. A page directory comprise 1024 
     * 32-bit entries (PDEs). A PDE is selected using the physical address
     * defined as follow:
     *
     * -- Bits 39:32 are all 0.
     * -- Bits 31:12 are from CR3.
     * -- Bits 11:2 are bits 31:22 of the linear address.
     * -- Bits 1:0 are 0.
     *
     *
     * CR3
     * 31                    12
     * +-----------------------+----+
     * | Base Physical Address |    |     
     * +-----------------------+----+
     *           |
     *           |
     *           |             Linear address
     *           |             31         22  
     *           |             +------------+---------------+
     *           |             | PDE offset |               |
     *           |             +------------+---------------+
     *           |                   |
     *           |                   |
     * PDE       |                   |
     * 31        V            12     V     2     0
     * +-----------------------+------------+----+
     * |                       |            | 00 |
     * +-----------------------+------------+----+
     *
     */
    __asm__ ("mov %%cr3, %0" : "=r" (CR3));
    PDE = (unsigned long *)(CR3 & 0xFFFFF000);
    PDE = (unsigned long *)((unsigned long)PDE | 
                           (((linear >> 22) & 0x3FF) << 2));
    PDE = (unsigned long *)((unsigned long)PDE & 0xFFFFFFFC);

    /* pdt points to PDEs */
    pde = &pgdir[linear >> 22];

    if (pde != PDE)
        panic("PDE != pde");

    /*
     * Because a PDE is identified using bits 31:22 of the linear address, it 
     * controls access to 4-MByte region of the linear-address space. Use of
     * the PDE depends on CR4.PSE and the PDE's PS flag (bit 7):
     */
    __asm__ ("mov %%cr4, %0" : "=r" (CR4));
    if (((CR4 >> 4) & 0x1) && ((*pde >> 7) & 0x1)) {
        /*
         * If CR4.PSE = 1 and the PDE's PS flag is 1, the PDE maps a 4-MByte
         * page. The final physical address is computed as follows.
         *
         * -- Bits 39:32 are bits 20:13 of the PDE.
         * -- Bits 31:22 are bits 31:22 of the PDE.
         * -- Bits 21:0 are from the original linear address
         */

#ifdef CONFIG_DEBUG_PDE_BITMAP_4M
        pde_4M_bitmap(*pde);
#endif

    } else if (!((CR4 >> 4) & 0x1) && !((*pde >> 7) & 0x1)) {
#ifdef CONFIG_DEBUG_PDE_BITMAP_4K
        pde_4K_bitmap(*pde);
#endif
        /*
         * If CR4.PSE = 0 or the PDE's PS flag is 0, a 4-KByte naturally 
         * aligned page table is located at the physical address specified in
         * bits 31:12 of the PDE. A page table comprises 1024 32-bit entries (
         * PTEs). A PTE is selected using the physical address defined as 
         * follows:
         * 
         * -- Bits 31:12 are from the PDE
         * -- Bits 11:2 are bit 21:12 of the linear address.
         * -- Bits 1:0 are 0.
         *
         *
         * PDE
         * 31                    12
         * +-----------------------+----+
         * | Base Physical Address |    |     
         * +-----------------------+----+
         *           |
         *           |
         *           |   Linear address
         *           |   31            21        12        0
         *           |   +------------+------------+-------+
         *           |   |            | PTE offset |       |
         *           |   +------------+------------+-------+
         *           |                   |
         *           |                   |
         * PTE       |                   |
         * 31        V            12     V     2     0
         * +-----------------------+------------+----+
         * |                       |            | 00 |
         * +-----------------------+------------+----+
         *
         */
        /* Relocate first page address of PTE */
        PTE = (unsigned long *)(*PDE & 0xFFFFF000);
        PTE = (unsigned long *)((unsigned long)PTE | 
                                (((linear >> 12) & 0x3FF) << 2));
        PTE = (unsigned long *)((unsigned long)PTE & 0xFFFFFFFC);

        /* Obtain PTE page base address */
        pde = (unsigned long *)(*pde & 0xFFFFF000);
        /* pte points to Page table */
        pte = &pde[(linear >> 12) & 0x3FF];

        /* Verify pte and PTE */
        if (pte != PTE)
            panic("PTE != pte");

    }

#ifdef CONFIG_DEBUG_PTE_BITMAP
    pte_bitmap(*pte);
#endif

    /*
     * Because a PTE is identified using bits 31:12 of the linear address,
     * every PTE maps a 4-KByte page. The final physical address is computed
     * as follows:
     *
     * -- Bits 31:12 are from the PTE.
     * -- Bits 11:0 are from the original linear address.
     *
     *
     * PTE
     * 31                    12
     * +-----------------------+----+
     * | Base Physical Address |    |     
     * +-----------------------+----+
     *           |
     *           |
     *           |  Linear address
     *           |  31         12  
     *           |  +------------+---------------+
     *           |  |            | Original addr |
     *           |  +------------+---------------+
     *           |                   |
     *           |                   |
     * Physical  |                   |
     * 31        V            12     V           0
     * +-----------------------+-----------------+
     * |                       |                 |
     * +-----------------------+-----------------+
     *
     *
     */
    PAGE = (unsigned long *)(*PTE & 0xFFFFF000);
    PAGE = (unsigned long *)((unsigned long)PAGE | (linear & 0xFFF));

    /* Obtain page address from pte */
    page = (unsigned char *)(*pte & 0xFFFFF000);
    physical = (unsigned long *)&page[linear & 0xFFF];

    if (PAGE != physical)
        panic("PAGE != physical");

    printk("Linear-addr:   %#lx\n", linear);
    printk("Physical-addr: %#lx\n", (unsigned long)physical);
    printk("Translation:   %s\n", (char *)physical);
    return 0;
}

#ifdef CONFIG_DEBUG_32BIT_PAGING_USER
asmlinkage int sys_demo_paging(unsigned long linear)
{
    paging_32bit(linear);

    return 0;
}

/* System call entry */
inline _syscall1(int, demo_paging, unsigned long, linear);
#endif

static int __unused paging_32bit_entence(void)
{
    unsigned long __unused virtual;
    unsigned long __unused logical;
    unsigned long __unused linear;
    unsigned short __unused SS;
    struct desc_struct __unused *desc;
    const char __unused *hello = "Hello biscuitOS";

    /* Obtain virtual address for 'hello' */
    virtual = (unsigned long)hello;

    /* Obtain logical address: SS: offset */
    __asm__ ("mov %%ds, %0" : "=m" (SS));

    /* Obtain linear address */
    if ((SS >> 2) & 0x1)
        desc = current->ldt + (SS >> 3);
    else
        desc = gdt + (SS >> 3);
    linear = get_base(*desc) + virtual;

#ifdef CONFIG_DEBUG_32BIT_PAGING_USER
    demo_paging(linear);
#elif defined CONFIG_DEBUG_32BIT_PAGING_KER
    paging_32bit(linear);
#endif

    return 0;
}
#ifdef CONFIG_DEBUG_32BIT_PAGING_USER
user1_debugcall_sync(paging_32bit_entence);
#elif defined CONFIG_DEBUG_32BIT_PAGING_KER
late_debugcall(paging_32bit_entence);
#endif

#ifdef CONFIG_DEBUG_MEM_ESTABLISH

/*
 * Establish first page table.
 */
static void __unused paging_table_boot(void)
{
    /*
     * 0          4k         8k
     * +----------+----------+----------+----------+----------+
     * |          |          |          |          |          |
     * |  page 0  |  pg_dir  |  page 1  | .....    |          |
     * |          |          |          |          |          |
     * +----------+----------+----------+----------+----------+
     */
    __asm__ ("movl $1024*2, %%ecx\n\r" /* 2 pages-swapper_pg_dir+1 page table*/
             "xorl %%eax, %%eax\n\r"
             /* swapper_pg_dir is at 0x1000 */
             "movl $swapper_pg_dir, %%edi\n\r"
             "cld; /*rep;stosl*/\n\r"
             /* Identity-map the kernel in low 4MB memory for ease of 
              * transition. */
             /* set present bit/user r/w */
             "movl $pg0+7, swapper_pg_dir\n\r"
             /* But the real place is at 0xC0000000, set present bit/user 
              * r/w */
             "movl $pg0+7, swapper_pg_dir+3072\n\r"
             /* Points to last item on first page table */
             "movl $pg0+4092, %%edi\n\r"
             /* 4Mb - 4096 + 7 (r/w user, p) */
             "movl $0x03ff007, %%eax\n\r"
             "std\n\r"
             /* fill the page backwards - more efficitent :-) */
             "1: stosl\n\r"
             "subl $0x1000, %%eax\n\r"
             "jge 1b\n\r"
             "cld\n\r"
             "movl $swapper_pg_dir, %%eax\n\r"
             /* cr3 = page directory start */
             "movl %%eax, %%cr3\n\r"
             "movl %%cr0,%%eax\n\r"
             "orl $0x80000000, %%eax\n\r"
             /* set paging (PG) bit */
             "movl %%eax,%%cr0"
              ::: "cx" , "memory");
}

/*
 * Establish Paging table which range from 0x0 to memory_end.
 *
 * +------------+ 4K
 * |            |
 * +------------+
 * |            |
 * +------------+
 * |            |
 * +------------+
 * |    768    -|------o
 * +------------+      |
 * |            |      |
 * +------------+      |
 * |            |      |          +-----------------+ 4K
 * +------------+      |          |                 |
 * |            |      |          +-----------------+
 * +------------+      |          |                -|---------> 4K_Page
 * |            |      |          +-----------------+
 * +------------+      |          |                 |
 * |            |      |          +-----------------+
 * +------------+      |          |                 |
 * |            |      |          +-----------------+
 * +------------+      o--------->|                 |
 * |     0     -|---------------->+-----------------+ 0
 * +------------+ 0
 *
 */
static int __unused paging_table_first(unsigned long memory_start,
                             unsigned long memory_end)
{
    unsigned long *pg_dir;
    unsigned long *pg_table;
    unsigned long tmp;
    unsigned long address;

    memory_start = PAGE_ALIGN(memory_start);
    address = 0;
    pg_dir = swapper_pg_dir;
    while (address < memory_end) {
        tmp = *(pg_dir + (0xC0000000 >> 22)); /* linear 0xC0000000 + 10M */
        if (!tmp) {
            /* Page directry item is empty. */
            tmp = memory_start | PAGE_TABLE;
            *(pg_dir + (0xC0000000 >> 22)) = tmp;
            memory_start += PAGE_SIZE;
        }
        /* Also map it in at 0x00000000 for init */
        *pg_dir = tmp;
        pg_dir++;
        pg_table = (unsigned long *)(tmp & PAGE_MASK);
        for (tmp = 0; tmp < PTRS_PER_PAGE; tmp++, pg_table++) {
            if (address < memory_end)
                *pg_table = address | PAGE_SHARED;
            else
                *pg_table = 0;
            address += PAGE_SIZE;
        }
    }
    invalidate();

    return memory_start;
}

/*
 * #define PAGE_SHIFT   12
 * #define MAP_NR(addr) ((addr) >> PAGE_SHIFT) 
 *
 *
 *
 * +------------------+-+-+-+-+-+-+-+-+-+----+-+-+-----------------------+
 * |                  | | | | | | | | | |    | | |                       |
 * |                  | | | | | | | | | | .. | | |                       |
 * |                  | | | | | | | | | |    | | |                       |
 * +------------------+-+-+-+-+-+-+-+-+-+----+-+-+-----------------------+
 *                    A                          A
 *                    |                          |
 *                    |                          |
 * memory_start0------o        memory_start1-----o
 *                    |
 * mem_map------------o
 * 
 *
 *
 *
 */
extern unsigned long pg0[1024];
static int __unused memory_mapping(unsigned long low_memory_start,
               unsigned long memory_start, unsigned long memory_end)
{
    int codepages = 0;
    int reservedpages = 0;
    int datapages = 0;
    unsigned long tmp;
    unsigned short *p;
    extern char etext[];
    unsigned short __unused *memory_map = NULL;
    unsigned long __unused free_pages_list;
    unsigned long __unused free_pages_nr;

    cli();
    memory_end &= PAGE_MASK;
    high_memory = memory_end;
    /* Aligned with (unsigned short) */
    memory_start += 0x0000000f;
    memory_start &= ~0x0000000f;
    /*
     * Establish a physical page map list.
     *
     *
     *
     *            | <-- MEM_NR(memory_end) -> |
     *            |     + memory_map          |
     * +--------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-------------------+
     * |        | | | | | | | | | | | | | | | | |                   |
     * | Kernel | | | | | | | | | | | | | | | | |                   |
     * |        | | | | | | | | | | | | | | | | |                   |
     * +--------+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-------------------+
     *            A                             A                   A
     *            |                             |                   |
     *            |                             |                   |
     *            |                             |                   |
     * memory_map o                memory_start-o       memory_end--o
     *
     */
    tmp = MAP_NR(memory_end);
    memory_map = (unsigned short *)memory_start;
    p = memory_map + tmp;
    memory_start = (unsigned long)p;
    /* Mark all physical page as RESERVED */
    while (p > memory_map)
        *--p = MAP_PAGE_RESERVED;
    /* Alignment to next page. */
    low_memory_start = PAGE_ALIGN(low_memory_start);
    memory_start = PAGE_ALIGN(memory_start);
    /*
     * Mark physical page which range from low_memory_start to 0xA0000 as free.
     */
    while (low_memory_start < 0xA0000) {
        memory_map[MAP_NR(low_memory_start)] = 0;
        low_memory_start += PAGE_SIZE;
    }
    /*
     * Mark physical page which range from memory_start to memory_end.
     */
    while (memory_start < memory_end) {
        memory_map[MAP_NR(memory_start)] = 0;
        memory_start += PAGE_SIZE;
    }
    /*
     * Here, the layout of memory_map[]
     *
     * +----------+-------------------------------+
     * |          |                               |
     * | Reserved |          free page            |
     * |          |                               |
     * +----------+-------------------------------+
     */

    free_pages_list = 0;
    free_pages_nr = 0;
    for (tmp = 0; tmp < memory_end; tmp += PAGE_SIZE) {
        if (memory_map[MAP_NR(tmp)]) {
            if (tmp >= 0xA0000 && tmp < 0x100000)
                reservedpages++;
            else if (tmp < (unsigned long)&etext)
                codepages++;
            else
                datapages++;
            continue;
        }
        /* list all free pages 
         *
         * +--------+      +--------+      +--------+      +--------+
         * |        |      |        |      |        |      |        |
         * +--------+      +--------+      +--------+      +--------+
         * |        |      |        |      |        |      |        |
         * +--------+      +--------+      +--------+      +--------+
         * |        |      |        |      |        |      |        |
         * +--------+      +--------+      +--------+      +--------+
         * |        |      |        |      |        |      |        |
         * +--------+      +--------+      +--------+      +--------+
         * |        |      |        |      |        |      |        |
         * +--------+      +--------+      +--------+      +--------+
         * |        |      |        |      |        |      |        |
         * +--------+      +--------+      +--------+      +--------+
         * |        |      |        |      |        |      |        |
         * +--------+      +--------+      +--------+      +--------+
         * |   0    |<-----|-       |<-----|-       |<-----|-       |<---o
         * +--------+      +--------+      +--------+      +--------+    |
         * 0               4K              8K              12K           |
         *                                                               |
         *                                                               |
         *                                                               |
         *                                             free_pages_list---o
         */
        *(unsigned long *)tmp = free_pages_list;
        free_pages_list = tmp;
        free_pages_nr++;
    }
    tmp = free_pages_nr << PAGE_SHIFT;

    return 0;
}

/*
 * Layout of zero page.
 *
 * +----------+------------------------------------------------------+
 * |  Offset  | Describe                                             |
 * +----------+------------------------------------------------------+
 * |  0x0002  | Extended memory size (over 1MByte memory)            |
 * +----------+------------------------------------------------------+
 */
extern char empty_zero_page[PAGE_SIZE];

/* End address of kernel code semgent */
extern char end[];

/*
 *
 * | <------ 1M ------> |
 * +-------------+------+------------------+------+-------------+
 * |             |      |                  |      |             |
 * | Kernel code | hole |    Low momery    | hole | High memory |      
 * |             |      |                  |      |             |
 * +-------------+------+------------------+------+-------------+
 * 0             A      A
 *               |      |
 *               |      |
 * end ----------o      |
 *                      |
 *    start_mem --------o
 */
static int __unused paging_mem_init(void)
{
    unsigned long __unused low_memory_start;
    unsigned long __unused memory_start;
    unsigned long __unused memory_end;
    unsigned long __unused extend_memory;

#ifdef CONFIG_DEBUG_MEM_PAGING_FIRST
    paging_table_boot();
#endif

    /* Obtain extend memory over 1-MByte. */
    extend_memory = *(unsigned short *)(empty_zero_page + 2);
    
    /* Obtain the end memory: 1-MByte + extended memory */
    memory_end = (1 << 20) + (extend_memory << 10);
    memory_end &= PAGE_MASK;

    /* Only use lower 16 MBytes memory */
    if (memory_end > (16 * 1024 * 1024))
        memory_end = 16 * 1024 * 1024;

    /* If kernel code segment over 1MByte, the first low memory boundary with
     * next page. */
    if ((unsigned long)end >= (1024 * 1024)) {
        memory_start = (unsigned long)end;
        low_memory_start = PAGE_SIZE;
    } else {
        memory_start = 1024 * 1024;
        low_memory_start = (unsigned long)end;
    }
    low_memory_start = PAGE_ALIGN(low_memory_start);

#ifdef CONFIG_DEBUG_MEM_PAGING_TABLE
    memory_start = paging_table_first(memory_start, memory_end);
#endif
#ifdef CONFIG_DEBUG_MEM_MAPPING
    memory_mapping(low_memory_start, memory_start, memory_end);
#endif

    return 0;
}
arch_debugcall(paging_mem_init);
#endif

#ifdef CONFIG_DEBUG_PAGE_TABLE_COPY
static int __unused page_table_copy(void)
{
    unsigned long __unused old_pg_dir, *old_page_dir;
    unsigned long __unused new_pg_dir, *new_page_dir;
    struct task_struct __unused *tsk;
    int i;

    /* Establish a new task */
    if (!(tsk = (struct task_struct *) get_free_page(GFP_KERNEL)))
        return -ENOMEM;

    /* Estalish a new page dirent */
    if (!(new_pg_dir = get_free_page(GFP_KERNEL)))
        return -ENOMEM;

    old_pg_dir = current->tss.cr3;
    tsk->tss.cr3 = new_pg_dir;

    /*
     * Obtain the base address of page dirent.
     *
     *
     *                  Old page dirent                  New page dirent
     *                  +----------+                     +----------+
     *                  |          |                     |          |
     *                  +----------+                     +----------+
     *                  |          |                     |          |
     *                  +----------+                     +----------+
     *                  |          |                     |          |
     *                  +----------+                     +----------+
     *                  |          |                     |          |
     *                  +----------+                     +----------+
     *                  |          |                     |          |
     *                  +----------+                     +----------+
     * old_page_dir     |          |                     |          |
     * o--------------->+----------+    new_page_dir---->+----------+
     * |
     * |      CR3
     * |      +--------------------------+
     * o------|-       old_pg_dir        |            
     *        +--------------------------+
     * 
     * 
     */
    old_page_dir = (unsigned long *) old_pg_dir;
    new_page_dir = (unsigned long *) new_pg_dir;

    for (i = 0; i < PTRS_PER_PAGE; i++, old_page_dir++, new_page_dir++) {
        unsigned long old_pg_table, *old_page_table;
        unsigned long new_pg_table, *new_page_table;
        int j;

        /* Obtain the value of page dirent 
         *
         *
         *                  Old page dirent
         *                  +------------------+
         *                  |                  |
         *                  +------------------+
         *                  |                  |
         *                  +------------------+
         *                  |                  |
         *                  +------------------+
         *                  |                  |
         *                  +------------------+
         *                  |                  |
         *                  +------------------+
         *                  |                  |
         *                  +------------------+
         *                  |                  |
         *                  +------------------+
         * old_page_dir     |   old_pg_table   |
         * o--------------->+------------------+
         * |
         * |                CR3
         * |                +--------------------------+
         * o----------------|-       old_pg_dir        |            
         *                  +--------------------------+
         */
        old_pg_table = *old_page_dir;
        /* Doesn't copy empty page table. */
        if (!old_pg_table)
            continue;
        if (old_pg_table >= high_memory || !(old_pg_table & PAGE_PRESENT)) {
            printk("page_table_copy: bad page table: "
                       "probable memory corruption\n");
            *old_page_dir = 0;
            continue;
        }
        /* Only copy all Reserved page dirent */
        if (mem_map[MAP_NR(old_pg_table)] & MAP_PAGE_RESERVED) {
            *new_page_dir = old_pg_table;
            continue;
        }

        /* Allocate new memory to new page table */
        if (!(new_pg_table = get_free_page(GFP_KERNEL))) {
            /* Free register pages. */
            return -ENOMEM;
        }
        /* Obtain the base address of page table 
         *
         *                                                    Old Page Table
         *                  Old Page Dirent                   +-----------+
         *                  +---------------+                 |           |
         *                  |               |                 +-----------+
         *                  +---------------+                 |           |
         *                  |               |                 +-----------+
         *                  +---------------+                 |           |
         *                  |               |                 +-----------+
         *                  +---------------+                 |           |
         *                  |               |                 +-----------+
         *                  +---------------+                 |           |
         *                  |               |                 +-----------+
         *                  +---------------+                 |           |
         *                  |               |                 +-----------+
         *                  +---------------+                 |           |
         *                  |               |                 +-----------+
         *                  +---------------+ old_page_table  |    pg     |
         * old_page_dir     | old_pg_table -|---------------->+-----------+
         * o--------------->+---------------+
         * |
         * |                CR3
         * |                +--------------------------+
         * o----------------|-       old_pg_dir        |            
         *                  +--------------------------+
         *
         */ 
        old_page_table = (unsigned long *)(PAGE_MASK & old_pg_table);
        new_page_table = (unsigned long *)(PAGE_MASK & new_pg_table);

        for (j = 0; j < PTRS_PER_PAGE;
                j++, old_page_table++, new_page_table++) {
            unsigned long pg;

            pg = *old_page_table;
            /* Skip all unused page table */
            if (!pg)
                continue;
            if (!(pg & PAGE_PRESENT)) {
                /* SWAP PAGE? */
                continue;
            }
            /* Mark father page table as READONLY */
            if ((pg & (PAGE_RW | PAGE_COW)) == (PAGE_RW | PAGE_COW))
                pg &= ~PAGE_RW;
            *new_page_table = pg;

            /* Kernel alway writes and read those pages. */
            if (mem_map[MAP_NR(pg)] & MAP_PAGE_RESERVED)
                continue;
            /* Prevent father task from writting the specify page. */
            *old_page_table = pg;
            /* Adding reference counter for specify page. */
            mem_map[MAP_NR(pg)]++;
        }
        *new_page_dir = new_pg_table | PAGE_TABLE;
    }
    invalidate();
    return 0;
}
late_debugcall(page_table_copy);
#endif

#ifdef CONFIG_DEBUG_PAGE_TABLE_CLONE
static int __unused page_table_clone(void)
{
    struct task_struct __unused *tsk;
    unsigned long __unused pg_dir;

    if (!(tsk = (struct task_struct *) get_free_page(GFP_KERNEL)))
        return -ENOMEM;

    pg_dir = current->tss.cr3;
    mem_map[MAP_NR(pg_dir)]++;
    tsk->tss.cr3 = pg_dir;

    return 0;
}
late_debugcall(page_table_clone);
#endif

/*
 * free_one_table - free one page table.
 *
 */
static void __unused free_one_table(unsigned long *page_dir)
{
    unsigned long pg_table = *page_dir;
    unsigned long *page_table;
    int j;

    /* Return if page table is empty. */
    if (!pg_table)
        return;

    /* Clear value on specify page directory item. */
    *page_dir = 0;

    /* Warning if page locate on high memory or page is not present on 
     * memory. */
    if (pg_table >= high_memory || !(pg_table & PAGE_PRESENT))
        printk("Bad page table: [%p]=%08lx\n", page_dir, pg_table);

    /* Doesn't release kernel page table. */
    if (mem_map[MAP_NR(pg_table)] & MAP_PAGE_RESERVED)
        return;

    /* Release all page table item. */
    page_table = (unsigned long *)(pg_table & PAGE_MASK);
    for (j = 0; j < PTRS_PER_PAGE; j++, page_table++) {
        unsigned long pg = *page_table;

        if (!pg)
            continue;
        *page_table = 0;
        if (pg & PAGE_PRESENT)
            free_page(PAGE_MASK & pg);
        else
            swap_free(pg);
    }
    /* Release this page. */
    free_page(PAGE_MASK & pg_table);
}

#ifdef CONFIG_DEBUG_PAGE_TABLE_CLEAR
/*
 * This function clears all user-level page table of a processor - this is
 * needed by execve(), so that old pages aren't in the way. Note that unlike
 * 'free_page_tables()', this function still leaves a valid page-table-tree
 * in memory: it just remove the user pages. The two functions are similar,
 * but there is a fundamental difference.
 */
static int __unused pgt_clear_entence(void)
{
    unsigned long __unused pg_dir;
    unsigned long __unused *page_dir;
    struct task_struct __unused *tsk = current;
    int i;

    if (!tsk)
        return -EINVAL;
    if (tsk == task[0]) {
        printk("task[0] (swapper) killed: unable to recover\n");
        panic("Trying to free up swapper memory space");
    }

    /*
     *                                       Page Directory
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     * CR3                                  +---------------+
     * +------------------------+ page_dir  |               |
     * | tsk->tss.cr3 (pg_dir) -|---------->+---------------+
     * +------------------------+
     */
    pg_dir = tsk->tss.cr3;
    page_dir = (unsigned long *) pg_dir;

    /* Only free user page table */
    if (!page_dir || page_dir == swapper_pg_dir) {
        printk("Trying to clear kernel page-directory: not good\n");
        return -EINVAL;
    }

    /*
     * If other proceessor reference this page-directory, we only remove user
     * page-directory and re-store kernel page-directory.
     */
    if (mem_map[MAP_NR(pg_dir)] > 1) {
        unsigned long *new_pg;

        if (!(new_pg = (unsigned long *) get_free_page(GFP_KERNEL))) {
            oom(tsk);
            return -EINVAL;
        }
        for (i = (0xC0000000 >> 22); i < 1024; i++) 
            new_pg[i] = page_dir[i];
        free_page(pg_dir);
        tsk->tss.cr3 = (unsigned long) new_pg;
        return 0;
    }

    /* Remove all user page-direntroy and hold kerenl page-directory tree
     * on memory. */
    for (i = 0; i < (0xC0000000 >> 22); i++, page_dir++)
        free_one_table(page_dir);
    invalidate();

    return 0;
}
#endif

#ifdef CONFIG_DEBUG_PAGE_TABLE_FREE
/*
 * This function frees up all page tables of a process when it exits.
 */
static int __unused pgt_free_entence(void)
{
    struct task_struct __unused *tsk = current;
    unsigned long pg_dir;
    unsigned long *page_dir;
    int i;

    if (!tsk)
        return -EINVAL;
    if (tsk == task[0]) {
        printk("task[0] (swapper) killed: unable to recover\n");
        panic("Trying to free up swapper memory space.\n");
    }
    /*
     *                                       Page Directory
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     *                                      +---------------+
     *                                      |               |
     * CR3                                  +---------------+
     * +------------------------+ page_dir  |               |
     * | tsk->tss.cr3 (pg_dir) -|---------->+---------------+
     * +------------------------+
     */

    pg_dir = tsk->tss.cr3;
    if (!pg_dir || pg_dir == (unsigned long) swapper_pg_dir) {
        printk("Trying to free kernel page-directory: not good\n");
        return -EINVAL;
    }
    tsk->tss.cr3 = (unsigned long) swapper_pg_dir;
    if (tsk == current)
        __asm__ __volatile__ ("movl %0, %%cr3" :: "a" (tsk->tss.cr3));
    if (mem_map[MAP_NR(pg_dir)] > 1) {
        free_page(pg_dir);
    }
    page_dir = (unsigned long *) pg_dir;
    for (i = 0; i < PTRS_PER_PAGE; i++, page_dir++)
        free_one_table(page_dir);
    free_page(pg_dir);
    invalidate();

    return 0;
}
#endif

#if defined CONFIG_DEBUG_PAGE_TABLE_CLEAR | \
    defined CONFIG_DEBUG_PAGE_TABLE_FREE 
asmlinkage int sys_demo_pgt_entence(void)
{
#ifdef CONFIG_DEBUG_PAGE_TABLE_CLEAR
    pgt_clear_entence();
#endif

#ifdef CONFIG_DEBUG_PAGE_TABLE_FREE
    pgt_free_entence();
#endif
    return 0;
}
/* System call entry */
inline _syscall0(int, demo_pgt_entence);
#endif

#ifdef CONFIG_DEBUG_PAGE_TABLE_CLEAR
static inline _syscall0(int,fork);

static int __unused page_table_clear(void)
{
    int pid;

    if (!(pid = fork())) {
        /* Child process */
        printf("Child PID: %#x waiting and clear user page table....\n", pid);
        demo_pgt_entence();
        while (1);
    }
    if (pid > 0) {
        /* Father wait */
        printf("Father PID: %#x always running....\n", pid);
    }
    return 0;
}
user1_debugcall_sync(page_table_clear);
#endif

#ifdef CONFIG_DEBUG_PAGE_TABLE_FREE
static inline _syscall0(int,fork);

static int __unused page_table_free(void)
{
    int pid;

    if (!(pid = fork())) {
        /* Child process */
        printf("Child PID: %#x waiting and free all page table....\n", pid);
        demo_pgt_entence();
        while (1);
    }
    if (pid > 0) {
        /* Father wait */
        printf("Father PID: %#x always running....\n", pid);
    }
    return 0;
}
user1_debugcall_sync(page_table_free);
#endif
