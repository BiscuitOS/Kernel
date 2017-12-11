#ifndef _TASK_H
#define _TASK_H

extern int test_task_scheduler(void);

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
extern int parse_gdtr(unsigned short *limit, unsigned long *base);
extern struct seg_desc *segment_descriptors(unsigned long selector);
extern int segment_descriptor_type(struct seg_desc *desc);
extern int parse_stack_segment_descriptor(void);
extern int parse_code_segment_descriptor(void);
extern int segment_descriptor_dpl(unsigned long selector);
extern int segment_descriptor_cpl(void);
#endif

#ifdef CONFIG_TESTCASE_SEGMENT
extern void debug_segment_common(void);
extern void parse_segment_selector(unsigned long selector);
#endif

#ifdef CONFIG_TESTCASE_GATE
extern void debug_system_descriptor_common(void);
#endif

#endif
