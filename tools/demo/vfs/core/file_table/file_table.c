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

#include <demo/debug.h>

static struct file *first_files;

#ifdef CONFIG_DEBUG_FILE_TABLE_INIT
static void file_table_inits(void)
{
    first_files = NULL;
    printk("** initialize file_table.\n");
}
#endif

static int debug_file_table(void)
{
#ifdef CONFIG_DEBUG_FILE_TABLE_INIT
    file_table_inits();
#endif
    return 0;
}
subsys_debugcall(debug_file_table);

static int debug_op(void)
{
    printf("Helllll\n");
    return 0;
}
user1_debugcall_sync(debug_op);
