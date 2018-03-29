/*
 * FS: exchange data between kernel and userland
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

static char *argv_rc[] = { "-/bin/sh", "/usr/bin/bash", NULL };

int sys_d_fs(const char *file, char **argv, char *buffer)
{
#ifdef CONFIG_DEBUG_SEGMENT_OBTAIN_USERLAND
    obtain_data_from_userland(file, argv);
#endif

#ifdef CONFIG_DEBUG_SEGMENT_PUSH_KERNEL
    copy_kernel_to_userland(buffer);
#endif

    return 0;
}

/* Invoke by system call: int $0x80 */
int debug_binary_aout_exec(void)
{
    char buffer[40];

    d_fs("/bin/sh", argv_rc, buffer);

    d_printf("Data: %c\n", buffer[0]);
    d_printf("String: %s\n", buffer);

    return 0;
}
