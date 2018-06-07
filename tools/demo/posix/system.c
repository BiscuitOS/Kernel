/*
 * System call
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <test/debug.h>
/*
 * In computing, a system call is the programmatic way in which a computer 
 * program requests a service from the kernel of the operating system it is 
 * executed on. This may include hardware-related services (for example, 
 * accessing a hard disk drive), creation and execution of new processes, 
 * and communication with integral kernel services such as process scheduling.
 * System calls provide an essential interface between a process and the 
 * operating system.
 */

/*
 * Tirgger system call
 *   On X86, the user program triggers "0x80" interrupt to access system 
 *   call. When program invoke system call on userland, the system will
 *   be interrupted and find a entry for "0x80" on IDT. After system 
 *   obtained specific IDT-descriptor, the system will parse it and obtain
 *   selector and offset of segment descriptor for system call. The 
 *   segment descriptor may be locate in LDT or GDT, the system detect 
 *   location with selector.
 *
 *   If the selector locates in LDT, the system will find LDT on GDT, and
 *   obtain specific segment descriptor on LDT. If the selector points
 *   GDT, the system directly obtain segment descriptor on GDT. After obtain
 *   segment descriptor, the system will parse it and obtain base address
 *   and access right and so on.
 *
 *   The last, the system will calculate the address for system call with
 *   base address and offset.
 */
