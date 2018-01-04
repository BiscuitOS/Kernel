#ifndef _TEST_INTERRUPT_H
#define _TEST_INTERRUPT_H

#ifdef CONFIG_DEBUG_INTERRUPT
extern int debug_interrupt_common(void);

#ifdef CONFIG_DEBUG_IDT
struct desc_node
{
    unsigned long a;
    unsigned long b;
};

struct gate_desc
{
    unsigned long offset;
    unsigned short sel;
    unsigned char dpl;
    unsigned char flag;
    unsigned char type;
};
extern void debug_idt_segment_desc_common(void);
#endif

#ifdef CONFIG_DEBUG_INT_USAGE
extern void interrupt_useage_common(void);
#endif

#ifdef CONFIG_DEBUG_INTERRUPT0
extern void common_interrupt0(void);

#ifdef CONFIG_DEBUG_DIVIDE_ZERO
extern void trigger_interrupt0(void);
#endif

#ifdef CONFIG_DEBUG_OVERFLOW_BIT
extern void trigger_interrupt0(void);
#endif

#ifdef CONFIG_DEBUG_SOFT_INT0
extern void trigger_interrupt0(void);
#endif

#endif

#endif // end of CONFIG_TESTCASE_INTERRUPT
#endif
