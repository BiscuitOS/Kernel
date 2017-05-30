#include <linux/kernel.h>

#ifdef CONFIG_TESTCODE
extern void test_strcpy(void);

/*
 * Test code entry
 */
void TestCode(void)
{
	test_strcpy();
}
#endif
