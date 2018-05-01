#ifndef _ASM_IO_H
#define _ASM_IO_H
/*
 * Thanks to James van Artsdalen for a better timing-fix than
 * the two short jumps: using outb's to a nonexistent port seems
 * to guarantee better timings even on fast machines.
 *
 *		Linus
 */

static void inline outb(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1"
		::"a" ((char) value),"d" ((unsigned short) port));
}

static void inline outb_p(char value, unsigned short port)
{
__asm__ volatile ("outb %0,%1\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80"
		::"a" ((char) value),"d" ((unsigned short) port));
}

static unsigned char inline inb(unsigned short port)
{
	unsigned char _v;
__asm__ volatile ("inb %1,%0"
		:"=a" (_v):"d" ((unsigned short) port));
	return _v;
}

static inline unsigned char inb_p(unsigned short port)
{
	unsigned char _v;
__asm__ volatile ("inb %1,%0\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80\n\t"
		  "outb %0,$0x80"
		:"=a" (_v):"d" ((unsigned short) port));
	return _v;
}
#endif
