#include <linux/kernel.h>
#include <linux/testcode.h>

#ifdef CONFIG_TESTCODE

/*
 * Test code entry
 */
void TestCode(void)
{
	//test_strcpy();
    //test_strncpy();
	//test_strcat();
	//test_strncat();
	//test_strcmp();
	//test_strncmp();
	test_strchr();
	//simple_inline_assembly();
	//test_get_eax();
	//test_get_ebx();
	//test_multi_output();
	//test_set_eax();
}
#endif
