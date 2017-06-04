#include <linux/kernel.h>

#ifdef CONFIG_TESTCODE
#include <linux/testcode.h>

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
	//test_interrupt2_nmi();
	//test_interrupt3_int3();
	//test_interrupt4_overflow();
	//test_interrupt5_bound();
	//test_interrupt6_invalid_op();
	//test_interrupt7_device_not_available();
	//test_interrupt8_double_fault();
	//test_interrupt9_coprocessor_segment_overrun();
	//test_interrupt10_invalid_TSS();
	//test_interrupt11_segment_not_present();
	//test_interrupt12_task_segment();
	//test_interrupt13_general_protection();
	//test_interrupt14_page_fault();
	//test_interrupt15_intel_reserved();
	//test_interrupt16_coprocessor_error();
	//test_interrupt17_47_Reserved();
	//test_interrupt39_parallel_interrupt();
	//test_interrupt45_irq13();
	//test_call_c_in_assembly();
	//test_get_c_return_address();
	//test_get_free_page();
	//test_free_page();
	//test_calc_mem();
	test_copy_page_table();
}
#endif
