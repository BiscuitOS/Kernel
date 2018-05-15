/*
 * disk null function
 *
 * (C) 2017 buddy <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef CONFIG_HARDDISK
void do_hd_request(void)
{
}

void unexpected_hd_interrupt(void)
{
}

int sys_setup(void *BIOS)
{
    return 0;
}

void do_hd(void)
{
};
#endif

#ifndef CONFIG_FLOPPY
unsigned char selected = 0;

void (*do_floppy) (void);

int floppy_change(unsigned int nr)
{
    return 0;
}

void unexpected_floppy_interrupt(void)
{
}
#endif