static void trigger_system_call(void)
{
    unsigned short selector;
    unsigned long  offset;
    unsigned long  base;
    unsigned long  syscall_addr;
    extern int system_call(void);

    /*
     * First, the program triggers "0x80" interrupt, the system will access
     * IDTR instantly to obtain the base address of IDT and the limit of
     * IDTR.
     */
    unsigned char idtr[6];
    unsigned long idt_base;
    unsigned long *trap_gate[2];

    __asm__ ("sidt %0"
             : "=m" (idtr));

    idt_base = idtr[2] | (idtr[3] << 8) | (idtr[4] << 16) | (idtr[5] << 24);

    /*
     * The length of Interrupt Segment Descriptor is 8-byte, and the vector
     * range is 0 to 255. The system obtains specific descriptor in vector
     * and length of descriptor, e.g:
     *   Descriptor = length * vector = 8 * vector
     * The vector of system call is 0x80, so, the descriptor of system call 
     * on IDT is:
     */
    /* LSB of descriptor */
    trap_gate[0] = (unsigned long *)(0x80 * 8 + (unsigned char *)idt_base);
    /* MSB of descriptor */
    trap_gate[1] = (unsigned long *)(4 + (unsigned char *)trap_gate[0]);

    /*
     * Parse Interrupt descriptor
     *   The descriptor of system call is system gate, the layer of system
     *   gate as follow:
     *   Trap Gate
     * 31---------------------------16-15--14---13-12--------8-7-----5-4--0
     * | Offset 31:16                 | P | DPL   | 0 D 1 1 1 | 0 0 0 |   | 4
     * --------------------------------------------------------------------
     * 31---------------------------16-15---------------------------------0
     * | Segment Selector             | Offset 15:0                       | 0
     * -------------------------------------------------------------------0
     */
    selector = (*(trap_gate[0]) >> 16);
    offset   = (*(trap_gate[0]) & 0xFFFF) | 
               (((*(trap_gate[1]) >> 16) & 0xFFFF) << 16);

    /*
     * After obtain the selector of system call, the system will find it on
     * LDT or GDT, and then diagnose whether selector located on LDT or GDT
     * that bit 3 of selecot sepcify the location of selector. 
     */
    if ((selector >> 2) & 1) {
        /* The selector locate on LDT */
        unsigned short selector_ldt;
        unsigned char __base[6];
        unsigned long *gdt_base;
        unsigned long *ldt_seg[2];
        unsigned long ldt_base;
        unsigned long *sys_seg[2];
        /*
         * Parse LDTR
         *   The LDTR register holds the 16-bit segment selector, base 
         *   address (32 bits in protected mode), segment limit, and 
         *   descriptor attributes for the LDT. The base address specifies
         *   the linear address of byte 0 of the LDT segment. The segment
         *   limit specifies the number of bytes in the segment.
         *
         *   The LLDT and SLDT instructions load and store the segment 
         *   selector part of the LDTR register, respectively. The segment 
         *   that contains the LDT must have a segment a segment descriptor
         *   in the GDT. When the LLDT instruction loads a segment in the 
         *   LDTR. the base address, limit, and descriptor attributes from 
         *   the LDT descriptor are auto-matically loaded in the LDTR.
         *
         */
        __asm__ ("sldt %0"
                 : "=m" (selector_ldt));

        /*
         * The Descirptor of LDT must hold on GDT, so obtain the segment
         * descriptor from GDT.
         *
         * Parse GDTR
         *   The GDTR register holds the base address (32 bits in protected 
         *   mode) and the 16-bit table limit for the GDT. The base address
         *   specifies the linear address of byte 0 of the GDT. The table
         *   limit specifies the number of bytes in the table.
         * 
         *   The LGDT and SGDT instruction load and store the GDTR register,
         *   respectively. On power up or reset of the processor, the base
         *   address is set to the default value of 0 and the limit is set
         *   to 0xffffh. A new base address must be loaded into the GDTR as
         *   part of the processor initialization process for protected-mode
         *   operation.
         */
        __asm__("sgdt %0"
                : "=m" (__base));
        gdt_base = (unsigned long *)(unsigned long)__base[2];

        /*
         * Obtain segment descriptor
         *   The index of segment selector is selector[15:3]
         */
        ldt_seg[0] = (unsigned long *)(((selector_ldt >> 3) * 8) + 
                     (unsigned char *)gdt_base);
        ldt_seg[1] = (unsigned long *)(4 + 
                     (unsigned char *)(unsigned long)ldt_seg[0]);

        /* Obtain base code address for LDT */
        ldt_base = ((*(ldt_seg[0]) >> 16) & 0xFFFF) | 
                    ((*(ldt_seg[1]) & 0xFF) << 16) |
                    (((*(ldt_seg[1]) >> 24) & 0xFF) << 24);

        /* Obtain code segment descriptor for System call on LDT */
        sys_seg[0] = (unsigned long *)(((selector >> 3) * 8) + 
                     (unsigned char *)ldt_base);
        sys_seg[1] = (unsigned long *)(4 + 
                     (unsigned char *)sys_seg[0]);
        base = (*(sys_seg[0]) >> 16) | ((*(sys_seg[1]) & 0xFF) << 16) |
               (((*(sys_seg[1]) >> 24) & 0xFF) << 24);

    } else {
        /* The selector locate on GDT */
        unsigned long *gdt_base;
        unsigned char __base[6];
        unsigned long *sys_seg[2];

        /*
         * Parse GDTR
         *   The GDTR register holds the base address (32 bits in protected 
         *   mode) and the 16-bit table limit for the GDT. The base address
         *   specifies the linear address of byte 0 of the GDT. The table
         *   limit specifies the number of bytes in the table.
         * 
         *   The LGDT and SGDT instruction load and store the GDTR register,
         *   respectively. On power up or reset of the processor, the base
         *   address is set to the default value of 0 and the limit is set
         *   to 0xffffh. A new base address must be loaded into the GDTR as
         *   part of the processor initialization process for protected-mode
         *   operation.
         */
        __asm__("sgdt %0"
                : "=m" (__base));
        gdt_base = (unsigned long *)(unsigned long)__base[2];

        /* Obtain segment descriptor from GDT */
        sys_seg[0] = (unsigned long *)(((selector >> 3) * 8) + 
                     (unsigned char *)gdt_base);
        sys_seg[1] = (unsigned long *)(4 + 
                     (unsigned char *)sys_seg[0]);
        base = (*(sys_seg[0]) >> 16) | ((*(sys_seg[1]) & 0xFF) << 16) |
               (((*(sys_seg[1]) >> 24) & 0xFF) << 24);
    }
    
    /* The address of system call on system */
    syscall_addr = base + offset;
    /* Diagnose address is correct */
    if (syscall_addr == (unsigned long)system_call) {
        printk("Obtain correct syscall address.\n");
        printk("System call address is %#x\n", system_call);
    } else {
        printk("Obtain incorrect syscall address.\n");
        printk("Correct address: %#x\n", system_call);
        printk("Un-detect address: %#x\n", syscall_addr);
    }
}

/* commom system call routine */
void system_call_rountine(void)
{

    if (1) {
        trigger_system_call();
    }
}
