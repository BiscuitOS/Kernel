#ifndef _TESTCASE_DEBUG_H_
#define _TESTCASE_DEBUG_H_

#ifdef CONFIG_TESTCASE_SCHED
#include <test/task.h>
#endif

#ifdef CONFIG_TESTCASE_IDT
#include <test/interrupt.h>
#endif

#ifdef CONFIG_TESTCASE_MMU
#include <test/mm.h>
#endif

extern void testcase_init(void);

#endif
