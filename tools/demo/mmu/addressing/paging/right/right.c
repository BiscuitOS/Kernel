/*
 * Paging mechanism
 *
 * (C) 2018.11.22 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

#include <demo/debug.h>

#ifdef CONFIG_DEBUG_MMU_RT_ESTABLISH
/*
 * Establish a user-mode address space for kernel.
 *
 *   The linear address range of user-mode from 0xC6400000 to 0xC6401000.
 *   And virutal address range from 0x6400000 to 0x6401000.
 */
static int __unused establish_user_mode_space(void)
{
    unsigned long __unused *virtual;
    unsigned long __unused *linear;
    unsigned long __unused *pde;
    unsigned long __unused *pte;
    unsigned long __unused tmp;
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;

    /* Obtain virtual address from KBuild */
    virtual = (unsigned long *)CONFIG_DEBUG_RT_EST_VA;

    /* Obtain linear address */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));
    if ((Sel & 0x3) > 2) 
        desc = current->ldt + (Sel >> 3);
    else
        desc = gdt + (Sel >> 3);
    linear = (unsigned long *)((unsigned long)virtual + get_base(*desc));

    /* Estabish page-director */
    pde = PAGE_DIR_OFFSET(current->tss.cr3, linear);
    if (*pde)
        panic("linear address has page-directory.");
    /* Establish a user-mode page-directory */
    if (!(tmp = get_free_page(GFP_KERNEL)))
        panic("No free memory for allocating pgae-directory");

    /* Mark page as page-directory */
    tmp |= PAGE_TABLE;
    /* Mark page as user-mode page-directory */
#ifdef CONFIG_DEBUG_RT_EST_USER
    tmp &= ~(PAGE_USER);
#endif
    *pde = tmp;

    /* Check page table */
    tmp &= PAGE_MASK;
    tmp += PAGE_PTR(linear);
    pte = (unsigned long *)tmp;

    /* Establish a user-mode page-table */
    if (!(tmp = get_free_page(GFP_KERNEL)))
        panic("No free memory for allocating page-table");

    /* Mark page as page-table */
    tmp |= PAGE_TABLE;
    /* Mark page as user-mode page-table */
#ifdef CONFIG_DEBUG_RT_EST_USER
    tmp &= ~(PAGE_USER);
#endif
    *pte = tmp;

    /* Setup default vaule on 0x6400000 */
    *virtual = CONFIG_DEBUG_RT_EST_VL;
    /* Force mark page-table as user-mode */
#ifdef CONFIG_DEBUG_RT_EST_USER
    tmp &= ~(PAGE_USER);
#else
    tmp |= PAGE_USER;
#endif
    *pte = tmp;


    return 0;
}
#endif
#ifdef CONFIG_DEBUG_MMU_RT_SUP
late_debugcall(establish_user_mode_space);
#endif

#ifdef CONFIG_DEBUG_RT_SUP_SUPDATA
static unsigned long __unused SupAddr = 0x99998888;
/*
 * Supervisor-mode read (implicitly or explicitly) any supervisor-mode address.
 */
static int __unused SUP_supervisor_data(void)
{   
    unsigned long __unused *pde;
    unsigned long __unused *pte;
    unsigned long __unused tmp;
    unsigned long __unused *address;
    unsigned long __unused *linear;
    unsigned long __unused Sel;
    struct desc_struct __unused *desc;

    /* Verify whether is supervisor-mode access. */
    __asm__ ("mov %%cs, %0" : "=m" (Sel));
    if ((Sel & 0x3) >= 3)
        panic("Only need supervisor-mode!");

    /* The virutal address points to kernel data segment. */ 
    address = &SupAddr;
    /* Cover virtual address to linear address on kernel */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));
    if ((Sel >> 2) & 0x1) 
        desc = current->ldt + (Sel >> 3);
    else
        desc = gdt + (Sel >> 3);
    linear = (unsigned long *)((unsigned long)address + get_base(*desc));
    
    /* Obtain page-direntory */
    pde = PAGE_DIR_OFFSET(current->tss.cr3, linear);
    /* The content of page directory item. */
    tmp = *pde;
    if ((tmp & PAGE_TABLE) == PAGE_TABLE)
        printk("Supervisor-mode permits access page directory.\n");
    else 
        printk("Supervisor-mode doesn't permit access page directory.\n");
    
    /* Obtain page table */
    tmp &= PAGE_MASK;
    tmp += PAGE_PTR(linear);
    pte = (unsigned long *)tmp;
    /* The content of page table item. */
    tmp = *pte;
    if ((tmp & PAGE_TABLE) == PAGE_TABLE)
        printk("Supervisor-mode permits access page table.\n");
    else 
        printk("Supervisor-mode doesn't permit access page table.\n");

    /* Access supervisor-mode address in Data */
    printk("Supervisor-mode data: %#lx --> %#lx\n", 
              (long unsigned int)address, (long unsigned int)address);
    return 0;
}
#endif

