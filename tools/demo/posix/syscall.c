/*
 * POSIX system mechanism
 *
 * (C) 2018.07.11 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/unistd.h>

#include <demo/debug.h>

/* This may be used only once, enforced by 'static int callable' */
asmlinkage int sys_demo_syscall(const char *filename, int flags, int mode)
{
    return 0;
}

/* System call entry */
inline _syscall3(int, demo_syscall, const char *, file, int, flag, int, mode);

static int debug_syscall(void)
{
    return 0;
}
user1_debugcall_sync(debug_syscall);
