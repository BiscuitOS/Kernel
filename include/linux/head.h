#ifndef _HEAD_H_
#define _HEAD_H_

typedef struct desc_struct {
	unsigned long a, b;
} desc_table[256];

extern unsigned long pg_dir[1024];
extern desc_table idt, gdt;

#endif
