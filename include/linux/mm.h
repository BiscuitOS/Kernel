#ifndef _MM_H_
#define _MM_H_

#define PAGE_SIZE 4096

extern void free_page(unsigned long);
extern unsigned long get_free_page(void);
#endif
