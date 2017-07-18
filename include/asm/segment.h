#ifndef _ASM_SEGMENT_H_
#define _ASM_SEGMENT_H_

static inline unsigned char get_fs_byte(const char *addr)
{
    unsigned register char _v;

    __asm__("movb %%fs:%1, %0": "=r"(_v):"m"(*addr));
    return _v;
}

static inline void put_fs_byte(char val, char *addr)
{
    __asm__("movb %0, %%fs:%1" :: "r" (val), "m" (*addr));
}

static inline void put_fs_long(unsigned long val, unsigned long *addr)
{
    __asm__("movl %0, %%fs:%1" :: "r" (val), "m" (*addr));
}

#endif
