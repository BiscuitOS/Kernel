/*
 * Register and Control bit on 32-Bit Paging Mode.
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <test/debug.h>

static char var[] = "BiscuitOS";
/*
 * Control Register 3 (CR3)
 *   Used when virtual addressing is enabled, hence when the PG bit is set
 *   in CR0. CR3 enables the processor to translate linear addresses into
 *   physical addresses by locating the page directory and page tables for
 *   the current task. Typically, the upper 20 bits of CR3 become the page
 *   directory base register (PDBR), which stores the physical address of
 *   the first page directory entry. If the PCIDE bit in CR4 is set, the 
 *   lowest 12 bits are used for the process-context identifier (PCID). 
 *
 *   31------------------------12---------5---4-----3------------0
 *   | Address of page directory | Ignored | PCD | PWT | Ignored |
 *   -------------------------------------------------------------
 */
static void parse_CR3(void)
{
    unsigned long cr3;

    /* Obtain CR3 contents */
    __asm__ ("movl %%cr3, %0" : "=r" (cr3));

    /* CR3[31:12]
     *   Physical address of the 4Kbyte aligned page directory used
     *   for linear-address translation.
     */
    printk("1st Page Directory: %#x\n", cr3 & 0xFFFFF000);
    /*
     * CR3[4]: PCD
     *   Page-level cache disabled. Indirectly determines the memory
     *   type used to access the page directory during linear-address
     *   translation.
     */
    printk("Page-level cache: %s\n", 
           (cr3 >> 4) & 0x1 ? "Disable" : "Enable");
    /*
     * CR3[3]: PWT
     *   Page-level write-through. Indirectly determines the memory
     *   type used to access the page directory during linear-address
     *   translation.
     */
     printk("Page-level write-through: %s\n",
            (cr3 >> 3) & 0x1 ? "Enable" : "Disable");
}

/*
 * Format of a 32-Bit Page-Directory Entry.
 *
 *   31-----------------------------------------------12
 *   |              Address of page table              |
 *   ---------------------------------------------------
 *
 *   11-------8--7----6---5----4-----3-----2-----1----0--
 *   | Ignored | PS | X | A | PCD | PWT | U/S | R/W | P |
 *   ----------------------------------------------------
 */
static void parse_PDE(void)
{
    unsigned long cr3, ds, cr4, *linear, *base;
    unsigned long *PDE;
    struct logic_addr la;
    struct desc_struct *desc;
 
    /* Establish logic address */
    __asm__ ("movl %%ds, %0" : "=r" (ds));
    la.offset = (unsigned long)&var;
    la.sel = ds;
    /* Obtain linear address */
    if ((la.sel >> 2) & 0x1)
        desc = &current->ldt[la.sel >> 3];
    else
        desc = &gdt[la.sel >> 3];
    linear = (unsigned long *)(get_base(*desc) + la.offset);
    /* Obtain PDE */
    __asm__ ("movl %%cr3, %0" : "=r" (cr3));
    base = (unsigned long *)(cr3 & 0xFFFFF000);
    PDE = &base[(unsigned long)linear >> 22];

    /*
     * PDE[31:12]
     *   Physical address of 4-KByte aligned page table referenced
     *   by this entry.
     */
    printk("PTE Address: %#x\n", (unsigned long)(*PDE & 0xFFFFF000));
    /*
     * PDE[7] PS
     *   If CR4.PSE = 1, must be 0 (Otherwise, this entry maps a
     *   4-MByte page), otherwise, ignored.
     */
    __asm__ ("movl %%cr4, %0" : "=r" (cr4));
    if ((cr4 >> 4) & 0x1) /* Check PSE on CR4 */
        if (((unsigned long)*PDE >> 7) & 0x1)
            panic("If CR4.PSE = 1, PS bit must be 0");
    /*
     * PDE[5] A
     *   Accessed. Indicates whether this entry has been used for
     *   linear-address translation.
     */
    if (((unsigned long)*PDE >> 5) & 0x1)
        printk("PDE has been used for linear-address translation.\n");
    else
        printk("PDE hasn't been used for linear-address translation.\n");
    /*
     * PDE[4] PCD
     *   Page-level cache disable. Indirectly determines the memory type
     *   used to access the page table referenced by this entry.
     */
    printk("Page-level cache: %s\n", ((unsigned long)*PDE >> 4) & 0x1 ?
               "disable" : "enable");
    /*
     * PDE[3] PWT
     *   Page-level write-through. Indirectly determines the memory type
     *   used to access the page table referenced by this entry.
     */
    printk("Page-level write-though %s\n", ((unsigned long)*PDE >> 3) & 0x1 ?
                "enable" : "disable");
    /*
     * PDE[2] U/S
     *   User/Supervisor. If 0, user-mode accesses are not allowed to the 
     *   4-MByte region controlled by this entry.
     */
    if (!(((unsigned long)*PDE >> 2) & 0x1))
        printk("Prevent user-mode access 4-MByte page.\n");
    /*
     * PDE[1] R/W
     *   Read/Write. If 0, write may not be allowed to the 4-MByte region
     *   controlled by this entry.
     */
    if (!(((unsigned long)*PDE >> 1) & 0x1))
        printk("Prevent write to 4-MByte page.\n");
    /*
     * PDE[0] P
     *   Present. Must be 1 to reference a page table.
     */
    if ((unsigned long)*PDE & 0x1)
        printk("PDE resident on Memory.\n");
    else
        printk("PDE doesn't exist on Memory.\n");
}

