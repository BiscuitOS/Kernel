/*
 * System Call: parse_elf
 *
 * (C) 2018.04 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <linux/unistd.h>

_syscall3(int, d_parse_elf, const char *, file, char **, argv, char **, envp)
