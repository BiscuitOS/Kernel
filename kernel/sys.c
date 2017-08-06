/*
 * linux/kernel/sys.c
 *
 * (C) 1991 Linus Torvalds
 */

#include <linux/sched.h>

#include <asm/segment.h>

int sys_time(long *tloc)
{
    int i;

    i = CURRENT_TIME;
    if (tloc) {
        verify_area(tloc, 4);
        put_fs_long(i, (unsigned long *)tloc);
    }
    return i;
}
