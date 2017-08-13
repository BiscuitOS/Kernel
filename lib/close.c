/*
 * linux/lib/close.c
 *
 * (C) 1991 Linux Torvalds
 */
#define __LIBRARY__
#include  <unistd.h>

_syscall1(int, close, int, fd)
