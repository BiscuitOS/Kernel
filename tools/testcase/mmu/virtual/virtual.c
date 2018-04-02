/*
 * Virtual Address Mechanism on MMU
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <unistd.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/head.h>
#include <test/debug.h>

#include <string.h>

static char *argv[] = { "-/bin/sh", "/usr/bin", NULL };
static char tmp[80];
/*
 * system call for virtual address to physical address.
 */
int sys_d_mmu(const char *filename, char **argv, char *buffer)
{
    unsigned long fs;
    unsigned long base;
    unsigned long *linear;
    unsigned long *virtual;
    unsigned long *physical;
    struct desc_struct *desc;
    unsigned long cr3, cr4;
    unsigned long *pgdir; /* pgdir array */
    unsigned long *PDE; /* Page direntry */
    unsigned long *ptedir; /* pte array */
    unsigned long *PTE; /* Page table */
    unsigned char *page; /* Page base address */

    /* Obtain segment selector for userland. The userland utilzes
     * fs as data segment */
    __asm__ ("movl %%fs, %0" : "=r" (fs));

    /* Obtain data segment descriptor from LDT or GDT */
    if ((fs >> 2) & 0x1) {
        /* Segment descriptor locate in LDT */
        desc = &current->ldt[fs >> 3];
    } else {
        /* Segment descriptor locate in GDT */
        desc = &gdt[fs >> 3];
    }

    /* Obtain segment base linear address */
    base = get_base(*desc);

    /* Obtain virtual address */
    virtual = (unsigned long *)filename;

    /* Obtain linear address,
     * linear = Segment-base + virtual  */
    linear = (unsigned long *)((unsigned long)base + (unsigned long)virtual);

    /* Obtain CR3 register that contain physical address of pgdir,
     * On linux0.11, linus thinks physical address of pgdir is 0, so
     * some assume had been used in procedure such as copy_page_tables() 
     * etc. */
    __asm__ ("movl %%cr3, %0" : "=r" (cr3));

    /* A 4-KByte naturally aligned page directory is located at the 
     * physical address specified in bits 31:12 of CR3. A page directory
     * comprises 1024 32-bit entries (PDEs). A PDE is selected using
     * the physical address defined as follow:
     *
     * -- Bits 39:32 are all 0
     * -- Bits 31:12 are from CR3.
     * -- Bits 11:2  are bits 31:22 of the linear address 
     * -- Bits  1:0  are 0. 
     *
     * Becasue a PDE is identified using bits 31:22 of the linear address,
     * it control access to a 4-Mbytes region of the linear-address space.
     * Use of the PDE depends on CR4.PSE and PDE's PS flag (bit 7)
     *
     * 31-------------12--------------2----0
     * |  CR3[31:12]    | Linar[31:22] | 0 |
     * -------------------------------------
     **/
    pgdir = (unsigned long *)((unsigned long)cr3 & 0xFFFFF000);

    /* PDE: a entry of PTE table */
    PDE = (unsigned long *)&pgdir[((unsigned long)linear >> 22) & 0x3FF];

    __asm__ ("movl %%cr4, %0" : "=r" (cr4));
    if (((cr4 >> 4) & 0x1) && ((*PDE >> 4) & 0x1)) {
        /* If CR4.PSE = 1 and the PDE's PS flag is 1, the PDE maps a 4-MByte
         * page. The final physical address is computed as follows:
         * 
         * -- Bits 39:32 are bits 20:13 of the PDE.
         * -- Bits 31:22 are bits 31:22 of the PDE.
         * -- Bits 21:0  are from the original linear address.
         */
        ptedir = 0; /* No complete on BiscuitOS */
    } else if (!((cr4 >> 4) & 0x1) && !((*PDE >> 4) & 0x1)) {
        /* If CR4.PSE = 0 or the PDE's PS flag is 0, a 4-KByte naturally
         * aligned page table is located at the physical address specified
         * in bits 31:12 of the PDE. A page table comprises 1024 32-bit
         * entries (PTEs). A PTE is selected using the physical address
         * defined as follows:
         *
         * -- Bits 39:32 are all 0
         * -- Bits 31:12 are from the PDE. 
         * -- Bits 11:2  are bits 21:12 of the linear address. 
         * -- Bits  1:0  are 0.
         *
         * 31-----------12---------------2----0
         * |  PDE[31:12]  | Linear[21:12] | 0 |
         * ------------------------------------
         */
        ptedir = (unsigned long *)((unsigned long)*PDE & 0xFFFFF000);
    }
    /* Obtain PTE of linear address */
    PTE = (unsigned long *)&ptedir[((unsigned long)linear >> 12) & 0x3FF];

    /* Because a PTE is identified using bits 31:12 of the linear address,
     * every PTE maps a 4-KByte page. The final physical address is 
     * computed as follows:
     *
     * -- Bits 39:32 are all 0
     * -- Bits 31:12 are from the PTE.
     * -- Bits 11:0  are from the original linear address. 
     *
     * 31-----------12----------------
     * |  PTE[31:12]  | Linear[11:0] |
     * -------------------------------
     */
    page = (unsigned char *)((unsigned long)*PTE & 0xFFFFF000);

    /* Obtain physical address */
    physical = (unsigned long *)(page + ((unsigned long)linear & 0x3FF));
    printk("Virtual Address: %#x\n", virtual);
    printk("Logical Address: %#x:%#x\n", virtual, fs);
    printk("Segment Address: %#x\n", base);
    printk("Linear  Address: %#x\n", linear);
    printk("Page - Offset  : %#x:%#x\n", page, (unsigned long)linear & 0x3FF);
    printk("Phyical Address: %#x\n", physical);
    return 0;
}

/* common linear address userland entry */
void debug_virtual_address_common_userland(void)
{
    char buffer[40];

    /* violation page-fault */
    strcpy(tmp, "Hello BiscuitOS");
    d_printf("String: %s\n", tmp);

    d_mmu(tmp, argv, buffer);
}

