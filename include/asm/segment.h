#ifndef _ASM_SEGMENT_H_
#define _ASM_SEGMENT_H_

static inline unsigned char get_fs_byte(const char *addr)
{
	unsigned register char _v;

	__asm__("movb %%fs:%1, %0" : "=r" (_v): "m" (*addr));
	return _v;
}

#endif
