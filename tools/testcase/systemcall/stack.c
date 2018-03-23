/*
 * Stack map on trigger system call
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
#include <linux/fs.h>

#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/*
 * Stack Map
 *
 * High address
 *   ---------------------------------------
 *   |   0x2C   |    Old ss                |
 *   ---------------------------------------
 *   |   0x28   |    Old esp               |
 *   ---------------------------------------
 *   |   0x24   |    eflags                |
 *   ---------------------------------------
 *   |   0x20   |    cS                    |
 *   ---------------------------------------
 *   |   0x1C   |    eip                   |
 *   ---------------------------------------
 *   |   0x18   |    ds                    |
 *   ---------------------------------------
 *   |   0x14   |    es                    |
 *   ---------------------------------------
 *   |   0x10   |    fs                    |
 *   ---------------------------------------
 *   |   0x0C   |    edx                   |
 *   ---------------------------------------
 *   |   0x08   |    ecx                   |
 *   ---------------------------------------
 *   |   0x04   |    ebx                   |
 *   ---------------------------------------
 *   |   0x00   |    eax                   |
 *   ---------------------------------------
 * Low address
 */
static int stack_map(unsigned long eax, unsigned long ebx,
                     unsigned long ecx, unsigned long edx,
                     unsigned long fs,  unsigned long es,
                     unsigned long ds,  unsigned long eip,
                     unsigned long cs,  unsigned long eflags,
                     unsigned long old_esp, unsigned long old_ss)
{
    printk("eax:    %#x\n", eax);
    printk("ebx:    %#x\n", ebx);
    printk("ecx:    %#x\n", ecx);
    printk("edx:    %#x\n", edx);
    printk("fs:     %#x\n", fs);
    printk("es:     %#x\n", es);
    printk("ds:     %#x\n", ds);
    printk("eip:    %#x\n", eip);
    printk("cs:     %#x\n", cs);
    printk("eflags: %#x\n", eflags);
    printk("oldesp: %#x\n", old_esp);
    printk("oldss:  %#x\n", old_ss);
    return 0;
} 

int sys_d_stack(unsigned int fildes)
{
    __asm__ volatile ("call stack_map\n\r"
                      "ret" ::);
    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_stack(void)
{
    d_stack(0);

    return 0;
}
