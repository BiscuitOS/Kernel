/*
 * Common System Call
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 */
#include <linux/kernel.h>

#include <test/debug.h>

int common_system_call_entry(void)
{
    printk("Hello World\n");
    return 0;
}
