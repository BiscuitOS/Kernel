/*
 * Obtain data from userland
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <test/debug.h>

#include <asm/segment.h>

/*
 * Copy data from userland.
 *
 *  While obtain data from userland on kernel, we should use
 *  get_fs_byte to obtain one byte from userland, such as:
 *    desc = get_fs_byte(src_addr)
 *  On example, 'desc' is a character value to hole data from userland and
 *  'src_addr' is a address of sourc character. So we should offer source
 *  address not source value.
 *  The same as get_fs_byte, if we want to obtain a pointer or a unsigned
 *  long value, we should offer address of source value, such as:
 *    desc = get_fs_long(src_addr)
 *  The 'desc' is a unsigned long value, and 'src_addr' is a address of 
 *  a unsigned long value.
 */
int obtain_data_from_userland(const char *file, char **argv)
{
    unsigned char ch;
    unsigned char *chp;
    unsigned long *point;

    /* Obtain a character from userland */
    ch = get_fs_byte(&file[0]);
    printk("Obtain character: %c\n", ch);
    ch = get_fs_byte(file);
    printk("Obtain character: %c\n", ch);
    
    /* Obtain next character from userland */
    ch = get_fs_byte(++file);
    printk("Obtain next character: %c\n", ch);
    file--;
    ch = get_fs_byte(&file[1]);
    printk("Obtain next character: %c\n", ch);

    /* Obtian 1st string from userland */
    chp = (unsigned char *)get_fs_long((unsigned long *)argv);
    printk("Obtain 1st string: %s\n", chp);

    /* Obtain 2nd string from userland */
    chp = (unsigned char *)get_fs_long((unsigned long *)++argv);
    printk("Obtain 2nd string: %s\n", chp);

    argv--;
    /* Obtain 1st pointer */
    point = (unsigned long *)get_fs_long((unsigned long *)&argv[0]);
    printk("Obtain 1st pointer: %s\n", (char *)point);

    /* Obtain 2nd pointer */
    point = (unsigned long *)get_fs_long((unsigned long *)&argv[1]);
    printk("Obtain 2nd pointer: %s\n", (char *)point);

    return 0;
}
