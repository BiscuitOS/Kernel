/*
 *  linux/kernel/sys_call.S
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * sys_call.S  contains the system-call and fault low-level handling routines.
 * This also contains the timer-interrupt handler, as well as all interrupts
 * and faults that can result in a task-switch.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call.
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

SIG_CHLD	= 17

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

/*
 * these are offsets into the task-struct.
 */
state		= 0
counter		= 4
priority	= 8
signal		= 12
sigaction	= 16		# MUST be 16 (=len of sigaction)
blocked		= (33*16)

/*
 * offsets within sigaction
 */
sa_handler	= 0
sa_mask		= 4
sa_flags	= 8
sa_restorer	= 12

ENOSYS = 38

/*
 * Ok, I get parallel printer interrupts while using the floppy for some
 * strange reason. Urgel. Now I just ignore them.
 */
.globl system_call, timer_interrupt, sys_execve
.globl device_not_available, coprocessor_error
.globl divide_error, debug, nmi, int3, overflow, bounds, invalid_op
.globl double_fault, coprocessor_segment_overrun
.globl invalid_TSS, segment_not_present, stack_segment
.globl general_protection, irq13, reserved
.globl alignment_check, page_fault
.globl keyboard_interrupt, hd_interrupt
.globl IRQ3_interrupt, IRQ4_interrupt

.align 2
bad_sys_call:
	movl $-ENOSYS,EAX(%esp)
	jmp ret_from_sys_call
.align 2
reschedule:
	pushl $ret_from_sys_call
	jmp schedule
.align 2
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

	cmpl NR_syscalls,%eax
	jae bad_sys_call
	call *sys_call_table(,%eax,4)
	movl %eax,EAX(%esp)		# save the return value
ret_from_sys_call:
	cmpw $0x0f,CS(%esp)		# was old code segment supervisor ?
	jne 2f
	cmpw $0x17,OLDSS(%esp)		# was stack segment = 0x17 ?
	jne 2f
1:	movl current,%eax
	cmpl task,%eax			# task[0] cannot have signals
	je 2f
	cmpl $0,need_resched
	jne reschedule
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
	movl %ebx,signal(%eax)
	incl %ecx
	pushl %ecx
	call do_signal
	popl %ecx
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

.align 2
irq13:
	pushl %eax
	xorb %al,%al
	outb %al,$0xF0
	movb $0x20,%al
	outb %al,$0x20
	jmp 1f
1:	jmp 1f
1:	outb %al,$0xA0
	popl %eax
coprocessor_error:
	pushl $-1		# mark this as an int. 
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
	jmp math_error

.align 2
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
	call math_emulate
	addl $4,%esp
	ret

.align 2
keyboard_interrupt:
	pushl $-1
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
        inb $0x21,%al
        jmp 1f
1:      jmp 1f
1:      orb $0x02,%al
        outb %al,$0x21
        jmp 1f
1:      jmp 1f
1:      movb $0x20,%al
        outb %al,$0x20
	sti
	call do_keyboard
	cli
        inb $0x21,%al
        jmp 1f
1:      jmp 1f
1:      andb $~(0x02),%al
        outb %al,$0x21
	jmp ret_from_sys_call

.align 2
IRQ3_interrupt:
	pushl $-1
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
        inb $0x21,%al
        jmp 1f
1:      jmp 1f
1:      orb $0x08,%al
        outb %al,$0x21
        jmp 1f
1:      jmp 1f
1:      movb $0x20,%al
        outb %al,$0x20
	sti
	call do_IRQ3
	cli
        inb $0x21,%al
        jmp 1f
1:      jmp 1f
1:      andb $~(0x08),%al
        outb %al,$0x21
	jmp ret_from_sys_call

.align 2
IRQ4_interrupt:
	pushl $-1
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
        inb $0x21,%al
        jmp 1f
1:      jmp 1f
1:      orb $0x10,%al
        outb %al,$0x21
        jmp 1f
1:      jmp 1f
1:      movb $0x20,%al
        outb %al,$0x20
	sti
	call do_IRQ4
	cli
        inb $0x21,%al
        jmp 1f
1:      jmp 1f
1:      andb $~(0x10),%al
        outb %al,$0x21
	jmp ret_from_sys_call

.align 2
timer_interrupt:
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
        inb $0x21,%al
        jmp 1f
1:      jmp 1f
1:      orb $0x01,%al
        outb %al,$0x21
        jmp 1f
1:      jmp 1f
1:      movb $0x20,%al
        outb %al,$0x20
	sti
	incl jiffies
	movl CS(%esp),%eax
	andl $3,%eax		# %eax is CPL (0 or 3, 0=supervisor)
	pushl %eax
	call do_timer		# 'do_timer(long CPL)' does everything from
	addl $4,%esp		# task switching to accounting ...
	cli
        inb $0x21,%al
        jmp 1f
1:      jmp 1f
1:      andb $~(0x01),%al
        outb %al,$0x21
	jmp ret_from_sys_call

.align 2
hd_interrupt:
	pushl $-1
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
        inb $0xA1,%al
        jmp 1f
1:      jmp 1f
1:      orb $0x40,%al
        outb %al,$0xA1
        jmp 1f
1:      jmp 1f
1:      movb $0x20,%al
        outb %al,$0xA0
        jmp 1f
1:      jmp 1f
1:      outb %al,$0x20
	andl $0xfffeffff, timer_active
	xorl %edx,%edx
	xchgl do_hd,%edx
	testl %edx,%edx
	jne 1f
	movl $unexpected_hd_interrupt,%edx
1:	call *%edx		# "interesting" way of handling intr.
	cli
        inb $0xA1,%al
        jmp 1f
1:      jmp 1f
1:      andb $~(0x40),%al
        outb %al,$0xA1
	jmp ret_from_sys_call

.align 2
sys_execve:
	lea (EIP+4)(%esp),%eax  # don't forget about the return address.
	pushl %eax
	call do_execve
	addl $4,%esp
	ret

divide_error:
	pushl $0 		# no error code
	pushl $do_divide_error
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

debug:
	pushl $0
	pushl $do_int3		# _do_debug
	jmp error_code

nmi:
	pushl $0
	pushl $do_nmi
	jmp error_code

int3:
	pushl $0
	pushl $do_int3
	jmp error_code

overflow:
	pushl $0
	pushl $do_overflow
	jmp error_code

bounds:
	pushl $0
	pushl $do_bounds
	jmp error_code

invalid_op:
	pushl $0
	pushl $do_invalid_op
	jmp error_code

coprocessor_segment_overrun:
	pushl $0
	pushl $do_coprocessor_segment_overrun
	jmp error_code

reserved:
	pushl $0
	pushl $do_reserved
	jmp error_code

double_fault:
	pushl $do_double_fault
	jmp error_code

invalid_TSS:
	pushl $do_invalid_TSS
	jmp error_code

segment_not_present:
	pushl $do_segment_not_present
	jmp error_code

stack_segment:
	pushl $do_stack_segment
	jmp error_code

general_protection:
	pushl $do_general_protection
	jmp error_code

alignment_check:
	pushl $do_alignment_check
	jmp error_code

page_fault:
	pushl $do_page_fault
	jmp error_code