#ifdef CONFIG_DEBUG_RT_SUP_USERDATA
/*
 * Supervisor-mode read Data from user-mode pages.
 *
 *   Access rights depend on the value of CR4.SMAP:
 *
 *   * If CR4.SMAP = 0, data may be read from any user-mode address with a
 *     protection key for which read access is permitted.
 *
 *   * If CR4.SMAP = 1, access rights depend on the value of EFLAGS.AC and
 *     whether the access is implicit or explicit:
 *
 *     -- If EFLAGS.AC = 1 and the access is explicit, data may be read from
 *        any user-mode address with a protetion key for which read access is
 *        permitted.
 *
 *     -- If EFLAGS.AC = 0 or access is implicit, data may not be read from any
 *        user-mode address.
 *
 */
static int __unused SUP_user_data(void)
{
    unsigned long __unused *pde;
    unsigned long __unused *pte;
    unsigned long __unused *address;
    unsigned long __unused *linear;
    unsigned long __unused tmp;
    unsigned long __unused CR4;
    unsigned short __unused Sel;
    unsigned long __unused EFLAGS;
    struct desc_struct __unused *desc;

    /* Verify supervisor- or user- mode? */
    __asm__ ("mov %%cs, %0" : "=m" (Sel));
    /* Here, only use supervisor-mode */
    if ((Sel & 0x3) >= 3)
        panic("Must be supervisor-mode");

    /* User-mode address: setup */
    address = (unsigned long *)CONFIG_DEBUG_RT_EST_VA;

    /* Obtain linear address */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));
    if ((Sel >> 2) & 0x1)
        desc = current->ldt + (Sel >> 3);
    else
        desc = gdt + (Sel >> 3);
    linear = (unsigned long *)((unsigned long)address + get_base(*desc));

    /* Obtain CR4.SMAP control bit */
    __asm__ ("movl %%cr4, %0" : "=r" (CR4));

#ifdef CONFIG_DEBUG_RT_SUP_RU_SMAP0
    CR4 &= ~(1 << 21);
#elif defined CONFIG_DEBUG_RT_SUP_RU_SMAP1
    CR4 |=  (1 << 21);
    __asm__ ("pushf ; popl %%eax" : "=a" (EFLAGS));
#ifdef CONFIG_DEBUG_RT_SUP_RU_EFLAGSAC0
    EFLAGS &= ~(1 << 18);
#elif defined CONFIG_DEBUG_RT_SUP_RU_EFLAGSAC1
    EFLAGS |= (1 << 18);
#endif
    __asm__ ("pushl %%eax ; popfl" :: "a" (EFLAGS));
#endif
    __asm__ ("movl %0, %%cr4" :: "r" (CR4));

    /* Verify whether address is user-mode address on page-directory */
    pde = PAGE_DIR_OFFSET(current->tss.cr3, linear);
    /* The content of page directory */
    tmp = *pde;
    if (!tmp || ((tmp & (PAGE_PRESENT | PAGE_USER)) == 
                        (PAGE_PRESENT | PAGE_USER)))
        panic("Maybe user-mode address on page-directory doesn't exist!");

    /* Verify whether address is user-mode address on page-table */
    tmp &= PAGE_MASK;
    tmp += PAGE_PTR(linear);
    pte = (unsigned long *)tmp;
    tmp = *pte;
    if (!tmp || ((tmp & (PAGE_PRESENT | PAGE_USER)) ==
                        (PAGE_PRESENT | PAGE_USER)))
        panic("Maybe user-mode address on page-table doesn't exist!");
     
    /* Reference it! */
    printk("User-mode address: %#x --> %#x\n", 
                         (unsigned int)address, (unsigned int)*address);

    return 0;
}
#endif

