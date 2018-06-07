/*
 * System Call: rmdir
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <unistd.h>

_syscall2(int, mkdir, const char *, filename, int, mode)
_syscall1(int, d_rmdir, const char *, filename)
