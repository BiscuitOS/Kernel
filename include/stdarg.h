#ifndef _STDARG_H_
#define _STDARG_H_

typedef char *va_list;

/*
 * Amount of spacerequired in an argument list for an arg of type TYPE.
 * TYPE may alternatively by an expression whose type is used.
 */
#define __va_rounded_size(TYPE)    \
	(((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#ifndef __sparc__
#define va_start(AP, LASTARG)                    \
	(AP = ((char *)&(LASTARG) + __va_rounded_size(LASTARG)))
#else
#define va_start(AP, LASTARG)                    \
	(__builtin_saverags(),                       \
	AP = ((char *)&(LASTARG) + __va_rounded_size(LASTARG)))
#endif

void va_end(va_list);		/* Defined in GUNlib */
#define va_end(AP)

#define va_arg(AP, TYPE)        \
	(AP += __va_rounded_size(TYPE),                   \
	*((TYPE *)(AP - __va_rounded_size(TYPE))))

#endif /* _STDARG_H_ */
