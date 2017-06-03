#ifndef _TESTCODE_
#define _TESTCODE_

#ifdef CONFIG_TESTCODE

extern void simple_inline_assembly(void);
extern void test_get_eax(void);
extern void test_get_ebx(void);
extern void test_multi_output(void);
extern void test_set_eax(void);
extern void test_strcpy(void);
extern void test_strncpy(void);
extern void test_strcat(void);
extern void test_strncat(void);
extern void test_strcmp(void);
extern void test_strncmp(void);
extern void test_strchr(void);
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
#endif

#endif
