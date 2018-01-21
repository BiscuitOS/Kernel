/*
 * Logical Address Mechanism on MMU
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/sched.h>

#include <test/debug.h>

static char var[50] = "BiscuitOS";
/*
 * Obtain vairable loagic address that from stack segment, 
 * code segment, data segment.
 */
static void obtain_logic_address(void)
{
    unsigned int a_var;
    unsigned int cs, ds, ss;
    struct logic_addr stack_logic, data_logic, code_logic;

    __asm__ ("movl %%cs, %0\n\r"
             "movl %%ds, %1\n\r"
             "movl %%ss, %2"
             : "=r" (cs), "=r" (ds), "=r" (ss));
    /* A logical address consists of a 16-bit segment selector and 
     * a 32-bit offset */
    /* The logic address on stack segment */
    stack_logic.offset = (unsigned long)&a_var;
    stack_logic.sel    = ss;
    /* The logic address on data segment */
    data_logic.offset = (unsigned long)&var;
    data_logic.sel    = ds;
    /* The logic address on code segment */
    code_logic.offset = (unsigned long)obtain_logic_address;
    code_logic.sel    = cs;

    printk("var  -> logic address: %#8x:%#x\n", 
                   data_logic.sel, data_logic.offset);
    printk("a_var-> logic address: %#8x:%#x\n",
                   stack_logic.sel, stack_logic.offset);
    printk("func -> logic address: %#8x:%#x\n",
                   code_logic.sel, code_logic.offset);
}

/*
 * Logical address convent to linear address
 */
static void logic_to_linear(void)
{
    unsigned long linear;
    unsigned long ds, cs;
    unsigned long cpl, dpl;
    unsigned long base, limit;
    struct logic_addr la;
    struct desc_struct *desc;

    /* Establlise a varible on Data segment */
    __asm__ ("movl %%ds, %0" : "=r" (ds));

    /* Logical address for "var" */
    la.offset = (unsigned long)&var;
    la.sel    = (unsigned long)ds;

    /* Uses the offset in the segment selector to locate the segment
     * descriptor for the segment in the GDT or LDT and reads it into
     * the processor. (This step is needed only when a new segment selector
     * is loaded into a segment register.) */
    if ((ds >> 2) & 0x1) {
        /* Segment descriptor locate in LDT */
        desc = &current->ldt[ds >> 3];
        base = get_base(*desc);
        limit = get_limit(*desc);
    } else {
        /* Segment descriptor locate in GDT */
        desc = &gdt[ds >> 3];
        base = get_base(*desc);
        limit = get_limit(*desc);
    }
    /* Examines the segment descriptor to check the access rights and range
     * of the segment to insure that the segment is accessible and that
     * offset is within the limit of the segment */
    limit = limit * 4096;
    if (la.offset > limit) {
        panic("Out of range for segment\n");
    }
    /* Obtain CPL from code selector */
    __asm__ ("movl %%cs, %0" : "=r" (cs));
    cpl = cs & 0x3;
    dpl = desc->b >> 13 & 0x3;
    if (cpl > dpl) {
        panic("Trigger #GP\n");
    }

    /* Adds the base address of segment from the segment descriptor to the 
     * offset to from a linear address. */
    linear = base + la.offset;

    printk("Logic  Address: %#8x:%#x\n", la.sel, la.offset);
    printk("Linear Address: %#8x\n", linear);
}

/*
 * Logical address convent to physical address
 */
