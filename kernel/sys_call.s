/*
 *  linux/kernel/sys_call.S
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * sys_call.S  contains the system-call and fault low-level handling routines.
 * This also contains the timer-interrupt handler, as well as all interrupts
 * and faults that can result in a task-switch.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call.
 *
 * I changed all the .align's to 4 (16 byte alignment), as that's faster
 * on a 486.
 *
 * Stack layout in 'ret_from_system_call':
 * 	ptrace needs to have all regs on the stack.
 *	if the order here is changed, it needs to be 
 *	updated in fork.c:copy_process, signal.c:do_signal,
 *	ptrace.c and ptrace.h
 *
 *	 0(%esp) - %ebx
 *	 4(%esp) - %ecx
 *	 8(%esp) - %edx
 *       C(%esp) - %esi
 *	10(%esp) - %edi
 *	14(%esp) - %ebp
 *	18(%esp) - %eax
 *	1C(%esp) - %ds
 *	20(%esp) - %es
 *      24(%esp) - %fs
 *	28(%esp) - %gs
 *	2C(%esp) - orig_eax
 *	30(%esp) - %eip
 *	34(%esp) - %cs
 *	38(%esp) - %eflags
 *	3C(%esp) - %oldesp
 *	40(%esp) - %oldss
 */

EBX		= 0x00
ECX		= 0x04
EDX		= 0x08
ESI		= 0x0C
EDI		= 0x10
EBP		= 0x14
EAX		= 0x18
DS		= 0x1C
ES		= 0x20
FS		= 0x24
GS		= 0x28
ORIG_EAX	= 0x2C
EIP		= 0x30
CS		= 0x34
EFLAGS		= 0x38
OLDESP		= 0x3C
OLDSS		= 0x40

IF_MASK		= 0x00000200
NT_MASK		= 0x00004000
VM_MASK		= 0x00020000

/*
 * these are offsets into the task-struct.
 */
state		= 0
counter		= 4
priority	= 8
signal		= 12
sigaction	= 16		# MUST be 16 (=len of sigaction)
blocked		= (33*16)
saved_kernel_stack = ((33*16)+4)
kernel_stack_page = ((33*16)+8)
flags		= ((33*16)+12)

/*
 * offsets within sigaction
 */
sa_handler	= 0
sa_mask		= 4
sa_flags	= 8
sa_restorer	= 12

ENOSYS = 38

.globl system_call, sys_execve
.globl device_not_available, coprocessor_error
.globl divide_error, debug, nmi, int3, overflow, bounds, invalid_op
.globl double_fault, coprocessor_segment_overrun
.globl invalid_TSS, segment_not_present, stack_segment
.globl general_protection, reserved
.globl alignment_check, page_fault
.globl ret_from_sys_call

.align 4
bad_sys_call:
	movl $-ENOSYS,EAX(%esp)
	jmp ret_from_sys_call
.align 4
reschedule:
	pushl $ret_from_sys_call
	jmp schedule
.align 4
system_call:
	pushl %eax		# save orig_eax
        cld
        push %gs
        push %fs
        push %es
        push %ds
        pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx
        movl $0x10,%edx
        mov %dx,%ds
        mov %dx,%es
        movl $0x17,%edx
        mov %dx,%fs

	movl $-ENOSYS,EAX(%esp)
	cmpl NR_syscalls,%eax
	jae ret_from_sys_call
	movl current,%ebx
	testl $0x20,flags(%ebx)		# PF_TRACESYS
	je 1f
	pushl $0
	pushl %ebx
	pushl $5			# SIGTRAP
	call send_sig
	addl $12,%esp
	call schedule
	movl ORIG_EAX(%esp),%eax
1:	call *sys_call_table(,%eax,4)
	movl %eax,EAX(%esp)		# save the return value
	movl current,%eax
	testl $0x20,flags(%eax)		# PF_TRACESYS
	je ret_from_sys_call
	cmpl $0,signal(%eax)
	jne ret_from_sys_call		# ptrace would clear signal
	pushl $0
	pushl %eax
	pushl $5			# SIGTRAP
	call send_sig
	addl $12,%esp
	call schedule
	.align 4,0x90
ret_from_sys_call:
	movl EFLAGS(%esp),%eax		# check VM86 flag: CS/SS are
	testl $VM_MASK,%eax		# different then
	jne 4f
	cmpw $0x0f,CS(%esp)		# was old code segment supervisor ?
	jne 2f
	cmpw $0x17,OLDSS(%esp)		# was stack segment = 0x17 ?
	jne 2f
