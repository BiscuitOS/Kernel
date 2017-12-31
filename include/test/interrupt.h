#ifndef _TEST_INTERRUPT_H
#define _TEST_INTERRUPT_H

#ifdef CONFIG_TESTCASE_INTERRUPT
extern int interrupt_main(void);
extern int get_interrupt_status(void);

#ifdef CONFIG_TESTCASE_IDTNULL
extern void trigger_interrupt_null(void);
#endif

#ifdef CONFIG_TESTCASE_IDT0
extern void trigger_interrupt0(void);
#endif // end of CONFIG_TESTCASE_IDT0

#ifdef CONFIG_TESTCASE_IDT1
extern void trigger_interrupt1(void);
#endif // end of CONFIG_TESTCASE_IDT1

#ifdef CONFIG_TESTCASE_IDT2
extern void trigger_interrupt2(void);
#endif

#ifdef CONFIG_TESTCASE_IDT3
extern void trigger_interrupt3(void);
#endif

#ifdef CONFIG_TESTCASE_IDT4
extern void trigger_interrupt4(void);
#endif

#ifdef CONFIG_TESTCASE_IDT5
extern void trigger_interrupt5(void);
#endif

#ifdef CONFIG_TESTCASE_IDT6
extern void trigger_interrupt6(void);
#endif

#ifdef CONFIG_TESTCASE_IDT7
extern void trigger_interrupt7(void);
#endif

#ifdef CONFIG_TESTCASE_IDT8
extern void trigger_interrupt8(void);
#endif

#ifdef CONFIG_TESTCASE_IDT9
extern void trigger_interrupt9(void);
#endif

#ifdef CONFIG_TESTCASE_IDT10
extern void trigger_interrupt10(void);
#endif

#ifdef CONFIG_TESTCASE_IDT11
extern void trigger_interrupt11(void);
#endif

#ifdef CONFIG_TESTCASE_IDT12
extern void trigger_interrupt12(void);
#endif

#ifdef CONFIG_TESTCASE_IDT13
extern void trigger_interrupt13(void);
#endif

#ifdef CONFIG_TESTCASE_IDT14
extern void trigger_interrupt14(void);
#endif

#ifdef CONFIG_TESTCASE_IDT15
extern void trigger_interrupt15(void);
#endif

#ifdef CONFIG_TESTCASE_IDT16
extern void trigger_interrupt16(void);
#endif

#ifdef CONFIG_TESTCASE_IDT17
extern void trigger_interrupt17(void);
#endif

#ifdef CONFIG_TESTCASE_IDT32
extern void trigger_interrupt32(void);
#endif

#ifdef CONFIG_TESTCASE_IDT39
extern void trigger_interrupt39(void);
#endif

#ifdef CONFIG_TESTCASE_IDT45
extern void trigger_interrupt45(void);
#endif

#ifdef CONFIG_TESTCASE_IDT46
extern void trigger_interrupt46(void);
#endif

#ifdef CONFIG_TESTCASE_IDT128
extern void trigger_interrupt128(void);
#endif

#ifdef CONFIG_TESTCASE_STACK_SWITCH
extern void common_stack_switch(void);
#endif

#endif // end of CONFIG_TESTCASE_INTERRUPT
#endif
