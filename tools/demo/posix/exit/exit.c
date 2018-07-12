/*
 * POSIX system call: exit
 *
 * (C) 2018.07.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/stat.h>
#include <linux/fs.h>

#include <demo/debug.h>

asmlinkage int sys_demo_exit(int exit_code)
{
    printk("EXIT_CODE %d\n", exit_code);
    return 0;
}

/* build a system call entry */
inline _syscall1(int, demo_exit, int, exit_code);

/* userland code */
static int debug_exit(void)
{
    demo_exit(1);
    return 0;
}
user1_debugcall_sync(debug_exit);
