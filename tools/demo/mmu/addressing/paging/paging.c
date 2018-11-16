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

#ifdef CONFIG_DEBUG_32BIT_PAGING
static int __unused paging_32bit(void)
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
    unsigned long __unused virtual;
    unsigned long __unused logical;
    unsigned long __unused linear;
    unsigned long __unused *physical;
    unsigned short __unused SS;
    struct desc_struct __unused *desc;
    char __unused *hello = "Hello BiscuitOS";

    /* Obtain virual address of 'hello' */
    virtual = (unsigned int)(unsigned long)hello;

    /* Obtain logical address of 'hello' */
    __asm__ ("mov %%ss, %0" : "=m" (SS));
    /* Logical address means: logical = SS : virtual */

    /* Obtain linear address of 'hello' */
    if ((SS >> 2) & 0x1)
        desc = current->ldt + (SS >> 3);
    else
        desc = gdt + (SS >> 3);
    linear = get_base(*desc) + virtual;

    /* Obtain pgdir virtual address, linux 1.0 define 'swapper_pg_dir' to
     * point first page directory */
    pgdir = swapper_pg_dir;

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

    return 0;
}
late_debugcall(paging_32bit);
#endif

#ifdef CONFIG_DEBUG_PAGING_ESTABLISH
static int __unused paging_32bit_init(void)
{
    unsigned long __unused start_mem = 0x100000;
    unsigned long __unused end_mem   = 0x1000000;
    unsigned long __unused *pg_dir;
    unsigned long __unused *pg_table;
    unsigned long __unused tmp;
    unsigned long __unused address;

    __asm__ ("mov $1024*2, %%ecx\n\r"
             "xorl %%eax, %%eax\n\r"
             "movl $swapper_pg_dir, %%edi\n\r"
             "cld\n\r"
             "rep stosl\n\r"
             "movl $pg0+7, swapper_pg_dir\n\r"
             "movl $pg0+7, swapper_pg_dir+3072\n\r"
             "movl $pg0+4092, %edi\n\r"
             "movl $0x03ff007, %%eax\n\r"
             "std\n\r"
             "1: stosl\n\r"
             "subl $0x1000, %%eax\n\r"
             "jge 1b\n\r"
             "cld\n\r"
             "movl $swapper_pg_dir, %%eax\n\r"
             "movl %%eax, %%cr3\n\r"
             "movl %%cr0, %%eax\n\r"
             "orl $0x80000000, %%eax\n\r"
             "movl %%eax, %%cr0");

    start_mem = PAGE_ALIGN(start_mem);
    address = 0;
    pg_dir = swapper_pg_dir;
    while (address < end_mem) {
        tmp = *(pg_dir + (0xC000000 >> 22)); /* at linear addr 0xC000000 */
        if (!tmp) {
            tmp = start_mem | PAGE_TABLE;
            *(pg_dir + (0xC0000000 >> 22)) = tmp;
            start_mem += PAGE_SIZE;
        }
        *pg_dir = tmp; /* also map it in at linear addr 0x00000000 */
        pg_dir++;
        pg_table = (unsigned long *)(tmp & PAGE_MASK);
        for (tmp = 0; tmp < PTRS_PER_PAGE; tmp++, pg_table++) {
            if (address < end_mem)
                *pg_table = address | PAGE_SHARED;
            else
                *pg_table = 0;
            address += PAGE_SIZE;
        }
    }

    return 0;
}
late_debugcall(paging_32bit_init);
#endif
