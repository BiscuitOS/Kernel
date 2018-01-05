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

#if defined CONFIG_DEBUG_DIVIDE_ZERO || defined CONFIG_DEBUG_OVERFLOW_BIT || \
    defined CONFIG_DEBUG_SOFT_INT0
extern void trigger_interrupt0(void);
#endif

#endif // INTERRUPT 0

#ifdef CONFIG_DEBUG_INTERRUPT1
extern void common_interrupt1(void);

#if defined CONFIG_DEBUG_SOFT_INT1 || defined CONFIG_DEBUG_EFLAGS_TF
extern void trigger_interrupt1(void);
#endif

#endif // INTERRUPT 1

#ifdef CONFIG_DEBUG_INTERRUPT2
extern void common_interrupt2(void);

#ifdef CONFIG_DEBUG_SOFT_INT2
extern void trigger_interrupt2(void);
#endif

#endif // INTERRUPT 2

#ifdef CONFIG_DEBUG_INTERRUPT3
extern void common_interrupt3(void);

#if defined CONFIG_DEBUG_SOFT_INT3 || defined CONFIG_DEBUG_INT3_TF
extern void trigger_interrupt3(void);
#endif

#endif // INTERRUPT 3


#endif // end of CONFIG_TESTCASE_INTERRUPT
#endif