#ifdef CONFIG_DEBUG_RT_WR_SUP
/*
 * Supervisor-mode writes data to supervisor-mode address.
 *
 *
 *   Access right depend on the value of CR0.WP:
 *
 *  * If CR0.WP = 0, data may be written to any supervisor-mode address.
 *
 *  * If CR0.WP = 1. data may be written to any supervisor-mode address with
 *    a translation for which the R/W flag (bit 1) is 1 in every paging-
 *    structure entry controlling the translation; data may not be written to
 *    any supervisor-mode address with a translation for which the R/W flag is
 *    0 in any paging-structure entry controlling the translation.
 *
 */
static unsigned long __unused Reserved_data = 0x89898989;

static int __unused SUP_wr_supervisor_data(void)
{
    unsigned long __unused *address;
    unsigned long __unused *linear;
    unsigned long __unused *pde;
    unsigned long __unused *pte;
    unsigned long __unused tmp;
    unsigned long __unused CR0;
    unsigned long __unused Sel;
    struct desc_struct __unused *desc;

    /* Verify whether is supervisor-mode */
    __asm__ ("mov %%cs, %0" : "=m" (Sel));
    if ((Sel & 0x3) > 2)
        panic("Must be supervisor-mode!");

    /* Obtain data on supervisor-mode address */
    address = &Reserved_data;

    /* Obtain linear address */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));
    if ((Sel >> 2) & 0x1)
        desc = current->ldt + (Sel >> 3);
    else 
        desc = gdt + (Sel >> 3);
    linear = (unsigned long *)((unsigned long)address + get_base(*desc));

    /* Obtain CR0.WP control bit. */
    __asm__ ("movl %%cr0, %0" : "=r" (CR0));

#ifdef CONFIG_DEBUG_RT_WR_SUP_WP0
    CR0 &= ~(1 << 16);
#elif defined CONFIG_DEBUG_RT_WR_SUP_WP1
    CR0 |= (1 << 16);
#endif
    __asm__ ("movl %0, %%cr0" :: "r" (CR0));

    /* If CR0.WP = 0, data may be written to any supervisor-mode address. */
    if ((CR0 >> 16) & 0x1) {
        /*
         * If CR0.WP = 1. data may be written to any supervisor-mode address
         * with a translation for which the R/W flag (bit 1) is 1 in every
         * paging- structure entry controlling the translation; data may not
         * be written to any supervisor-mode address with a translation for
         * which the R/W flag is 0 in any paging-structure entry controlling
         * the translation.
         */
        pde = PAGE_DIR_OFFSET(current->tss.cr3, linear);
#ifdef CONFIG_DEBUG_RT_WR_SUP_PGD_WR
        *pde |= PAGE_RW; 
#elif defined CONFIG_DEBUG_RT_WR_SUP_PGD_NW
        *pde &= ~PAGE_RW;
#endif
        tmp = *pde;
        if (!tmp || ((tmp & (PAGE_PRESENT || PAGE_USER)) != 
                                     (PAGE_PRESENT || PAGE_USER)))
            panic("Page-table doesn't exist!");
        if ((tmp & PAGE_RW) != PAGE_RW)
            printk("Page-table doesn't permitt to write!\n");

        /* Obtain Page-Table entence */
        tmp = (tmp & PAGE_MASK) + PAGE_PTR(linear);
        pte = (unsigned long *)tmp;
#ifdef CONFIG_DEBUG_RT_WR_SUP_PTE_WR
        *pte |= PAGE_RW;        
#elif defined CONFIG_DEBUG_RT_WR_SUP_PTE_NW
        *pte &= ~PAGE_RW;
#endif
        tmp = *pte;
        if (!tmp || ((tmp & (PAGE_PRESENT || PAGE_USER)) != 
                                     (PAGE_PRESENT || PAGE_USER)))
            panic("Page doesn't exist!\n");
        if ((tmp & PAGE_RW) != PAGE_RW)
            printk("Page doesn't permiitt to write!\n");
    }

    /* Write data into supervisor-mode address */
    printk("Original data: %#lx -> %#lx\n", (unsigned long)address, *address);
    *address = 0xBB66BB66;
    printk("Re-write data: %#lx -> %#lx\n", (unsigned long)address, *address);

    return 0;
}
#endif

