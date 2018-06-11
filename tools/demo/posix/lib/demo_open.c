/*
 * POSIX: open
 *
 * (C) 2018.06.11 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/unistd.h>

_syscall3(int, demo_open, const char *, file, int, flag, int, mode);
