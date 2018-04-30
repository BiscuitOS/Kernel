/*
 * linux/kernel/system_call.s
 *
 * (C) 1991 Linus Torvalds
 */

/*
 * system_call.s contains the system-call low-level handing routines.
 * This also contains the timer-interrupt handler, as some of the code is
 * the same. The hd- and floppy-interrupts are also here.
 *
 * NOTE: this code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call. Ordinary interrupts
 * don't handle signal-recognition, as that would clutter them up totally
 * unnecessarilly.
 *
 * Stack layout in 'ret_from_system_call':
 * ptrace needs to have all regs on the stack.
 * if the order here is changed, it needs to be 
 * updated in fork.c:copy_process, signal.c:do_signal,
 * ptrace.c ptrace.h
 *
 *       0(%esp) - %ebx
 *       4(%esp) - %ecx
 *       8(%esp) - %edx
 *       C(%esp) - %esi
 *      10(%esp) - %edi
 *      14(%esp) - %ebp
 *      18(%esp) - %eax
 *      1C(%esp) - %ds
 *      20(%esp) - %es
 *      24(%esp) - %fs
 *      28(%esp) - %gs
 *      2C(%esp) - %orig_eax
 *      30(%esp) - %eip
 *      34(%esp) - %cs
 *      38(%esp) - %eflags
 *      3C(%esp) - %oldesp
 *      40(%esp) - %oldss
 */

SIG_CHLD	= 17

EBX               = 0x00
ECX               = 0x04
EDX               = 0x08
ESI               = 0x0C
EDI               = 0x10
EBP               = 0x14
EAX               = 0x18
DS                = 0x1C
ES                = 0x20
FS                = 0x24
GS                = 0x28
ORIG_EAX          = 0x2C
EIP               = 0x30
CS                = 0x34
EFLAGS            = 0x38
OLDESP            = 0x3C
OLDSS             = 0x40

state      = 0       # these are offsets into the task-struct.   
counter    = 4
priority   = 8
signal     = 12
sigaction  = 16      # Must be 16 (=len of sigaction)
blocked    = (33 * 16)

# offsets within sigaction
sa_handler = 0
sa_mask    = 4
sa_flags   = 8
sa_restorer = 12

nr_system_calls = 82

ENOSYS = 38

/*
 * Ok, I get parallel printer interrupts while using the floppy for some
 * strange reason. Urgel. Now I just ignore them.
 */
.globl system_call, timer_interrupt, sys_execve 
.globl hd_interrupt, floppy_interrupt, parallel_interrupt
.globl device_not_available, coprocessor_error

.align 2
bad_sys_call:
	pushl $-ENOSYS
	jmp ret_from_sys_call
.align 2
reschedule:
        pushl $ret_from_sys_call
        jmp schedule
.align 2
system_call:
	cld
	pushl	%eax		# save orig_eax
        push	%gs
        push	%fs
        push	%es
        push	%ds
	pushl	%eax		# Save eax. The return value will be put here
        pushl	%ebp
        pushl	%edi
        pushl	%esi
        pushl	%edx
        pushl	%ecx              # push %ebx,%ecx,%edx as parameters
        pushl	%ebx              # to the system call
        movl	$0x10,%edx         # set up ds,es to kernel space
        mov	%dx,%ds
        mov	%dx,%es
        movl	$0x17,%edx         # fs points to local data space
        mov	%dx,%fs
	cmpl	NR_syscalls, %eax
	jae	bad_sys_call
        call	*sys_call_table(,%eax,4)
	movl	%eax, EAX(%esp)		# save the return value
2:
        movl current,%eax
        cmpl $0,state(%eax)             # state
        jne reschedule
        cmpl $0,counter(%eax)           # counter
        je reschedule
ret_from_sys_call:
	movl current, %eax
	cmpl task, %eax		# task[0] cannot have signals
	je 3f
	cmpw $0x0f, CS(%esp)
	jne 3f
	cmpw $0x17, OLDSS(%esp)
	jne 3f
	movl signal(%eax), %ebx
	movl blocked(%eax), %ecx
	notl %ecx
	andl %ebx, %ecx
	bsfl %ecx, %ecx
	je 3f
	btrl %ecx, %ebx
	movl %ebx, signal(%eax)
	incl %ecx
	pushl %ecx
	call do_signal
	popl %ecx
	testl %eax, %eax
	jne 2b		# see if we need to switch tasks, or do more signals
3:
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popl %eax
	pop  %ds
	pop  %es
	pop  %fs
	pop  %gs
	addl $4, %esp	# skip orig_eax
	iret

.align 2
coprocessor_error:
	cld
	pushl $-1	# mark this as an int.
	push %gs
	push %fs
	push %es
	push %ds
	pushl %eax	# save eax.
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %edx
	pushl %ecx
	pushl %ebx
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	movl $0x17, %eax
	mov %ax, %fs
	pushl $ret_from_sys_call
	jmp math_error

.align 2
device_not_available:
	cld
	pushl $-1	# mark this as an int
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
	movl $0x10, %eax
	mov  %ax, %ds
	mov  %ax, %es
	movl $0x17, %eax
	mov  %ax, %fs
	pushl $ret_from_sys_call
	clts
	movl %cr0, %eax
	testl $0x4, %eax
	je math_state_restore
	pushl $0	# temporary storage for ORIG_EIP
	call math_emulate
	addl $4, %esp
	ret

.align 2
timer_interrupt:
	cld
	pushl $-1	# mark this as an int
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
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	movl $0x17, %eax
	mov %ax, %fs
	incl jiffies
	movb $0x20, %al       # EOI to interrupt controller #1
	outb %al, $0x20
	movl CS(%esp), %eax
	andl $3, %eax		# %eax is CPL (0 or 3, 0=supervisor)
	pushl %eax
	call do_timer		# 'do_timer(long CPL)' does everything from
	addl $4, %esp		# task switching to accounting ...
	jmp ret_from_sys_call

.align 2
sys_execve:
	lea (EIP+4)(%esp), %eax	# don't forget about the return address.
	pushl %eax
	call do_execve
	addl $4, %esp
	ret

hd_interrupt:
	cld
	pushl %eax
	pushl %ecx
	pushl %edx
	push  %ds
	push  %es
	push  %fs
	movl  $0x10, %eax
	mov   %ax, %ds
	mov   %ax, %es
	movl  $0x17, %eax
	mov   %ax, %fs
	movb  $0x20, %al
	outb  %al, $0xA0     # EOI to interrupt controller #1
	jmp   1f             # give port chance to breathe
1:	jmp   1f
1:	outb  %al, $0x20
	andl  $0xfffeffff, timer_active
	xorl  %edx, %edx
	xchgl do_hd, %edx
	testl %edx, %edx
	jne 1f
	movl $unexpected_hd_interrupt, %edx
1:
	call *%edx           # "interrupting" way of handing intr.
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret

floppy_interrupt:
	cld
	pushl %eax
	pushl %ecx
	pushl %edx
	push  %ds
	push  %es
	push  %fs
	movl $0x10, %eax
	mov  %ax, %ds
	mov  %ax, %es
	movl $0x17, %eax
	mov  %ax, %fs
	movb $0x20, %al
	outb %al, $0x20         # EOI to interrupt controller #1
	xorl %eax, %eax
	xchgl do_floppy, %eax
	testl %eax, %eax
	jne 1f
	movl $unexpected_floppy_interrupt, %eax
1:	call *%eax              # "interrupting" way of handing intr.
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret

parallel_interrupt:
	cld
	pushl %eax
	movb $0x20, %al
	outb %al, $0x20
	popl %eax
	iret
