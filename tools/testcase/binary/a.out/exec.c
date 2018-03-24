/*
 * exec.c
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
#include <linux/mm.h>

#include <sys/stat.h>

#include <asm/segment.h>

#include <errno.h>
#include <a.out.h>

/*
 * MAX_ARG_PAGES defines the number of pages allocated for arguments
 * and envelope for the new program. 32 should suffice, this gives
 * a maximum env+arg of 128kB!
 */
#define MAX_ARG_PAGES 32

extern int sys_close(int fd);

/*
 * count() counts the number of arguments/envelops.
 */
static int count(char **argv)
{
    int i = 0;
    char **tmp;

    if ((tmp = argv))
        while (get_fs_long((unsigned long *)(tmp++)))
            i++;
    return i;
}

/*
 * 'copy_string()' copies argument/envelope strings from user
 * memory to free pages in kernel mem. These are in a format ready
 * to be put directly into the top of new user memory.
 *
 * Modified by TYT, 11/24/91 to add the from_kmem argument, which specifies
 * whether the string and the string array are from user or kernel segment:
 *
 * from_kmem   argv *         argv **
 *   0         user space     user space
 *   1         kernel space   user space
 *   2         kernel space   kernel space
 *
 * We do this by playing games with the fs segment register. Since it
 * is expensive to load a segment register, we try to avoid calling
 * set_fs() unless wew absolutely have to.
 */
static unsigned long copy_strings(int argc, char **argv, unsigned long *page,
                     unsigned long p, int from_kmem)
{
    unsigned long old_fs, new_fs;
    char *tmp, *pag = NULL;
    int len, offset = 0;

    if (!p)
        return 0;
    new_fs = get_ds();
    old_fs = get_fs();
    if (from_kmem == 2)
        set_fs(new_fs);
    while (argc-- > 0) {
        if (from_kmem == 1)
            set_fs(new_fs);
        if (!(tmp = (char *)get_fs_long(((unsigned long *)argv)+argc)))
            panic("argc is wrong");
        if (from_kmem == 1)
            set_fs(old_fs);
        len = 0; /* remember zero-padding */
        do {
            len++;
        } while (get_fs_byte(tmp++));
        if (p - len < 0) {
            set_fs(old_fs); /* this should't happen - 128kB */
            return 0;
        }
        while (len) {
            --p;
            --tmp;
            --len;
            if (--offset < 0) {
                offset = p % PAGE_SIZE;
                if (from_kmem == 2)
                    set_fs(old_fs);
                if (!(pag = (char *)page[p / PAGE_SIZE]) &&
                    !(pag = (char *)(page[p / PAGE_SIZE] = 
                            get_free_page())))
                    return 0;
                if (from_kmem == 2)
                    set_fs(new_fs);
            }
            *(pag + offset) = get_fs_byte(tmp);
        }
    }
    if (from_kmem == 2)
        set_fs(old_fs);
    return p;
}

static unsigned long change_ldt(unsigned long text_size, unsigned long *page)
{
    unsigned long code_limit, data_limit, code_base, data_base;
    int i;

    code_limit = text_size + PAGE_SIZE - 1;
    code_limit &= 0xFFFFF000;
    data_limit = 0x4000000;
    code_base = get_base(current->ldt[1]);
    data_base = code_base;
    set_base(current->ldt[1], code_base);
    set_limit(current->ldt[1], code_limit);
    set_base(current->ldt[2], data_base);
    set_limit(current->ldt[2], data_limit);
    /* make sure fs points to the NEW data segment */
    __asm__("pushl $0x17\n\tpop %%fs"::);
    data_base += data_limit;
    for (i = MAX_ARG_PAGES - 1; i >= 0; i--) {
        data_base -= PAGE_SIZE;
        if (page[i])
            put_page(page[i], data_base);
    }
    return data_limit;
}

/*
 * create_tables() parses the env- and arg-strings in new user
 * memory and creates the pointer tables from them, and puts their
 * addresses on the 'stack', returning the new stack pointer value.
 */
