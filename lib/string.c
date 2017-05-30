
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
