/* 
 * main.c
 * Maintainer: Buddy <buddy.zhang@aliyun.com>
 *
 * Copyright (C) 2017 BiscuitOS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <time.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/tty.h>

#include <asm/io.h>


/*
 * This is set up by the setup-routine at boot-time
 */
#define EXT_MEM_K        (*(unsigned short *)0x90002)
#define DRIVE_INFO       (*(struct drive_info *)0x90080)
#define ORIG_ROOT_DEV    (*(unsigned short *)0x901FC)

/*
 * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
 * and this seems to work. I anybody has more info on the real-time
 * clock I'd be interested. Most of this was trial and error, and some
 * bios-listing reading. Urghh.
 */
#define CMOS_READ(addr)  ({   \
	outb_p(0x80 | addr, 0x70);   \
	inb_p(0x71);                 \
})

#define BCD_TO_BIN(val)  ((val)=((val) & 15) + ((val) >> 4) * 10)

struct drive_info {
	char dummy[32];
} drive_info;

const char *command_line = "loglevel=8 console=ttyS0,115200";
static long memory_end = 0;
static long buffer_memory_end = 0;
static long main_memory_start = 0;

extern void mem_init(long, long);
extern void blk_dev_init(void);
extern void chr_dev_init(void);
extern void hd_init(void);
extern void floppy_init(void);
extern long kernel_mktime(struct tm *);
extern long startup_time;

static void time_init(void)
{
	struct tm time;

	do {
		time.tm_sec   = CMOS_READ(0);
		time.tm_min   = CMOS_READ(2);
		time.tm_hour  = CMOS_READ(4);
		time.tm_mday  = CMOS_READ(7);
		time.tm_mon   = CMOS_READ(8);
		time.tm_year  = CMOS_READ(9);
	} while (time.tm_sec != CMOS_READ(0));

	BCD_TO_BIN(time.tm_sec);
	BCD_TO_BIN(time.tm_min);
	BCD_TO_BIN(time.tm_hour);
	BCD_TO_BIN(time.tm_mday);
	BCD_TO_BIN(time.tm_mon);
	BCD_TO_BIN(time.tm_year);
	time.tm_mon--;
	startup_time = kernel_mktime(&time);
}

/*
 * Detect memory from setup-routine.
 */
static void memory_detect(void)
{
	memory_end = (1 << 20) + (EXT_MEM_K << 10);
	memory_end &= 0xFFFFF000;
	
	/* Current version only support litter than 16Mb */
	if (memory_end > 16 * 1024 * 1024)
		memory_end = 16 * 1024 * 1024;
	if (memory_end > 12 * 1024 * 1024)
		buffer_memory_end = 4 * 1024 * 1024;
	else if (memory_end > 6 * 1024 * 1024)
		buffer_memory_end = 2 * 1024 * 1024;
	else
		buffer_memory_end = 1 * 1024 * 1024;

	main_memory_start = buffer_memory_end;
}

static void info_init(void)
{
	printk("Compressing kernel.....\n");
	printk("Booting BisuitOS on physical CPU 0x0\n");
	printk("Machine:Intel i386\n");
	printk("Kernel command line: %s\n", command_line);
	printk("Welcome to BisuitOS :-)\n");
}

void main(void)
{
	/* 
	 * Interrupts are still disabled. Do necessary setups, then
	 * enable them.
	 */
	ROOT_DEV = ORIG_ROOT_DEV;
	drive_info = DRIVE_INFO;
	memory_detect();
	mem_init(main_memory_start, memory_end);
	trap_init();
	blk_dev_init();
	chr_dev_init();
	tty_init();
	info_init();
	time_init();
	sched_init();
	buffer_init(buffer_memory_end);
	hd_init();
	floppy_init();
}
