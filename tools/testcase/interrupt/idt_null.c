/*
 * interrupt null: empty interrupt handle
 *
 * (C) 2017.10 <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

/*
 * empty interrupt handler
 */
void trigger_interrupt_null(void)
{
    printk("Test interrupt null\n");
}
