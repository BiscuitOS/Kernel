
#ifndef __GNUC__
#error I want GCC!
#endif

#define extern
#define inline
#define static
#define __LIBRARY__
#include <string.h>

inline char *strcpy(char *dest, const char *src)
{
	__asm__("cld\n"
			"1:\tlodsb\n\t"
			"stosb\n\t"
			"testb %%al, %%al\n\t"
			"jne 1b"
			::"S" (src), "D" (dest));
	return dest;
}

inline char *strncpy(char *dest, const char *src, int count)
{
	__asm__("cld\n"
			"1:\tdecl %2\n\t"
			"js 2f\n\t"
			"lodsb\n\t"
			"stosb\n\t"
			"testb %%al, %%al\n\t"
			"jne 1b\n\t"
			"rep\n\t"
			"stosb\n"
			"2:"
			::"S" (src), "D" (dest), "c" (count));
	return dest;
}

inline char *strcat(char *dest, const char *src)
{
	__asm__("cld\n\t"
			"repne\n\t"
			"scasb\n\t"
			"decl %1\n"
			"1:\tlodsb\n\t"
			"stosb\n\t"
			"testb %%al, %%al\n\t"
			"jne 1b"
			:: "S" (src), "D" (dest), "a" (0), "c" (0xffffffff));
	return dest;
}

inline char *strncat(char *dest, const char *src, int count)
{
	__asm__("cld\n\t"
			"repne\n\t"
			"scasb\n\t"
			"decl %1\n\t"
			"movl %4, %3\n"
			"1:\tdecl %3\n\t"
			"js 2f\n\t"
			"lodsb\n\t"
			"stosb\n\t"
			"testb %%al, %%al\n\t"
			"jne 1b\n"
			"2: \txorl %2, %2\n\t"
			"stosb"
			:: "S" (src), "D" (dest), "a" (0), "c" (0xffffffff), "g" (count)
			);
	return dest;
}

inline int strcmp(const char *cs, const char *ct)
{
	register int __res;
	__asm__("cld\n"
			"1:\tlodsb\n\t"
			"scasb\n\t"
			"jne 2f\n\t"
			"testb %%al, %%al\n\t"
			"jne 1b\n\t"
			"xorl %%eax, %%eax\n\t"
			"jmp 3f\n"
			"2:\tmovl $1, %%eax\n\t"
			"jl 3f\n\t"
			"negl %%eax\n"
			"3:"
			:"=a" (__res) : "D" (cs), "S" (ct));
	return __res;
}

inline int strncmp(const char *cs, const char *ct, int count)
{
	register int __res;
	__asm__("cld\n"
			"1:\tdecl %3\n\t"
			"js 2f\n\t"
			"lodsb\n\t"
			"scasb\n\t"
			"jne 3f\n\t"
			"testb %%al, %%al\n\t"
			"jne 1b\n"
			"2:\txorl %%eax, %%eax\n\t"
			"jmp 4f\n"
			"3:\tmovl $1, %%eax\n\t"
			"jl 4f\n\t"
			"negl %%eax\n"
			"4:"
			:"=a" (__res):"D" (cs), "S" (ct), "c" (count));
	return __res;
}

inline char *strchr(const char *s, char c)
{
	register char *__res;
	__asm__("cld\n\t"
			"movb %%al, %%ah\n"
			"1:\tlodsb\n\t"
			"cmpb %%ah, %%al\n\t"
			"je 2f\n\t"
			"testb %%al, %%al\n\t"
			"jne 1b\n\t"
			"movl $1, %1\n"
			"2:\tmovl %1, %0\n\t"
			"decl %0"
			:"=a" (__res): "S" (s), "0" (c));
	return __res;
}

inline int strlen(const char *s)
{
	register int __res;

	__asm__("cld\n\t"
			"repne\n\t"
			"scasb\n\t"
			"notl %0\n\t"
			"decl %0"
			: "=c"(__res):"D"(s),
			"a"(0),"0" (0xffffffff));
	return __res;
}

inline void *memset(void *s, char c, int count)
{
	__asm__("cld\n\t"
			"rep\n\t"
			"stosb"
			: : "a"(c), "D"(s), "c"(count)
			);
	return s;
}
