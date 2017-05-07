#include <linux/sched.h>
#include <linux/mm.h>

#define _S(nr)  (1 << ((nr)-1))
#define _BLOCKABLE (~(_S(SIGKILL) | _S(SIGSTOP)))

long user_stack[PAGE_SIZE >> 2];

struct {
	long *a;
	short b;
} stack_start = { &user_stack[PAGE_SIZE >> 2], 0x10};

union task_union {
	struct task_struct task;
	char stak[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK,};

long volatile jiffies = 0;
struct task_struct *current = &(init_task.task);
struct task_struct *last_task_used_math = NULL;

struct task_struct *task[NR_TASKS] = {&(init_task.task), };

/*
 * schedule() is the scheduler function. This is GOOD CODE!
 * There probably won't be any reason to change this, as it should 
 * work well in all circumstances (ie gives IO-bound processes good
 * response etc).
 * The one thing you might take a look at is the signal-handler
 * here.
 * NOTE! Task 0 is the 'idle' task, which gets called when no other
 * tasks can run. It can not be killed, and it cannot sleep.
 * The information in task[0] is never used.
 */
void schedule(void)
{
	int i, next, c;
	struct task_struct **p;

	/* 
	 * Check alarm, wake up any interruptible tasks that have
	 * got a signal.
	 */
	for (p = &LAST_TASK; p > &FIRST_TASK; --p) {
		if (*p) {
			if ((*p)->alarm && (*p)->alarm < jiffies) {
				(*p)->signal |= (1 << (SIGALRM - 1));
				(*p)->alarm = 0;
			}	
		if (((*p)->signal & ~(_BLOCKABLE & (*p)->blocked)) && 
			(*p)->state == TASK_INTERRUPTIBLE )
			(*p)->state = TASK_RUNNING;
		}	 
		
		/* This is the schedule proper: */
		while (1) {
			c = -1;
			next = 0;
			i = NR_TASKS;
			p = &task[NR_TASKS];
			
			while (--i) {
				if (!*--p)
					continue;
				if ((*p)->state == TASK_RUNNING && 
					(*p)->counter > c)
					c = (*p)->counter, next = i;
			}
			if (c)
				break;
			for (p = &LAST_TASK; p > &FIRST_TASK; --p)
				if (*p)
					(*p)->counter = ((*p)->counter >> 1) + 
							(*p)->priority;
		}
		switch_to(next);
	}
}

void interruptible_sleep_on(struct task_struct **p)
{
	struct task_struct *tmp;

	if (!p)
		return;
	
	if (current == &(init_task.task))
		panic("task[0] trying to sleep");
	tmp = *p;
	*p = current;
	current->state = TASK_UNINTERRUPTIBLE;
	schedule();
	if (tmp)
		tmp->state = 0;
}

void wake_up(struct task_struct **p)
{
	if (p && *p) {
		(**p).state = 0;
		*p = NULL;
	}	
}














