4:	orl $IF_MASK,%eax		# these just try to make sure
	andl $~NT_MASK,%eax		# the program doesn't do anything
	movl %eax,EFLAGS(%esp)		# stupid
1:	cmpl $0, need_resched
	jne reschedule
	movl current, %eax
	cmpl task,%eax			# task[0] cannot have signals
	je 2f
	cmpl $0,state(%eax)		# state
	jne reschedule
	cmpl $0,counter(%eax)		# counter
	je reschedule
	movl signal(%eax),%ebx
	movl blocked(%eax),%ecx
	notl %ecx
	andl %ebx,%ecx
	bsfl %ecx,%ecx
	je 2f
	btrl %ecx,%ebx
	incl %ecx
	movl %ebx,signal(%eax)
	movl %esp,%ebx
	testl $VM_MASK,EFLAGS(%esp)
	je 3f
	pushl %ebx
	pushl %ecx
	call save_v86_state
	popl %ecx
	movl %eax,%ebx
	movl %eax,%esp
3:	pushl %ebx
	pushl %ecx
	call do_signal
	popl %ecx
	popl %ebx
	testl %eax, %eax
	jne 1b			# see if we need to switch tasks, or do more signals
2:	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	pop %ds
	pop %es
	pop %fs
	pop %gs
	addl $4,%esp 		# skip the orig_eax
	iret

.align 4
sys_execve:
	lea (EIP+4)(%esp),%eax  # don't forget about the return address.
	pushl %eax
	call do_execve
	addl $4,%esp
	ret

.align 4
divide_error:
	pushl $0 		# no error code
	pushl $do_divide_error
.align 4,0x90
error_code:
	push %fs
	push %es
	push %ds
	pushl %eax
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	cld
	movl $-1, %eax
	xchgl %eax, ORIG_EAX(%esp)	# orig_eax (get the error code. )
	xorl %ebx,%ebx			# zero ebx
	mov %gs,%bx			# get the lower order bits of gs
	xchgl %ebx, GS(%esp)		# get the address and save gs.
	pushl %eax			# push the error code
	lea 52(%esp),%edx
	pushl %edx
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	movl $0x17,%edx
	mov %dx,%fs
	call *%ebx
	addl $8,%esp
	jmp ret_from_sys_call

.align 4
coprocessor_error:
	pushl $0
	pushl $do_coprocessor_error
	jmp error_code

.align 4
device_not_available:
	pushl $-1		# mark this as an int
        cld
        push %gs
        push %fs
        push %es
        push %ds
        pushl %eax
        pushl %ebp
        pushl %edi
        pushl %esi
        pushl %edx
        pushl %ecx
        pushl %ebx
        movl $0x10,%edx
        mov %dx,%ds
        mov %dx,%es
        movl $0x17,%edx
        mov %dx,%fs
	pushl $ret_from_sys_call
	clts				# clear TS so that we can use math
	movl %cr0,%eax
	testl $0x4,%eax			# EM (math emulation bit)
	je math_state_restore
	pushl $0		# temporary storage for ORIG_EIP
	#call math_emulate
	addl $4,%esp
	ret

.align 4
debug:
	pushl $0
	pushl $do_debug		# _do_debug
	jmp error_code

.align 4
nmi:
	pushl $0
	pushl $do_nmi
	jmp error_code

.align 4
int3:
	pushl $0
	pushl $do_int3
	jmp error_code

.align 4
overflow:
	pushl $0
	pushl $do_overflow
	jmp error_code

.align 4
bounds:
	pushl $0
	pushl $do_bounds
	jmp error_code

.align 4
invalid_op:
	pushl $0
	pushl $do_invalid_op
	jmp error_code

.align 4
coprocessor_segment_overrun:
	pushl $0
	pushl $do_coprocessor_segment_overrun
	jmp error_code

.align 4
reserved:
	pushl $0
	pushl $do_reserved
	jmp error_code

.align 4
double_fault:
	pushl $do_double_fault
	jmp error_code

.align 4
invalid_TSS:
	pushl $do_invalid_TSS
	jmp error_code

.align 4
segment_not_present:
	pushl $do_segment_not_present
	jmp error_code

.align 4
stack_segment:
	pushl $do_stack_segment
	jmp error_code

.align 4
general_protection:
	pushl $do_general_protection
	jmp error_code

.align 4
alignment_check:
	pushl $do_alignment_check
	jmp error_code

.align 4
page_fault:
	pushl $do_page_fault
	jmp error_code
