/*
 * Kernel virtual space on i386
 *
 * Copyright (C) 2018.12.16 BuddyZhang1 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include <asm/dma.h>
#include <asm/highmem.h>
#include <asm/fixmap.h>

#ifdef CONFIG_DEBUG_VA_KERNEL_VMALLOC
#include <linux/vmalloc.h>
#endif

#ifdef CONFIG_DEBUG_VA_KERNEL_FIXMAP
#include <linux/highmem.h>
#endif

#ifndef CONFIG_X86_32
/* PKMAP Layout */
#ifdef CONFIG_X86_PAE
#define LAST_PKMAP 512
#else
#define LAST_PKMAP 1024
#endif
#define PKMAP_BASE ((FIXADDR_START - PAGE_SIZE * (LAST_PKMAP + 1))    \
                          & PMD_MASK)
#endif

/*
 * Virtual kernel memory layout: (IA32)
 *
 * 4G +----------------+
 *    |                |
 *    +----------------+-- FIXADDR_TOP
 *    |                |
 *    |                | FIX_KMAP_END
 *    |     Fixmap     |
 *    |                | FIX_KMAP_BEGIN
 *    |                |
 *    +----------------+-- FIXADDR_START
 *    |                |
 *    |                |
 *    +----------------+--
 *    |                | A
 *    |                | |
 *    |   Persistent   | | LAST_PKMAP * PAGE_SIZE
 *    |    Mappings    | |
 *    |                | V
 *    +----------------+-- PKMAP_BASE
 *    |                |
 *    +----------------+-- VMALLOC_END / MODULE_END
 *    |                |
 *    |                |
 *    |    VMALLOC     |
 *    |                |
 *    |                |
 *    +----------------+-- VMALLOC_START / MODULE_VADDR
 *    |                | A
 *    |                | |
 *    |                | | VMALLOC_OFFSET
 *    |                | |
 *    |                | V
 *    +----------------+-- high_memory
 *    |                |
 *    |                |
 *    |                |
 *    | Mapping of all |
 *    |  physical page |
 *    |     frames     |
 *    |    (Normal)    |
 *    |                |
 *    |                |
 *    +----------------+-- MAX_DMA_ADDRESS
 *    |                |
 *    |      DMA       |
 *    |                |
 *    +----------------+
 *    |     .bss       |
 *    +----------------+
 *    |     .data      |
 *    +----------------+
 *    | 8k thread size |
 *    +----------------+ 
 *    |     .init      |
 *    +----------------+
 *    |     .text      |
 *    +----------------+ 0xC0008000
 *    | swapper_pg_dir |
 *    +----------------+ 0xC0004000
 *    |                |
 * 3G +----------------+-- TASK_SIZE / PAGE_OFFSET
 *    |                |
 *    |                |
 *    |                |
 * 0G +----------------+
 *
 */

/* Kernel virtual map layout */
static unsigned int kernel_text;     /* Base address of kernel code segment */
static unsigned int kernel_etext;    /* End address of kernel code segment */
static unsigned int kernel_data;     /* Base address of kernel data segment */
static unsigned int kernel_edata;    /* End address of kernel data segment */
static unsigned int kernel_init;     /* Base address of kernel init segment */
static unsigned int kernel_einit;    /* End address of kernel init segment */

/* kernel DMA area */
static unsigned int kernel_DMA; 
static unsigned int kernel_eDMA; 

/* kernel Normal area */
static unsigned int kernel_normal; 
static unsigned int kernel_enormal; 

/* kernel vmalloc area */
static unsigned int kernel_vmalloc; 
static unsigned int kernel_evmalloc; 

/* PKMAP */
static unsigned int kernel_pkmap; 
static unsigned int kernel_epkmap; 

/* Fixmap */
static unsigned int kernel_fixmap; 
static unsigned int kernel_efixmap; 

/* Module */
static unsigned int kernel_module; 
static unsigned int kernel_emodule; 

