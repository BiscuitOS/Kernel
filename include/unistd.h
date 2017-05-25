#ifndef _UNISTD_H_
#define _UNISTD_H_

#ifdef __LIBRARY__
#define __NR_fork  2

#define _syscall0(type, name) \
	type name(void) \
{     \
	long __res;    \
	__asm__ volatile ("int $0x80"    \
	: "=a" (__res)               \
	: "0"  (__NR_##name));       \
	if (__res >= 0)              \
		return (type) __res;     \
	errno = -__res;              \
	return -1;                   \
}

#endif /* __LIBRARY__ */

extern int errno;
static int fork(void);

#endif
