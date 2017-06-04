#ifndef _TESTCODE_
#define _TESTCODE_

#ifdef CONFIG_TESTCODE

extern void simple_inline_assembly(void);
extern void test_get_eax(void);
extern void test_get_ebx(void);
extern void test_multi_output(void);
extern void test_set_eax(void);
/* C string library */
extern void test_strcpy(void);
extern void test_strncpy(void);
extern void test_strcat(void);
extern void test_strncat(void);
extern void test_strcmp(void);
extern void test_strncmp(void);
extern void test_strchr(void);
/* inline assembly */
extern void test_set_multi_register(void);
extern void test_set_and_get_register(void);
extern void test_call_c_in_assembly(void);
extern void test_get_c_return_address(void);
/* Interrupt table */
extern void test_interrupt0_divide_error(void);
extern void test_interrupt1_debug(void);
extern void test_interrupt2_nmi(void);
extern void test_interrupt3_int3(void);
extern void test_interrupt4_overflow(void);
extern void test_interrupt5_bound(void);
extern void test_interrupt6_invalid_op(void);
extern void test_interrupt7_device_not_available(void);
extern void test_interrupt8_double_fault(void);
extern void test_interrupt9_coprocessor_segment_overrun(void);
extern void test_interrupt10_invalid_TSS(void);
extern void test_interrupt11_segment_not_present(void);
extern void test_interrupt12_task_segment(void);
extern void test_interrupt13_general_protection(void);
extern void test_interrupt14_page_fault(void);
extern void test_interrupt15_intel_reserved(void);
extern void test_interrupt16_coprocessor_error(void);
extern void test_interrupt17_47_Reserved(void);
extern void test_interrupt39_parallel_interrupt(void);
extern void test_interrupt45_irq13(void);
/* Memory manage test */
extern void test_get_free_page(void);
extern void test_free_page(void);
extern void test_calc_mem(void);
extern void test_copy_page_table(void);
#endif

#endif