#ifdef CONFIG_DEBUG_RT_WR_USER
/*
 * Supervisor-mode write data into user-mode address
 *
 * Access right depend on the value of CR0.WP:
 *
 * * If CR0.WP = 0. access rights depend on the value of CR4.SMAP:
 *
 *   -- If CR4.SMAP = 0, data may be written to any user-mode address with a
 *      protection key for which write access is permitted.
 *
 *   -- If CR4.SMAP = 1, access rights depend on the value of EFLAGS.AC and
 *      whether the access is implicit or explicit:
 *
 *      * If EFLAGS.AC = 1 and the access is explicit, data may be written to
 *        any user-mode address with a protection key for which write access
 *        is permitted.
 *
 *      * If EFLAGS.AC = 0 or the access is implicit, data may not be written
 *        to any user-mode address.
 *
 * * If CR0.WP = 1, access rights depend on the value of CR4.SMAP:
 *
 *   -- If CR4.SMAP = 0, data may be written to any user-mode address with a
 *      translation for which the R/W flag is 1 in every paging-structure 
 *      entry controlling the translation and with a protection key for which
 *      write access is permitted; data may not be written to any user-mode
 *      address with a translation for which the R/W flag is 0 in any paging-
 *      structure entry controlling the translation.
 *
 *   -- If CR4.SMAP = 1, access rights depend on the value of EFLAGS.AC and
 *      whether the access is implicit or explict:
 *
 *      * If EFLAGS.AC = 1 and the access is explicit, data may be written to
 *        any user-mode address with a translation for which the R/W flag is 1
 *        in every paging-structure entry controlling the translation and with
 *        a protection key for which write access is permitted; data may not
 *        be written to any user-mode address with a translation for which the
 *        R/W flag is 0 in any paging-structure entry controlling the
 *        translation.
 *
 *      * If EFLAGS.AC = 0 or the access is implicit, data may not be written
 *        to any user-mode address.
 */
