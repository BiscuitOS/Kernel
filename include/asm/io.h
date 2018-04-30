#ifndef _ASM_IO_H
#define _ASM_IO_H

static inline void inline outb(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1"::"a" (value),"d" (port));
}

static inline void inline outb_p(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"
	::"a" (value),"d" (port));
}

static inline unsigned char inline inb(unsigned short port)
{
	unsigned char _v;
	__asm__ volatile ("inb %1,%0":"=a" (_v):"d" (port));
	return _v;
}

static inline unsigned char inb_p(unsigned short port)
{
	unsigned char _v;
	__asm__ volatile ("inb %1,%0\n"
		"\tjmp 1f\n"
		"1:\tjmp 1f\n"
		"1:":"=a" (_v):"d" ((unsigned short) port));
	return _v;
}
#endif
