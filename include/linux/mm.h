#ifndef _MM_H_
#define _MM_H_

#define PAGE_SIZE 4096

#define kfree(x) free_s((void *)(x), (int)0)

extern void free_page(unsigned long);
extern unsigned long get_free_page(void);
extern void *kmalloc(unsigned int len);
extern void free_s(void *obj, int size);
#endif