static void logic_to_physic(void)
{
    struct logic_addr la;
    unsigned long virtual, linear, physic, physic2;
    unsigned long base, limit;
    unsigned long cs, ds, cr3, cr4;
    unsigned char cpl, dpl;
    struct desc_struct *desc;
    unsigned long PDE, PTE, *PDE2, *PTE2;
    unsigned char *page;

    /* Obtain specific segment selector */
    __asm__ ("movl %%cs, %0\n\r"
             "movl %%ds, %1"
             : "=r" (cs), "=r" (ds));

    /* Establish a logical address */
    la.offset = (unsigned long)&var;
    la.sel    = ds;

    /* Obtain virtual address */
    virtual = la.offset;

    /* violation COW */
    var[48] = 'A';

    /* Uses the offset in the segment selector to locate the segment
     * descriptor for the segment in the GDT or LDT and reads it into
     * the processor. (This step is needed only when a new segment selector
     * is loaded into a segment register.) */
    if ((la.sel >> 2) & 0x1)
        desc = &current->ldt[la.sel >> 3];
    else
        desc = &gdt[la.sel >> 3];
    /* Examines the segment descriptor to check the access rights and range
     * of the segment to insure that the segment is accessible and that
     * offset is within the limit of the segment */
    base  = get_base(*desc);
    limit = get_limit(*desc) * 4096;
    if (la.offset > limit)
        panic("Out of segment\n");
    cpl = cs & 0x3;
    dpl = desc->b >> 13 & 0x3;
    if (cpl > dpl)
        panic("Trigger #GP");

    /* Obtain linear address on flat-protect model */
    linear = base + la.offset;
    
    /* Obtain physical address of pgdir from CR3 register, the contents of
     * 'cr3' is point to the physical of pg_dir, so refers it as a pointer. */
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
    PDE =  ((unsigned char)(((cr3 >> 12) & 0xFFFFF) << 12) +
           (((linear >> 22) & 0x3FF) << 2)) & 0xFFFFFFFC;
    /* Above figure, that CR3 is a unsigned long array and array base
     * address[31:12] is CR3[31:12], other bits are zero. And Linear[31:22]
     * is index of array, So procedure can find specific PTD from CR3 via
     * index. Another way to obtain PDE address is:
     *   P_CR3 = CR3 & 0xFFFFF000 
     *   PDE   = P_CR3[linear >> 22];
     */
    PDE2  = (unsigned long *)(cr3 & 0xFFFFF000);     /* CR3 base address */
    PDE2  = &PDE2[linear >> 22];  /* PDE on CR3 */

    __asm__ ("movl %%cr4, %0" : "=r" (cr4));
    if (((cr4 >> 4) & 0x1) && ((PDE >> 4) & 0x1)) {
        /* If CR4.PSE = 1 and the PDE's PS flag is 1, the PDE maps a 4-MByte
         * page. The final physical address is computed as follows:
         * 
         * -- Bits 39:32 are bits 20:13 of the PDE.
         * -- Bits 31:22 are bits 31:22 of the PDE.
         * -- Bits 21:0  are from the original linear address.
         */
        PTE = 0x0; /* No complete on BiscuitOS */
    } else if (!((cr4 >> 4) & 0x1) && !((PDE >> 4) & 0x1)) {
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
        PTE = (unsigned long)((unsigned char *)(
              ((*(unsigned long *)PDE >> 12) & 0xFFFFF) << 12) +
              (((linear >> 12) & 0x3FF) << 2)) & 0xFFFFFFFC;
        /* Above Figure, PTE from a array that comprises PDE[31:12] and
         * Linear[21:12]. The linear[21:12] offer offset on array and 
         * PDE[31:12] is base address, so procedure can compute PTE as:
         *   PDE2 = *PDE2 & 0xFFFFF000
         *   PTE  = &PDE2[(linear >> 12) & 0x3FF] */
        PTE2 = (unsigned long *)(*PDE2 & 0xFFFFF000); /* PTE base address */
        PTE2 = &PTE2[(linear >> 12) & 0x3FF];
    }
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
    physic = (unsigned long)(unsigned char *)(
             ((*(unsigned long *)PTE >> 12) & 0xFFFFF) << 12) +
             (linear & 0xFFF);
    /* Above Figure, Physical address from a array, and array base address
     * is PET[31:12] and index is Linear address[11:0], the length of member
     * on array is 1 Byte. So, another way to obtain physica address is:
     *    PageBase = *PTE2 & 0xFFFFF000
     *    Physic   = PageBase + offset
     **/
    page = (unsigned char *)(*PTE2 & 0xFFFFF000);
    physic2 = (unsigned long)&page[linear & 0xFFF]; 

    printk("Logical Address: %#x:%#x\n", la.sel, la.offset);
    printk("Virtual Address: %#x\n", virtual);
    printk("Linear  Address: %#x\n", linear);
    printk("Physic  Address: %#x\n", physic);
    printk("Physic2 Address: %#x\n", physic2);
    printk("Original:  %s\n", la.offset);
    printk("Translate: %s\n", physic);
    printk("Translate: %s\n", physic2);
}

/* Brief logic to physic */
static void logic_2_phys_brief(void)
{
    unsigned long *virtual, *linear, *physic;
    unsigned long cr3, ds, *PDE, *PTE, *base;
    unsigned char *page;
    struct desc_struct *desc;
    struct logic_addr la;

    /* Establish logical address */
    la.offset = (unsigned long)&var;
    __asm__ ("movl %%ds, %0" : "=r" (ds));
    la.sel = ds;

    /* Obtain virtual address */
    virtual = (unsigned long *)la.offset;

    /* Obtain linear address */
    if ((la.sel >> 2) & 0x1)
        desc = &current->ldt[la.sel >> 3];
    else
        desc = &gdt[la.sel >> 3];
    linear = (unsigned long *)(la.offset + get_base(*desc));

    /* Obtain base address of PDE */
    __asm__ ("movl %%cr3, %0" : "=r" (cr3));
    base = (unsigned long *)(cr3 & 0xFFFFF000);

    /* Obtain PDE */
    PDE = &base[(unsigned long)linear >> 22];
    
    /* Obtain PTE */
    base = (unsigned long *)((unsigned long)*PDE & 0xFFFFF000);
    PTE  = &base[((unsigned long)linear >> 12) & 0x3FF];

    /* Obtain page */
    page = (unsigned char *)((unsigned long)*PTE & 0xFFFFF000);

    /* Obtain physical address */
    physic = (unsigned long *)&page[(unsigned long)linear & 0xFFF];

    printk("Logic   Address: %#8x:%#x\n", la.offset, la.sel);
    printk("Virtual Address: %#8x\n", virtual);
    printk("Linear  Address: %#8x\n", linear);
    printk("Phyisc  Address: %#8x\n", physic);
    printk("Original: %s\n", la.offset);
    printk("Physic:   %s\n", physic);
}

/* common linear address entry */
void debug_logic_address_common(void)
{
    if (1) {
        logic_2_phys_brief();
    } else {
        obtain_logic_address();
        logic_to_linear();
        logic_to_physic();
        logic_2_phys_brief();
    }
}
