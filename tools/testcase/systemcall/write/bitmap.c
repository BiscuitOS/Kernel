/*
 * bitmap.c
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>

#define clear_block(addr) \
__asm__ __volatile__ ("cld\n\t" \
    "rep\n\t"  \
    "stosl" \
    ::"a" (0),"c" (BLOCK_SIZE/4),"D"((long)(addr)))

#define set_bit(nr, addr) ({\
register int res; \
__asm__ __volatile__("btsl %2,%3\n\tsetb %%al": \
"=a" (res): "0" (0), "r" (nr), "m" (*(addr))); \
res;})

#define find_first_zero(addr) ({ \
int __res; \
__asm__ __volatile__ ("cld\n" \
                      "1:\tlodsl\n\t" \
                      "notl %%eax\n\t" \
                      "bsfl %%eax, %%edx\n\t" \
                      "je 2f\n\t" \
                      "addl %%edx, %%ecx\n\t" \
                      "jmp 3f\n"  \
                      "2:\taddl $32, %%ecx\n\t" \
                      "cmpl $8192, %%ecx\n\t" \
                      "jl 1b\n" \
                      "3:" \
                      :"=c" (__res):"c" (0), "S" (addr)); \
__res;})


int d_new_block(int dev)
{
    struct buffer_head *bh;
    struct super_block *sb;
    int i, j;

    if (!(sb = get_super(dev)))
        panic("trying to get new block from nonexistant device");
    j = 8192;
    for (i = 0; i < 8; i++)
        if ((bh = sb->s_zmap[i]))
            if ((j = find_first_zero(bh->b_data)) < 8192)
                break;
    if (i >= 8 || !bh || j >= 8192)
        return 0;
    if (set_bit(j, bh->b_data))
        panic("new_block: bit already set");
    bh->b_dirt = 1;
    j += i *8192 + sb->s_firstdatazone - 1;
    if (j >= sb->s_nzones)
        return 0;
    if (!(bh = getblk(dev, j)))
        panic("new_block: cannot get block");
    if (bh->b_count != 1)
        panic("new block: count is != 1");
    clear_block(bh->b_data);
    bh->b_uptodate = 1;
    bh->b_dirt = 1;
    brelse(bh);
    return j;
}