static int __unused SUP_wr_user_data(void)
{
    unsigned long __unused *address;
    unsigned long __unused *linear;
    unsigned long __unused *pde;
    unsigned long __unused *pte;
    unsigned long __unused tmp;
    unsigned long __unused CR4;
    unsigned long __unused CR0;
    unsigned long __unused EFLAGS;
    unsigned short __unused Sel;
    struct desc_struct __unused *desc;

    /* Verify whether is explicit supervisor-mode */
    __asm__ ("mov %%cs, %0" : "=m" (Sel));
    if ((Sel & 0x3) > 2)
        panic("Must be supervisor-mode");

    /* Obtain user-mode address from KBuild */
    address = (unsigned long *)CONFIG_DEBUG_RT_EST_VA;

    /* Obtain the linear address */
    __asm__ ("mov %%ds, %0" : "=m" (Sel));
    if ((Sel >> 2) & 0x1) 
        desc = current->ldt + (Sel >> 3);
    else
        desc = gdt + (Sel >> 3);
    linear = (unsigned long *)((unsigned long)address + get_base(*desc));

    /* Obtain CR0, CR4, and EFLAGS */
    __asm__ ("mov %%cr0, %0\n\r"
             "mov %%cr4, %1\n\r"
             "pushf ; popl %%eax" : "=r" (CR0), "=r" (CR4), "=a" (EFLAGS));

#ifdef CONFIG_DEBUG_RT_WR_USER_CR0WP0
    CR0 &= ~(1 << 16);
#elif defined CONFIG_DEBUG_RT_WR_USER_CR0WP1
    CR0 |=  (1 << 16);
#endif

#ifdef CONFIG_DEBUG_RT_WR_USER_CR4SMAP0
    CR4 &= ~(1 << 21);
#elif defined CONFIG_DEBUG_RT_WR_USER_CR4SMAP1
    CR4 |=  (1 << 21);

#ifdef CONFIG_DEBUG_RT_WR_USER_EFLAGSAC0
    EFLAGS &= ~(1 << 18);
#elif defined CONFIG_DEBUG_RT_WR_USER_EFLAGSAC1
    EFLAGS |=  (1 << 18);
#endif

#endif
    /* Reload CR0, CR4, and EFLAGS */
    __asm__ ("mov %0, %%cr0\n\r"
             "mov %1, %%cr4\n\r"
             "pushl %%eax ; popfl" :: "r" (CR0), "r" (CR4), "a" (EFLAGS));

    /* Translation on Page-directory */
    pde = PAGE_DIR_OFFSET(current->tss.cr3, linear);
    tmp = *pde;

#ifdef CONFIG_DEBUG_RT_WR_USER_PGD_RDONLY
    tmp &= ~PAGE_RW;
#elif defined CONFIG_DEBUG_RT_WR_USER_PGD_RDONLY
    tmp |= PAGE_RW;
#endif
    *pde = tmp;

    /* Verify wheterh page is user-mode address */
    if (!tmp || ((tmp & (PAGE_PRESENT | PAGE_USER)) == 
                        (PAGE_PRESENT | PAGE_USER)))
        panic("Must be user-mode address on page-directory");

    /* Translation on Page-table */
    tmp &= PAGE_MASK;
    tmp += PAGE_PTR(linear);
    pte = (unsigned long *)tmp;
    tmp = *pte;
    
#ifdef CONFIG_DEBUG_RT_WR_USER_PGT_RDONLY
    tmp &= ~PAGE_RW;
#elif defined CONFIG_DEBUG_RT_WR_USER_PGT_WR
    tmp |= PAGE_RW;
#endif
    *pte = tmp;

    /* Verify whether page is user-mode address */
    if (!tmp || ((tmp & (PAGE_PRESENT | PAGE_USER)) ==
                        (PAGE_PRESENT | PAGE_USER)))
        panic("Must be user-mode address on page-table");

    printk("Original-data: %#lx\n", *address);
    /* If CR0.WP = 0. access rights depend on the value of CR4.SMAP */
    if (!((CR0 >> 16) & 0x1)) {
        if (!((CR4 >> 21) & 0x1)) {
            /*
             * If CR4.SMAP = 0, data may be written to any user-mode address 
             * with a protection key for which write access is permitted.
             */
            printk("CR0.WP(0) CR4.SMAP(0): write any user-mode address\n");
            *address = 0x23333333;
        } else {
            /*
             * If CR4.SMAP = 1, access rights depend on the value of EFLAGS.AC
             * and whether the access is implicit or explicit:
             */
            if ((EFLAGS >> 18) & 0x1) {
                /*
                 * If EFLAGS.AC = 1 and the access is explicit, data may be
                 * written to any user-mode address with a protection key for
                 * which write access is permitted.
                 */
                printk("CR0.WP(0) CR4.SMAP(1) EFLAGS.AC(1): explicity access"
                       " user-mode address\n");
                *address = 0x23333333;
            } else {
                /*
                 * If EFLAGS.AC = 0 or the access is implicit, data may not be
                 * written to any user-mode address.
                 */
                printk("CR0.WP(0) CR4.SMAP(1) EFLAGS.AC(0): explicit access"
                       " any user-mode address");
                *address = 0x23333333;
            }
        }
    } else {
        /* If CR0.WP = 1, access rights depend on the value of CR4.SMAP */
        if (!((CR4 >> 21) & 0x1)) {
            /*
             * If CR4.SMAP = 0, data may be written to any user-mode address
             * with a translation for which the R/W flag is 1 in every 
             * paging-structure entry controlling the translation and with a
             * protection key for which write access is permitted; data may not
             * be written to any user-mode address with a translation for which
             * the R/W flag is 0 in any paging-structure entry controlling the
             * translation.
             */
            if (((*pde & PAGE_RW) == PAGE_RW) && 
                           (((*pte & PAGE_RW) == PAGE_RW))) {
                printk("CR0.WP(1) CR4.SMAP(0) Paging-structure R/W (1)\n");
                *address = 0x23333333;
            } else if (((*pde & PAGE_RW) != PAGE_RW) && 
                           (((*pte & PAGE_RW) != PAGE_RW))) {
                printk("CR0.WP(1) CR4.SMAP(0) Paging-structure R/W (0)\n");
                *address = 0x23333333;
            } else {
                printk("CR0.WP(1) CR4.SMAP(0) Paging-structure R/W (0/1)\n");
                *address = 0x23333333;
            }
        } else {
            /* 
             * If CR4.SMAP = 1, access rights depend on the value of EFLAGS.AC
             * and whether the access is implicit or explict
             */
            if ((EFLAGS >> 18) & 0x1) {
                /*
                 * If EFLAGS.AC = 1 and the access is explicit, data may be 
                 * written to any user-mode address with a translation for
                 * which the R/W flag is 1 in every paging-structure entry
                 * controlling the translation and with a protection key for
                 * which write access is permitted; data may not be written to
                 * any user-mode address with a translation for which the R/W
                 * flag is 0 in any paging-structure entry controlling the
                 * translation.
                 */
                if (((*pde & PAGE_RW) == PAGE_RW) &&
                               (((*pte & PAGE_RW) == PAGE_RW))) {
                    printk("CR0.WP(1) CR4.SMAP(1) EFLAGS.AC(1) "
                                          "Paging-structure R/W (1)\n");
                    *address = 0x23333333;
                } else if (((*pde & PAGE_RW) != PAGE_RW) &&
                               (((*pte & PAGE_RW) != PAGE_RW))) {
                    printk("CR0.WP(1) CR4.SMAP(1) EFLAGS.AC(1) "
                                          "Paging-structure R/W (0)\n");
                    *address = 0x23333333;
                } else {
                    printk("CR0.WP(1) CR4.SMAP(1) EFLAGS.AC(1) "
                                          "Paging-structure R/W (0/1)\n");
                    *address = 0x23333333;
                }
            } else {
                /*
                 * If EFLAGS.AC = 0 or the access is implicit, data may not be
                 * written to any user-mode address.
                 */
                printk("CR0.WP(1) CR4.SMAP(1) EFLAGS.AC(0)\n");
                *address = 0x23333333;
            }
        }
    }

    printk("Modify-data: %#lx\n", *address);

    return 0;
}
#endif

