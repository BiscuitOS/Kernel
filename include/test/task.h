#ifndef _TASK_H
#define _TASK_H

#ifdef CONFIG_TESTCASE_SCHED
extern int test_task_scheduler(void);
#if defined (CONFIG_DEBUG_USERLAND_EARLY) || \
    defined (CONFIG_DEBUG_USERLAND_SHELL)
extern void debug_scheduler_kernel_on_userland(void);
#endif

#ifdef CONFIG_TESTCASE_GDT
struct gdt_node
{
    unsigned long a;
    unsigned long b;
};

struct seg_desc
{
    unsigned long limit; /* optional alignment */
    unsigned long base;
    unsigned char type;
    unsigned char dpl;
    unsigned char flag;
};

extern void debug_gdt_common(void);
extern struct seg_desc *segment_descriptors(unsigned short selector);
extern int segment_descriptor_type(struct seg_desc *desc);
extern int segment_descriptor_dpl(unsigned short selector);
extern int segment_descriptor_cpl(void);
extern int segment_descriptor_rpl(unsigned short selector);
#endif

#ifdef CONFIG_TESTCASE_SEGMENT
extern void debug_segment_common(void);
#endif

#ifdef CONFIG_TESTCASE_GATE
extern void debug_system_descriptor_common(void);
#endif

#ifdef CONFIG_TESTCASE_IDT
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

#ifdef CONFIG_TESTCASE_STACK
extern void debug_stack_common(void);
#endif

#ifdef CONFIG_TESTCASE_MULT_PRIVILEGE
extern void debug_stack_kernel_on_userland(void);
#endif

#endif
#endif