/*
 * Format of a 32-Bit Page-Table entry.
 * 
 *   31-------------------------------------------------------
 *   |           Address of 4KB page frame                   |
 *   ---------------------------------------------------------
 *
 *   11-------9--8----7----6---5----4-----3-----2-----1----0--
 *   | Ignored | G | PAT | D | A | PCD | PWT | U/S | R/W | P |
 *   --------------------------------------------------------- 
 */
static void parse_PTE(void)
{
    unsigned long cr3, cr4, ds, *linear, *PDE, *PTE, *base;
    struct desc_struct *desc;
    struct logic_addr la;

    /* Establish logical address */
    __asm__ ("movl %%ds, %0" : "=r" (ds));
    la.offset = (unsigned long)&var;
    la.sel    = ds;
    /* Obtain Linear address */
    if ((la.sel >> 2) & 0x1)
        desc = &current->ldt[la.sel >> 3];
    else
        desc = &gdt[la.sel >> 3];
    linear = (unsigned long *)(get_base(*desc) + la.offset);
    /* Obtain PDE */
    __asm__ ("movl %%cr3, %0" : "=r" (cr3));
    base = (unsigned long *)(cr3 & 0xFFFFF000);
    PDE = &base[(unsigned long)linear >> 22];
    /* Obtain PTE */
    base = (unsigned long *)((unsigned long)*PDE & 0xFFFFF000);
    PTE = &base[((unsigned long)linear >> 12) & 0x3FF];
    
    /* PTE[31:12]
     *   Physical address of the 4-KByte page referenced by this
     *   entry. 
     */
    printk("4-KByte Page base address: %#x\n", 
                              (unsigned long)*PTE & 0xFFFFF000);
    /*
     * PTE[8] G
     *   Global. If CR4.PGE = 1. determines whether the translation
     *   is global.
     */
    __asm__ ("movl %%cr4, %0" : "=r" (cr4));
    if ((cr4 >> 7) & 0x1) { /* bit7 of CR4 is PGE */
        if (((unsigned long)*PTE >> 8) & 0x1)
            printk("Page Translation is global.\n");
        else
            printk("Page Translation isn't global.\n");
    }
    /*
     * PTE[7] PAT
     *   If the PAT is supported. Indirectly determines the memory type
     *   used to access the 4-KByte page referenced by this entry. 
     *   otherwise, reserved (must be 0)
     */
    if (((unsigned long)*PTE >> 7) & 0x1) 
        printk("Support PAT and access the 4-KByte page reference\n");
    /*
     * PTE[6] D
     *   Dirty. Indicates whether software has written to the 4-KByte
     *   page referenced by this entry.
     */
    if (((unsigned long)*PTE >> 6) & 0x1)
        printk("4-KByte is Dirty.\n");
    else
        printk("Software has written to the 4-KByte.\n");
    /*
     * PTE[5] A
     *   Accessed. Idicates whether software has accessed the 4-KByte page
     *   referenced by this entry.
     */
    if (((unsigned long)*PTE >> 5) & 0x1)
        printk("4-KByte Page has referenced!\n");
    else
        printk("4-KByte Page doesn't referenced!\n");
    /*
     * PTE[4] PCD
     *   Page-Level cache disable. Idirectly determines the memory type used
     *   to access the 4-KByte page referenced by this entry.
     */
    if (((unsigned long)*PTE >> 4) & 0x1)
        printk("Page-Level cache disable\n");
    else
        printk("Page-Level cache enable\n");
    /*
     * PTE[3] PWT
     *   Page-Level write-through. Indirectly determines the memory type
     *   used to access the 4-KByte page referenced by this entry.
     */
    if (((unsigned long)*PTE >> 3) & 0x1)
        printk("Page-Level write-through enable\n");
    else
        printk("Page-Level write-through disable\n");
    /*
     * PTE[2] U/S
     *   User/Supervisor. If 0, used-mode accesses are not allowed to
     *   4-KByte page referenced by this entry.
     */
    if (((unsigned long)*PTE >> 2) & 0x1)
        printk("User-mode allowes to access 4-KByte page referenced.\n");
    else
        printk("User-mode doesn't allows to access 4-KByte page reference.\n");
    /*
     * PTE[1] R/W
     *   Read/Write. If 0, writes may not be allowed to the 4-KByte page
     *   referenced by this entry.
     */
    if (((unsigned long)*PTE >> 1) & 0x1)
        printk("Allow to write to 4-KByte page\n");
    else
        printk("Forbid to write to 4-KByte page\n");
    /*
     * PTE[0] P
     *   Present, must be 1 to map a 4-KByte Page.
     */
    if (((unsigned long)*PTE >> 0) & 0x1)
        printk("4-KByte page has map to physic\n");
    else
        printk("4-KByte page doesn't map to physic\n");
}

int debug_paging_register_common(void)
{
    if (1) {
        parse_PTE();
    } else {
        parse_CR3();
        parse_PDE();
        parse_PTE();
    }
    return 0;
}
