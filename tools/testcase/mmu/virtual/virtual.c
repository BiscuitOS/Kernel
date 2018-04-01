/*
 * Virtual Address Mechanism on MMU
 *
 * (C) 2018.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <unistd.h>

#include <linux/kernel.h>
#include <test/debug.h>

#include <string.h>

static char *argv[] = { "-/bin/sh", "/usr/bin", NULL };
/*
 * system call for virtual address to physical address.
 */
int sys_d_mmu(const char *filename, char **argv, char *buffer)
{
    unsigned long fs;
    unsigned long base, limit;

    /* Obtain */
    __asm__ ("movl %%fs, %0" : "=r" (fs));
    /**/

    return 0;
}


/* common linear address userland entry */
void debug_virtual_address_common_userland(void)
{
    const char *tmp = "Hello BiscuitOS";
    char buffer[40];

    d_printf("String: %s\n", tmp);
    d_printf("Virtual Address: %#x\n", tmp);

    d_mmu(tmp, argv, buffer);
}

