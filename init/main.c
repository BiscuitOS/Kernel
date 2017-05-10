/* ----------------------------------------------------------
 * main.c
 * Maintainer: Buddy <buddy.zhang@aliyun.com>
 *
 * Copyright (C) 2017 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>

/*
 * This is set up by the setup-routine at boot-time
 */
#define EXT_MEM_K        (*(unsigned short *)0x90002)
#define DRIVE_INFO       (*(struct drive_info *)0x90080)
#define ORIG_ROOT_DEV    (*(unsigned short *)0x901FC)

struct drive_info {
	char dummy[32];
} drive_info;

static long memory_end = 0;
static long buffer_memory_end = 0;
static long main_memory_start = 0;

extern void mem_init(long, long);

void main(void)
{
	/* 
	 * Interrupts are still disabled. Do necessary setups, then
	 * enable them.
	 */
	ROOT_DEV = ORIG_ROOT_DEV;
	drive_info = DRIVE_INFO;
	memory_end = (1 << 20) + (EXT_MEM_K << 10);
	memory_end &= 0xfffff000;

	if (memory_end > 16 * 1024 * 1024)
		memory_end = 16 * 1024 * 1024;
	if (memory_end > 12 * 1024 * 1024)
		buffer_memory_end = 4 * 1024 * 1024;
	else if (memory_end > 6 * 1024 * 1024)
		buffer_memory_end = 2 * 1024 * 1024;
	else
		buffer_memory_end = 1 * 1024 * 1024;
	main_memory_start = buffer_memory_end;
	mem_init(main_memory_start, memory_end);
	trap_init();
}
