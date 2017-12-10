#ifndef _TASK_H
#define _TASK_H

extern int test_task_scheduler(void);

#ifdef CONFIG_TESTCASE_GDT
extern void debug_gdt_common(void);
#endif

#ifdef CONFIG_TESTCASE_SEGMENT
extern void debug_segment_common(void);
#endif

#endif
