#ifndef _KERNEL_H_
#define _KERNEL_H_

int printk(const char *, ...);
int tty_write(unsigned, char *, int);
void panic(const char *);
#endif
