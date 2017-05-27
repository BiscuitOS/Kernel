#ifndef _KERNEL_H_
#define _KERNEL_H_


/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
		const typeof(((type *)0)->member) * __mptr = (ptr);	\
		(type *)((char *)__mptr - offsetof(type, member)); })

int printk(const char *, ...);
int tty_write(unsigned, char *, int);
void panic(const char *);
#endif
