/*
 * a.out format
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#define __LIBRARY__
#include <unistd.h>

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>

#include <test/debug.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <a.out.h>

static char *argv_rc[] = { "-/bin/sh", NULL };
static char *envp_rc[] = { "HOME=/usr/root", NULL };

/*
 * parse a.out header
 *
 *   The header varies somewhat from one version of Unix to another, but 
 *   the version in BSD Unix.
 *
 *   -----------------------------------------------------------------
 *   | Offset |   Name   | Describe                                  |
 *   -----------------------------------------------------------------
 *   | 0x00   | a_magic  | Use macros N_MAGIC                        |
 *   -----------------------------------------------------------------
 *   | 0x04   | a_text   | The length of text, in bytes              |
 *   -----------------------------------------------------------------
 *   | 0x08   | a_data   | The length of data, in bytes              |
 *   -----------------------------------------------------------------
 *   | 0x0C   | a_bss    | The length of uninitialized data area for |
 *   |        |          | file, in bytes                            |
 *   -----------------------------------------------------------------
 *   | 0x10   | a_syms   | The length of symble table data in file,  |
 *   |        |          | in bytes.                                 |
 *   -----------------------------------------------------------------
 *   | 0x14   | a_entry  | start address                             |
 *   -----------------------------------------------------------------
 *   | 0x18   | a_trsize | length of relocation info for text, in    |
 *   |        |          | bytes.                                    |
 *   -----------------------------------------------------------------
 *   | 0x1C   | a_drsize | length of relocation info fir data, in    |
 *   |        |          | bytes.                                    |
 *   -----------------------------------------------------------------
 *
 *   The magic number a_magic indicates what kind of executable file this is. 
 *   (Make this a footnote: Historically, the magic number on the original 
 *   PDP-11 was octal 407, which was a branch instruction that would jump 
 *   over the next seven words of the header to the beginning of the text 
 *   segment. That permitted a primitive form of position independent code. 
 *   A bootstrap loader could load the entire executable including the file 
 *   header to be loaded by into memory, usually at location zero, and then 
 *   jump to the beginning of the loaded file to start the program. Only a 
 *   few standalone programs ever used this ability, but the 407 magic number
 *   is still with us 25 years later.) Different magic numbers tell the 
 *   operating system program loader to load the file in to memory 
 *   differently; we discuss these variations below. The text and data segment
 *   sizes a_text and a_data are the sizes in bytes of the read-only code 
 *   and read-write data that follow the header. Since Unix automatically 
 *   initializes newly allocated memory to zero, any data with an initial 
 *   contents of zero or whose contents don't matter need not be present 
 *   in the a.out file. The uninitialized size a_bss says how much 
 *   uninitialized data (really zero-initialized) data logically follows 
 *   the data in the a.out file.
 *
 *   The a_entry field gives the starting address of the program, while 
 *   a_syms, a_trsize, and a_drsize say how much symbol table and relocation 
 *   information follow the data segment in the file. Programs that have been
 *   linked and are ready to run need no symbol nor relocation info, so these
 *   fields are zero in runnable files unless the linker has included symbols
 *   for the debugger. 
 */
static void parse_aout_header(struct exec ex)
{
    if (N_MAGIC(ex) == ZMAGIC)
        printk("load a.out format file.\n");
}

int sys_d_execve(const char *file, char **argv, char **envp)
{
    struct buffer_head *bh;
    struct m_inode *inode;
    struct exec ex;

    if (!(inode = namei(file)))
        return -ENOENT;
    if (!(bh = bread(inode->i_dev, inode->i_zone[0]))) {
        panic("Unable read data zone!");
        return -ENOENT;
    }
    ex = *((struct exec *)bh->b_data);
    parse_aout_header(ex);

    return 0;    
}

/* Invoke by system call: int $0x80 */
int debug_binary_aout_format(void)
{
    d_execve("/bin/sh", argv_rc, envp_rc);
    return 0;
}
