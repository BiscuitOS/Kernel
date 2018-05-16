#ifndef _LINUX_SEGMENT_H
#define _LINUX_SEGMENT_H

/* Kernel Code Segment */
#ifdef CONFIG_KERNEL_CS
#define KERNEL_CS	CONFIG_KERNEL_CS
#else
#define KERNEL_CS	0x10
#endif

/* Kernel Data Segment */
#ifdef CONFIG_KERNEL_DS
#define KERNEL_DS	CONFIG_KERNEL_DS
#else
#define KERNEL_DS	0x18
#endif

/* Userland Code Segment */
#ifdef CONFIG_USER_CS
#define USER_CS		CONFIG_USER_CS
#else
#define USER_CS		0x23
#endif

/* Userland Data Segment */
#ifdef CONFIG_USER_DS
#define USER_DS		CONFIG_USER_DS
#else
#define USER_DS		0x2B
#endif

#endif
