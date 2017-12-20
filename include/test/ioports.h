#ifndef _DEBUG_IOPORTS_H
#define _DEBUG_IOPORTS_H

extern int test_common_ioports(void);

#ifdef CONFIG_TESTCASE_PORT_0X70
extern void debug_cmos_ram_common(void);
#endif

#endif
