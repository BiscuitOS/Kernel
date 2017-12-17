#ifndef _DEBUG_TIMER
#define _DEBUG_TIMER

#ifdef CONFIG_TESTCASE_TIMER
extern int test_common_timer(void);
#endif

#ifdef CONFIG_TESTCASE_CMOS_CLK
extern void debug_cmos_clk_common(void);
#endif

#ifdef CONFIG_TESTCASE_8253
extern void debug_8253_common();
#endif

#endif