/* Initialization entence */
static __init int kern_space_init(void)
{
    kernel_DMA        = (unsigned int)(unsigned long)PAGE_OFFSET;
    kernel_eDMA       = (unsigned int)(unsigned long)MAX_DMA_ADDRESS;
    kernel_normal     = (unsigned int)(unsigned long)MAX_DMA_ADDRESS;
    kernel_enormal    = (unsigned int)(unsigned long)high_memory;
    kernel_vmalloc    = (unsigned int)(unsigned long)VMALLOC_START;
    kernel_evmalloc   = (unsigned int)(unsigned long)VMALLOC_END;
    kernel_pkmap      = (unsigned int)(unsigned long)PKMAP_BASE;
    kernel_epkmap     = (unsigned int)(unsigned long)(PKMAP_BASE +
                                               (LAST_PKMAP * PAGE_SIZE));
    kernel_fixmap     = (unsigned int)(unsigned long)FIXADDR_START;
    kernel_efixmap    = (unsigned int)(unsigned long)FIXADDR_TOP;
    kernel_module     = (unsigned int)(unsigned long)MODULES_VADDR;
    kernel_emodule    = (unsigned int)(unsigned long)MODULES_END;

    printk("***************************************************************\n");
    printk("Data Segment: .data Describe\n\n");
    printk("+--------+-----+--------------+-------------+-------+--------+\n");
    printk("|        |     |              |             |       |        |\n");
    printk("|  User  | DMA |    Normal    |   VMALLOC   | PKMAP | FIXMAP |\n");
    printk("|        |     |              |             |       |        |\n");
    printk("+--------+-----+--------------+-------------+-------+--------+\n");
    printk("0       3G                                                  4G\n");
    printk(".text         %#08x -- %#08x\n", kernel_text, kernel_etext);
    printk(".data         %#08x -- %#08x\n", kernel_data, kernel_edata);
    printk(".init         %#08x -- %#08x\n", kernel_init, kernel_einit);
    printk("DMA:          %#08x -- %#08x\n", kernel_DMA, kernel_eDMA); 
    printk("Normal:       %#08x -- %#08x\n", kernel_normal, kernel_enormal);
    printk("VMALLOC:      %#08x -- %#08x\n", kernel_vmalloc, kernel_evmalloc);
    printk("Module:       %#08x -- %#08x\n", kernel_module, kernel_emodule);
    printk("PKMAP:        %#08x -- %#08x\n", kernel_pkmap, kernel_epkmap);
    printk("FIXMAP:       %#08x -- %#08x\n", kernel_fixmap, kernel_efixmap);
    printk("***************************************************************\n");

#ifdef CONFIG_DEBUG_VA_KERNEL_DMA
    /*
     * DMA Virtual Space
     * 0    3G                                                  4G
     * +----+---------------------------------+-----------------+
     * |    |                                 |                 |
     * |    |        DMA Virtual Space        |                 |
     * |    |                                 |                 |
     * +----+---------------------------------+-----------------+
     *      A                                 A
     *      |                                 |
     *      |                                 |
     *      |                                 |
     *      o                                 o
     *  PAGE_OFFSET                     MAX_DMA_ADDRESS
     */
    unsigned int *DMA_int = NULL;

    DMA_int = (unsigned int *)kmalloc(sizeof(unsigned int), GFP_DMA);
    printk("[*]unsigned int *DMA_int:        Address: %#08x\n", 
                              (unsigned int)(unsigned long)DMA_int);
    if (DMA_int)
        kfree(DMA_int);
#endif

#ifdef CONFIG_DEBUG_VA_KERNEL_NORMAL
    /*
     * Normal Virtual Space
     * 0    3G                                                  4G
     * +----+--+-----------------------------------+------------+
     * |    |  |                                   |            |
     * |    |  |       Normal Virtual Space        |            |
     * |    |  |                                   |            |
     * +----+--+-----------------------------------+------------+
     *         A                                   A
     *         |                                   |
     *         |                                   |
     *         |                                   |
     *         o                                   o    
     *  MAX_DMA_ADDRESS                       high_memory
     */
    unsigned int *NORMAL_int = NULL;

    NORMAL_int = (unsigned int *)kmalloc(sizeof(unsigned int), GFP_KERNEL);
    printk("[*]unsigned int *NORMAL_int:     Address: %#08x\n", 
                                 (unsigned int)(unsigned long)NORMAL_int);
    if (NORMAL_int)
        kfree(NORMAL_int);
#endif

#ifdef CONFIG_DEBUG_VA_KERNEL_VMALLOC
    /*
     * VMALLOC Virtual Space
     * 0    3G                                                  4G
     * +----+----+----------------+-----------------------+-----+
     * |    |    |                |                       |     |
     * |    |    |                | VMALLOC Virtual Space |     |
     * |    |    |                |                       |     |
     * +----+----+----------------+-----------------------+-----+
     *           A                A                       A
     *           |                |                       |
     *           | <------------> |                       |
     *           | VMALLOC_OFFSET |                       |
     *           |                |                       |
     *           o                o                       o
     *      high_memory      VMALLOC_START           VMALLOC_END
     *
     * Just any arbitrary offset to the start of the vmalloc VM area: the
     * current 8MB value just means that there will be a 8MB "hole" after
     * the physical memory until the kernel virtual memory starts. That 
     * means that any out-of-bounds memory accesses will hopefully be
     * caught. The vamlloc() routines leasves a hole of 4KB between each
     * vammloced area for the same reason. :)
     */
    unsigned int *VMALLOC_int = NULL;

    VMALLOC_int = (unsigned int *)vmalloc(sizeof(unsigned int));
    printk("[*]unsigned int *VMALLOC_int:    Address: %#08x\n", 
                               (unsigned int)(unsigned long)VMALLOC_int);
    if (VMALLOC_int)
        vfree(VMALLOC_int);
#endif

#ifdef CONFIG_DEBUG_VA_KERNEL_MODULE
    /*
     * Moudule Virtual Space
     * 0    3G                                                  4G
     * +----+----+---------------+------------------------+-----+
     * |    |    |               |                        |     |
     * |    |    |               |  Module Virtual Space  |     |
     * |    |    |               |                        |     |
     * +----+----+---------------+------------------------+-----+
     *                           A                        A
     *                           |                        |
     *                           |                        |
     *                           |                        |
     *                           o                        o
     *                      MODULES_VADDR             MODULES_END
     */
    static unsigned int MODULE_int = 0x2018;

    printk("[*]unsigned int MODULE_int:    Address: %#08x\n",
                               (unsigned int)(unsigned long)&MODULE_int);
#endif

#if defined CONFIG_DEBUG_VA_KERNEL_KMAP && defined CONFIG_X86_32
    /*
     * PKMAP Virtual Space.
     * 0    3G                                                  4G
     * +----+----------+---------------------------------+------+
     * |    |          |                                 |      |
     * |    |          |   Persister Mappings (FIXMAP)   |      |
     * |    |          |                                 |      |
     * +----+----------+---------------------------------+------+
     *                 A                                 A
     *                 |                                 |
     *                 |                                 |
     *                 |                                 |
     *                 o                                 o
     *             PKMAP_BASE           PKMAP_BASE+(LAST_PKMAP * PAGE_SIZE))
     */

    struct page *high_page  = NULL;
    unsigned int *KMAP_init = NULL;

    /* Allocate a page from Highmem */
    high_page = alloc_pages(__GFP_HIGHMEM, 0);
    /* Map on Kmap */
    KMAP_int  = kmap(high_page);
    printk("[*]unsigned int *KMAP_int:       Address: %#08x\n", 
                               (unsigned int)(unsigned long)KMAP_int);
    if (KMAP_int)
        kunmap(high_page);
    __free_pages(high_page, 0);
#endif

#if defined CONFIG_DEBUG_VA_KERNEL_FIXMAP && defined CONFIG_X86_32
    /*
     * FIXMAP Virtual Space.
     * 0    3G                                                  4G
     * +----+---------------------------+----------------+------+
     * |    |                           |                |      |
     * |    |                           |  Fixmap Space  |      |
     * |    |                           |                |      |
     * +----+---------------------------+----------------+------+
     *                                  A                A
     *                                  |                |
     *                                  |                |
     *                                  |                |
     *                                  o                o
     *                             FIXADDR_START    FIXADDR_TOP
     *
     * Here define all the compile-time 'special' virtual addresses. The
     * point is to have a constant address at compile time, but to set the
     * physical address only in the boot process.
     *
     * For IA32: We allocate these speical addresses from the end of virtual
     * memory (0xfffff000) backwards. Also this lets us do fail-safe 
     * vmalloc(), we can gurarntee that these special addresses and 
     * vmalloc()-ed addresses never overlap.
     *
     * These 'compile-time allocated' memory buffers are fixed-size 4K pages
     * (or larger if used with an increament higher than 1). Use set 
     * fixmap(idx, phys) to associate physical memory with fixmap indices.
     *
     * TLB entries of such buffers will not be flushed accross task switch.
     */

    struct page *high_page;
    int idx, type;
    unsigned long vaddr;

    /* Allocate a physical page */
    high_page = alloc_page(__GFP_HIGHMEM);

    /* Obtain current CPU's FIX_KMAP_BEGIN */
    type = kmap_atomic_idx_push();
    idx  = FIX_KMAP_BEGIN + type + KM_TYPE_NR * smp_processor_id();

    /* Obtain fixmap virtual address by index */
    vaddr = fix_to_virt(idx);
    /* Associate fixmap virtual address with physical address */
    set_fixmap(idx, page_to_phys(high_page));

    printk("[*]unsignd long vaddr:       Address: %#08x\n", 
                               (unsigned int)(unsigned long)vaddr);

    /* Remove associate with fixmap */
    clear_fixmap(idx);
#endif

    /* Must return 0 for here! */
    return 0;
}

/* Exit entence */
static __exit void kern_space_exit(void)
{
}

/* Module information */
module_init(kern_space_init);
module_exit(kern_space_exit);

MODULE_LICENSE("GPL v2");
