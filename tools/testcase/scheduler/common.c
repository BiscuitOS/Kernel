/*
 * Test Common task scheduler
 *
 * (C) 2017.12 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>

#include <test/task.h>

/*
 * Get current task information
 */
static int dump_task_information(void)
{
    struct task_struct *task = current;

    printk("counter %#x\n", task->counter);

    return 0;
}

/* common task interface */
int test_task_scheduler(void)
{
    printk("Test common task scheduler.\n");

    dump_task_information();

    return 0;
}

