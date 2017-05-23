/*
 * kernel/printk.c
 * Maintainer: Buddy <buddy.zhang@aliyun.com>
 *
 * Copyright (C) 2017 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdarg.h>

#include <linux/kernel.h>

static char buf[1024];

extern int vsprintf(char *buf, const char *fmt, va_list args);

int printk(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);

	__asm__("push %%fs\n\t"
			"push %%ds\n\t"
			"pop %%fs\n\t"
			"pushl %0\n\t"
			"pushl $buf\n\t"
			"pushl $0\n\t"
			"call tty_write\n\t"
			"addl $8, %%esp\n\t"
			"popl %0\n\t"
			"pop %%fs"
			::"r"(i)
			:"ax", "cx", "dx");
	return i;
}
