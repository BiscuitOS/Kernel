/*
 * linux/lib/_exit.h
 *
 * (C) 1991 Linus Torvalds
 */

#define __LIBRARY__
#include <unistd.h>

void _exit(int exit_code)
{
    __asm__ ("movl %1, %%ebx\n\t"
             "int $0x80" :: "a" (__NR_exit), "g" (exit_code));
}
