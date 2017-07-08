/*
 * linux/fs/supper.c
 * 
 * (C) 1991 Linus Torvalds
 */

/*
 * super.c contains code to handle the super-block tables.
 */
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/system.h>

/* this is initialized in init/main.c */
int ROOT_DEV = 0;
struct super_block super_block[NR_SUPER];

static void wait_on_super(struct super_block *sb)
{
    cli();
    while (sb->s_lock)
        sleep_on(&(sb->s_wait));
    sti();
}

struct super_block *get_super(int dev)
{
   struct super_block *s;

   if (!dev)
       return NULL;

   s = 0 + super_block;
   while (s < NR_SUPER + super_block)
       if (s->s_dev == dev) {
           wait_on_super(s);
           if (s->s_dev == dev)
               return s;
           s = 0 + super_block;
       } else
           s++;
    return NULL;
}
