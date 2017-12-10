#ifndef _TASK_H
#define _TASK_H

extern int test_task_scheduler(void);

#ifdef CONFIG_TESTCASE_GDT
extern void debug_gdt_common(void);
#endif

#endif