static unsigned long *create_tables(char *p, int argc, int envc)
{
    unsigned long *argv, *envp;
    unsigned long *sp;

    sp = (unsigned long *)(0xfffffffc & (unsigned long)p);
    sp -= envc + 1;
    envp = sp;
}

int d_do_execve(unsigned long *eip, long tmp, char *filename,
              char **argv, char **envp)
{
    int i, argc, envc;
    unsigned long page[MAX_ARG_PAGES];
    struct m_inode *inode;
    int retval, sh_bang = 0;
    int e_uid, e_gid;
    struct buffer_head *bh;
    struct exec ex;
    unsigned long p = PAGE_SIZE * MAX_ARG_PAGES - 4;

    /* eip[1] is cs that segment selector for code segment. d_do_execve()
     * only executes by user mode. */
    if ((0xffff & eip[1]) != 0x000f)
        panic("exec called from superisor mode");
    for (i = 0; i < MAX_ARG_PAGES; i++)
        page[i] = 0;
    if (!(inode = namei(filename)))
        return -ENOENT;
    argc = count(argv);
    envc = count(envp);
    
    if (!S_ISREG(inode->i_mode)) { /* Must be regular file */
        retval = -EACCES;
        goto exec_error2;
    }
    i = inode->i_mode;
    e_uid = (i & S_ISUID) ? inode->i_uid : current->euid;
    e_gid = (i & S_ISGID) ? inode->i_gid : current->egid;
    if (current->euid == inode->i_uid)
        i >>= 6;
    else
        i >>= 3;
    if (!(i & 1) &&
        !((inode->i_mode & 0111) && suser())) {
        retval = -ENOEXEC;
        goto exec_error2;
    }
    /* Read a.out first block from hd */
    if (!(bh = bread(inode->i_dev, inode->i_zone[0]))) {
        retval = -ENOEXEC;
        goto exec_error2;
    }
    /* obtain a.out header  */
    ex = *((struct exec *)bh->b_data);
    if ((bh->b_data[0] == "#") && (bh->b_data[1] == "!") && (!sh_bang)) {
        /* Shell scripts */;
    }
    brelse(bh);
    /* verify a.out format */
    if (N_MAGIC(ex) != ZMAGIC || ex.a_trsize || ex.a_drsize ||
        ex.a_text + ex.a_data + ex.a_bss > 0x3000000 ||
        inode->i_size < ex.a_text + ex.a_data + ex.a_syms +
        N_TXTOFF(ex)) {
        retval = -ENOEXEC;
        goto exec_error2;
    }
    if (N_TXTOFF(ex) != BLOCK_SIZE) {
        printk("%s: N_TXTOFF != BLOCK_SIZE. See a.out.h", filename);
        retval = -ENOEXEC;
        goto exec_error2;
    }
    if (!sh_bang) {
        p = copy_strings(envc, envp, page, p, 0);
        p = copy_strings(argc, argv, page, p, 0);
        if (!p) {
            retval = -ENOMEM;
            goto exec_error2;
        }
    }
    /* OK, This is the point of no return */
    if (current->executable)
        iput(current->executable);
    current->executable = inode;
    for (i = 0; i < 32; i++)
        current->sigaction[i].sa_handler = NULL;
    for (i = 0; i < NR_OPEN; i++)
        if ((current->close_on_exec >> i) & 1)
            sys_close(i);
    current->close_on_exec = 0;
    free_page_tables(get_base(current->ldt[1]), get_limit(0x0f));
    free_page_tables(get_base(current->ldt[2]), get_limit(0x17));
    if (last_task_used_math == current)
        last_task_used_math = NULL;
    current->used_math = 0;
    p += change_ldt(ex.a_text, page) - MAX_ARG_PAGES * PAGE_SIZE;
    p = (unsigned long)create_tables((char *)p, argc, envc);

exec_error2:
    iput(inode);

    return retval;
}
