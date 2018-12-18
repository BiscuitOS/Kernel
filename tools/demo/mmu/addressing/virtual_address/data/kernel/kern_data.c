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

#ifdef CONFIG_DEBUG_VA_KERNEL_DMA
static unsigned int *DMA_int = NULL;
#endif

#ifdef CONFIG_DEBUG_VA_KERNEL_NORMAL
static unsigned int *NORMAL_int = NULL;
#endif

#ifdef CONFIG_DEBUG_VA_KERNEL_VMALLOC
static unsigned int *VMALLOC_int = NULL;
#endif

#if defined CONFIG_DEBUG_VA_KERNEL_KMAP && defined CONFIG_X86_32
static unsigned int *KMAP_int = NULL;
static struct page *high_page = NULL;
#endif

/* Initialization entence */
static __init int kern_space_init(void)
{
    printk("***************************************************************\n");
    printk("Data Segment: .data Describe\n\n");
    printk("+--------+-----+--------------+-------------+-------+--------+\n");
    printk("|        |     |              |             |       |        |\n");
    printk("|  User  | DMA |    Normal    |   VMALLOC   | PKMAP | FIXMAP |\n");
    printk("|        |     |              |             |       |        |\n");
    printk("+--------+-----+--------------+-------------+-------+--------+\n");
    printk("0       3G                                                  4G\n");
    printk("DMA:          %#08x -- %#08x\n", (unsigned int)PAGE_OFFSET, 
                                             (unsigned int)MAX_DMA_ADDRESS);
    printk("Normal:       %#08x -- %#08x\n", (unsigned int)MAX_DMA_ADDRESS,
                               (unsigned int)(PAGE_OFFSET + 896 * (1 << 20)));
    printk("VMALLOC:      %#08x -- %#08x\n", (unsigned int)VMALLOC_START,
                                             (unsigned int)VMALLOC_END);
    printk("PKMAP:        %#08x -- %#08x\n", (unsigned int)PKMAP_BASE,
                                             (unsigned int)(FIXADDR_START));
    printk("FIXMAP:       %#08x -- %#08x\n", (unsigned int)FIXADDR_START,
                                             (unsigned int)FIXADDR_TOP);
    printk("***************************************************************\n");

#ifdef CONFIG_DEBUG_VA_KERNEL_DMA
    DMA_int = (unsigned int *)kmalloc(sizeof(unsigned int), GFP_DMA);
    printk("[*]unsigned int *DMA_int:        Address: %#08x\n", 
                              (unsigned int)(unsigned long)DMA_int);
    if (DMA_int)
        kfree(DMA_int);
#endif

#ifdef CONFIG_DEBUG_VA_KERNEL_NORMAL
    NORMAL_int = (unsigned int *)kmalloc(sizeof(unsigned int), GFP_KERNEL);
    printk("[*]unsigned int *NORMAL_int:     Address: %#08x\n", 
                                 (unsigned int)(unsigned long)NORMAL_int);
    if (NORMAL_int)
        kfree(NORMAL_int);
#endif

#ifdef CONFIG_DEBUG_VA_KERNEL_VMALLOC
    VMALLOC_int = (unsigned int *)vmalloc(sizeof(unsigned int));
    printk("[*]unsigned int *VMALLOC_int:    Address: %#08x\n", 
                               (unsigned int)(unsigned long)VMALLOC_int);
    if (VMALLOC_int)
        vfree(VMALLOC_int);
#endif

#if defined CONFIG_DEBUG_VA_KERNEL_KMAP && defined CONFIG_X86_32
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
