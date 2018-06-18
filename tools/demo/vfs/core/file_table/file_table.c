/*
 * File table
 *
 * (C) 2018.06.07 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/page.h>
#include <linux/mm.h>

#include <demo/debug.h>

static __unused struct file *first_files;
static __unused int nr_file = 0;

#ifdef CONFIG_DEBUG_FILE_TABLE_INIT
static void file_table_inits(void)
{
    first_files = NULL;
    printk("** initialize file_table.\n");
}
#endif

#ifdef CONFIG_DEBUG_FILE_GET

/*
 * insert_file_free()
 *  The 'file_table' manage a file descriptor linked list. 'first_file' is
 *  header for 'file_table' that pointer first free file descrptor.
 *  'insert_file_free()' will insert new free file descriptor into head of
 *  list.
 *
 *                 +------------+  f_prev  +------------+
 *                 |            |<---------|-           |<------- ...
 *                 | file       |          | file       |
 *                 | descriptor |          | descriptor |
 *                 |           -|--------->|           -|-------> ...
 * first_file----->+------------+  f_next  +------------+
 *
 *
 */
static void insert_file_free(struct file *file)
{
    file->f_next = first_files;
    file->f_prev = first_files->f_prev;
    file->f_next->f_prev = file;
    file->f_prev->f_next = file;
    first_files = file;
}

/*
 * remove_file_free()
 *  Remove a file structure from free file structure list. As figure.
 *
 *                 +------------+  f_prev  +------------+
 *                 |            |<---------|-           |<------- ...
 *                 | file       |          | file       |
 *                 | descriptor |          | descriptor |
 *                 |           -|--------->|           -|-------> ...
 * first_file----->+------------+  f_next  +------------+
 *
 *                             (Remove)
 *                  f_prev  +------------+
 *         NULL  <----------|            |
 *                          | file       |
 *                          | descriptor |   f_next
 *                          |           -|----------> NULL
 *                          +------------+
 *
 */
static void remove_file_free(struct file *file)
{
    if (first_files == file)
        first_files = first_files->f_next;
    if (file->f_next)
        file->f_next->f_prev = file->f_prev;
    if (file->f_prev)
        file->f_prev->f_next = file->f_next;
    file->f_prev = file->f_next = NULL;
}

/*
 * put_last_free()
 *  When we otbain a free file descriptor from file_table, it will
 *  be removed from file_table. And then this file descriptor will
 *  be add into tail of file_table. So, the file_table manage all
 *  file descriptor, the header of file table is 'first_file' that
 *  points first free file descriptor or NULL. And file descirptor 
 *  will be insert into tail of file_table when it be allocated.
 *
 *      (Used)                  (Free)                  (Free)
 *  +------------+  f_prev  +------------+  f_prev  +------------+
 *  |            |<---------|-           |<---------|-           |<-- ..
 *  | file       |          | file       |          | file       |
 *  | descriptor |          | descriptor |          | descriptor |
 *  |           -|--------->|           -|--------->|           -|--> ..
 *  +------------+  f_next  +------------+  f_next  +------------+
 *                          A
 *                          |
 *          first_file------o
 *
 *
 */
static void put_last_free(struct file *file)
{
    remove_file_free(file);
    file->f_prev = first_files->f_prev;
    file->f_prev->f_next = file;
    file->f_next = first_files;
    file->f_next->f_prev = file;
}

/*
 * grow_file()
 *  Expand the 'file_table' structure. grow_file() will be trigger if
 *  first_file is null or 'nr_files' is litter than NR_FILE. The 'file_table'
 *  manage a free file descriptor structure list, we can obtain new free
 *  file descriptor from this list. A new free page will be allocate when
 *  'file_table' was expend, and new page will cut a lot of metadata.
 *  The new metadata will insert into "file_table" by 'insert_file_free()'.
 *
 *
 *  0-----------------+-----------------+-----+----------------+-----4k
 *  |                 |                 |     |                |      |
 *  | file descriptor | file descriptor | ... | file desciptor | hole |
 *  |                 |                 |     |                |      |
 *  +-----------------+-----------------+-----+----------------+------+
 *                                   |
 *                                   |
 *                                   |
 *                                   V
 *                           insert_file_free()
 *                                   |
 *                                   |
 *                                   V
 *
 *                 +------------+  f_prev  +------------+
 *                 |            |<---------|-           |<------- ...
 *                 | file       |          | file       |
 *                 | descriptor |          | descriptor |
 *                 |           -|--------->|           -|-------> ...
 * first_file----->+------------+  f_next  +------------+
 *
 *
 */
static void grow_file(void)
{
    struct file *file;
    int i;

    file = (struct file *)get_free_page(GFP_KERNEL);

    if (!file)
        return;

    nr_file += i = PAGE_SIZE / sizeof(struct file);

    if (!first_files) {
        file++;
        file->f_next = file->f_prev = first_files = file, i--;
    }
    for (; i; i--)
        insert_file_free(file++);
}

/*
 * get_empty_filps()
 *  Obtain a free file descriptor from 'file_table'. The 'file_table' is a
 *  typical single linked list and the 'first_file' points to first free
 *  file descriptor. At first, 'get_empty_filps' will search free file 
 *  descriptor structure on 'first_file', if it is empty, we expand 
 *  'file_table' and then search again. When we search a valid file
 *  descriptor, we will remove it from file table but we will insert it
 *  into tail of file_table. But some time, if nr_files is small than
 *  NR_FILE, we also inovke grow_file to add new free file descriptor into
 *  file_table. and search again.
 */
static struct file *get_empty_filps(void)
{
    int i;
    struct file *f;

    if (!first_files)
        grow_file();
repeat:
    for (f = first_files, i = 0; i < nr_file; i++, f = f->f_next)
        if (!f->f_count) {
            remove_file_free(f);
            memset(f, 0, sizeof(*f));
            put_last_free(f);
            f->f_count = 1;
            return f;
        }
    if (nr_file < NR_FILE) {
        grow_file();
        goto repeat;
    }
    return NULL;
}

#endif

static int debug_file_table(void)
{
    struct file *f __unused;

#ifdef CONFIG_DEBUG_FILE_TABLE_INIT
    file_table_inits();
#endif

#ifdef CONFIG_DEBUG_FILE_GET
    f = get_empty_filps();
    if (f)
        printk("File descriptor: %#08x\n", (unsigned int)f);
#endif

    return 0;
}
late_debugcall(debug_file_table);
