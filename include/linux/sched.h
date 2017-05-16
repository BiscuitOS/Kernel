#ifndef _SCHED_H_
#define _SCHED_H_

#include <linux/head.h>
#include <linux/fs.h>
#include <signal.h>

#define NR_TASKS 64
#define HZ 100

#define FIRST_TASK task[0]
#define LAST_TASK  task[NR_TASKS - 1]

#define TASK_RUNNING         0
#define TASK_INTERRUPTIBLE   1
#define TASK_UNINTERRUPTIBLE 2
#define TASK_ZOMBIE          3
#define TASK_STOPPED         4

#ifndef NULL
#define NULL ((void *) 0)
#endif

struct i387_struct {
	long cwd;
	long swd;
	long twd;
	long fip;
	long fcs;
	long foo;
	long fos;
	long st_space[20];
};

struct tss_struct {
	long back_link;
	long esp0;
	long ss0;
	long esp1;
	long ss1;
	long esp2;
	long ss2;
	long cr3;
	long eip;
	long eflags;
	long eax, ecx, edx, ebx;
	long esp;
	long ebp;
	long esi;
	long edi;
	long es;
	long cs;
	long ss;
	long ds;
	long fs;
	long gs;
	long ldt;
	long trace_bitmap;
	struct i387_struct i387;
};

struct task_struct {
	/* these are hardcoded - don't touch */
	long state;      /* -1 unrunable, 0 runable, >0 stopped */
	long counter;
	long priority;
	long signal;
	struct sigaction sigaction[32];
	long blocked;
	int exit_code;
	unsigned long start_code, end_code, end_data, brk, start_stack;
	long pid, father, pgrp, session, leader;
	unsigned short uid, euid, suid;
	unsigned short gid, egid, sgid;
	long alarm;
	long utime, stime, cutime, cstime, start_time;
	unsigned short used_math;
	int tty;
	unsigned short umask;
	struct m_inode *pwd;
	struct m_inode *root;
	struct m_inode *executable;
	unsigned long close_on_exec;
	struct file *filp[NR_OPEN];
	struct desc_struct ldt[3];
	struct tss_struct tss;
};

/*
 * INIT_TASK is used to set up the first task table, touch at 
 * your own risk! Base=0, limit=0x9ffff (=640kB)
 */
#define INIT_TASK  \
 /* state etc */  { 0, 15, 15,      \
 /* signals */      0, { {},}, 0,   \
 /* ec,brk */       0, 0, 0, 0, 0, 0,  \
 /* pid etc */      0, -1, 0, 0, 0,    \
 /* uid etc */      0, 0, 0, 0, 0, 0,  \
 /* alarm */        0, 0, 0, 0, 0, 0,  \
 /* math */         0,                 \
 /* fs info */      -1, 0022, NULL, NULL, NULL, 0, \
 /* filp */         { NULL,},                      \
					{                              \
					{0, 0}, \
 /* ldt */          {0x9f, 0xc0fa00},              \
					},                             \
 /* tss */          {0, PAGE_SIZE + (long)&init_task, 0x10, 0, 0, \
					0, 0, (long)&pg_dir,           \
					0, 0, 0, 0, 0, 0, 0, 0, \
					0, 0, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,     \
					_LDT(0), 0x80000000,    \
					{}                      \
					}, \
}

/*
 * Entry into gdt where to find first TSS.
 * 0 - nul, 1 - cs, 2 - ds, 3 - syscall
 * 4 - TSS0, 5 - LDT0, 6 - TSS1 etc ...
 */
#define FIRST_TSS_ENTRY  4
#define FIRST_LDT_ENTRY  (FIRST_TSS_ENTRY + 1)
#define _TSS(n)  ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
#define _LDT(n)  ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))
#define ltr(n)   __asm__("ltr %%ax" :: "a" (_TSS(n)))
#define lldt(n)  __asm__("lldt %%ax" :: "a" (_LDT(n)))
#define str(n)   \
				 __asm__("str %%ax\n\t"   \
						 "subl %2, %%eax\n\t"    \
						 "shrl $4, %%eax"        \
						 :"=a" (n)   \
						 :"a" (0), "i" (FIRST_TSS_ENTRY<<3))

/*
 * switch_to(n) - should switch tasks to task nr n, first
 * checking that n isn't the current task, in which case it 
 * does nothing.
 * This also clears the TS-flag if the task we switched to 
 * has used tha math co-processor latest.
 */
#define switch_to(n) { \
	struct { long a, b;} __tmp;   \
	__asm__("cmp %%ecx, current\n\t" \
			"je 1f\n\t"    \
			"movw %%dx, %1\n\t"   \
			"xchgl %%ecx, current\n\t"   \
			"ljmp *%0\n\t"       \
			"cmpl %%ecx, last_task_used_math\n\t"   \
			"jne 1f\n\t"      \
			"clts\n"   \
			"1:"  \
			::"m"  (*&__tmp.a), "m" (*&__tmp.b),   \
			"d" (_TSS(n)), "c" ((long) task[n]));  \
}

static inline unsigned long _get_base(char *addr)
{
	unsigned long __base;

	__asm__ ("movb %3, %%dh\n\t"
			 "movb %2, %%dl\n\t"
			 "shll $16, %%edx\n\t"
			 "movw %1, %%dx"
			 : "=&d" (__base)
			 : "m" (*((addr) + 2)),
			   "m" (*((addr) + 4)),
			   "m" (*((addr) + 7)));
	return __base;
}

#define get_base(ldt) _get_base((char *)&(ldt))

#define get_limit(segment)  ({    \
	unsigned long __limit;        \
	__asm__ ("lsll %1, %0\n\tincl %0":"=r" (__limit):"r" (segment));  \
	__limit;})

extern struct task_struct *current;
extern struct task_struct *task[NR_TASKS];
extern struct task_struct *last_task_used_math;
extern struct task_struct *last_task_used_math;

extern void schedule(void);
extern int tty_write(unsigned minor, char *buf, int count);
extern void interruptible_sleep_on(struct task_struct **);
extern void wake_up(struct task_struct **);
extern void trap_init(void);
extern void sched_init(void);
extern void add_timer(long, void (*fn)(void));

#ifndef PANIC
void panic(const char *str);
#endif
#endif
