/*
 * biscuitos debug on userland
 *
 * (C) 2018.02 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>


int sys_biscuitos_debug(void)
{
    printk("GHGGGGG\n");
    return 0;
}
