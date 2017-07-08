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

/*
 * This is defined as a macro, but at some point this might become a
 * real subroutine that sets a flag if it returns true (to do
 * BSD-style accounting where the process is flagged if it uses root
 * privs). The implication of this is that you should do normal
 * permissions checks first, and check suser() last.
 */
#define suser() (current->euid == 0)


int printk(const char *, ...);
int tty_write(unsigned, char *, int);
void panic(const char *);
#endif
