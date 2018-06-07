/*
 * System Call: brk
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

#include <asm/segment.h>

#include <sys/types.h>
#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


int sys_d_brk(unsigned long end_data_seg)
{
    if (end_data_seg >= current->end_code &&
        end_data_seg <  current->start_stack - 16384)
        current->brk = end_data_seg;
    return current->brk;
}

/* Invoke by system call: int $0x80 */
int debug_syscall_brk0(void)
{
    unsigned long brk;

    brk = d_brk(0x2000000);
    printf("BRK: %#x\n", brk);
    return 0;
}
