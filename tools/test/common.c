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
	//test_strchr();
	//simple_inline_assembly();
	//test_get_eax();
	//test_get_ebx();
	//test_multi_output();
	//test_set_eax();
	//test_set_multi_register();
	//test_set_and_get_register();
	//test_interrupt0_divide_error();
	//test_interrupt1_debug();
	//test_interrupt3_int3();
	test_interrupt4_overflow();
	//test_call_c_in_assembly();
	//test_get_c_return_address();
}
#endif
