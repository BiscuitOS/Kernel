/*
 * Push data to userland
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

#include <asm/segment.h>

int copy_kernel_to_userland(char *userland)
{
    char *buffer = "Hello World!";
    int nr = 14;

    /* Put a character to userland */
    put_fs_byte(buffer[0], userland);

    while (nr--)
        put_fs_byte(buffer[13 - nr], userland++);

    return 0;
}