#ifdef CONFIG_DEBUG_MMU_RT_SUP
/*
 * For supervisor-mode accesses:
 *
 *   Every access to a linear address is either a supervisor-mode access or a
 *   user-mode access. For all instruction fetures and most data accesses, the
 *   distinction is determined by the current privilege level (CPL): accesses 
 *   made while CPL < 3 are supervisor-mode accesses.
 *
 *   Some operations implicitly access system data structures with linear 
 *   address; the resulting accesses to those data structure are supervisor-
 *   mode accesses regardless of CPL. Examples of such accesses include the 
 *   following:
 *
 *   * Accesses to the global descriptor table (GDT) or local descriptor 
 *     table (LDT) to load a segment descriptor.
 *
 *   * Accesses to the interrupt descriptor table (IDT) when delivering an
 *     interrupt or exception.
 *
 *   * Accesses to the task-state segment (TSS) as part of a task switch or
 *     change of CPL.
 *
 *   All these accesses are called implicit supervisor-mode accesses regardless
 *   of CPL. Other accesses made while CPL < 3 are called explicit supervisor-
 *   mode accesses.
 */
static int __unused paging_right_supervisor(void)
{
    /*
     * Access rights are also controlled by the mode of a linear address as
     * specified by the paging-structure entries controlling the translation
     * of the linear address. If the U/S flag (bit 2) is 0 in at least one of
     * the page-structure entries, the address is a supervisor-mode address.
     * Otherwise, the address is a user-mode address.
     */

#ifdef CONFIG_DEBUG_RT_SUP_SUPDATA
    SUP_supervisor_data();
#endif

#ifdef CONFIG_DEBUG_RT_SUP_USERDATA
    SUP_user_data();
#endif

#ifdef CONFIG_DEBUG_RT_WR_SUP
    SUP_wr_supervisor_data();
#endif

#ifdef CONFIG_DEBUG_RT_WR_USER
    SUP_wr_user_data();
#endif

    return 0;
}
late_debugcall(paging_right_supervisor);
#endif

#ifdef CONFIG_DEBUG_MMU_RT_USER
/*
 * For supervisor-mode accesses:
 *
 *   Every access to a linear address is either a supervisor-mode access or a
 *   user-mode access. For all instruction fetures and most data accesses, the
 *   distinction is determined by the current privilege level (CPL): while 
 *   accesses made while CPL = 3 are user-mode accesses.
 */
static int __unused paging_right_user(void)
{
    /*
     * Access rights are also controlled by the mode of a linear address as
     * specified by the paging-structure entries controlling the translation
     * of the linear address. If the U/S flag (bit 2) is 0 in at least one of
     * the page-structure entries, the address is a supervisor-mode address.
     * Otherwise, the address is a user-mode address.
     */
    return 0;
}
user1_debugcall_sync(paging_right_user);
#endif
