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

static inline unsigned long get_fs_long(const unsigned long *addr)
{
    unsigned long _v;

    __asm__("movl %%fs:%1, %0":"=r" (_v):"m" (*addr));
    return _v;
}

/*
 * Someone who knows GNU asm better than I should double check the following.
 * It seens to work. but I don't know if I'm doing something subtly wrong.
 * --- TYT, 11/24/91
 * [ noting wrong here, Linus ]
 */

static inline unsigned long get_fs()
{
    unsigned short _v;

    __asm__("mov %%fs, %%ax":"=a" (_v):);
    return _v;
}

static inline unsigned long get_ds()
{
    unsigned short _v;

    __asm__("mov %%ds, %%ax":"=a" (_v):);
    return _v;
}

static inline void set_fs(unsigned long val)
{
    __asm__("mov %0, %%fs"::"a" ((unsigned short) val));
}

#endif
