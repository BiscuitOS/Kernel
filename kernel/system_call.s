

CS        = 0x20
OLDSS     = 0x2C

signal    = 12
blocked   = (33 * 16)

state   = 0
counter = 4
signal  = 12
blocked = (33 * 16)

nr_system_calls = 72

.globl coprocessor_error, parallel_interrupt
.globl device_not_available, timer_interrupt, system_call
.globl hd_interrupt, floppy_interrupt
.globl sys_fork

.align 2
bad_sys_call:
	movl $-1, %eax
	iret
.align 2
reschedule:
        pushl $ret_from_sys_call
        jmp schedule
.align 2
system_call:
        cmpl $nr_system_calls-1,%eax
        ja bad_sys_call
        push %ds
        push %es
        push %fs
        pushl %edx
        pushl %ecx              # push %ebx,%ecx,%edx as parameters
        pushl %ebx              # to the system call
        movl $0x10,%edx         # set up ds,es to kernel space
        mov %dx,%ds
        mov %dx,%es
        movl $0x17,%edx         # fs points to local data space
        mov %dx,%fs
        call *sys_call_table(,%eax,4)
        pushl %eax
        movl current,%eax
        cmpl $0,state(%eax)             # state
        jne reschedule
        cmpl $0,counter(%eax)           # counter
        je reschedule
.align 2
ret_from_sys_call:
	movl current, %eax
	cmpl task, %eax
	je 3f
	cmpw $0x0f, CS(%esp)
	jne 3f
	cmpw $0x17, OLDSS(%esp)
	jne 3f
	mov signal(%eax), %ebx
	mov blocked(%eax), %ecx
	notl %ecx
	andl %ebx, %ecx
	bsfl %ecx, %ecx
	je 3f
	btrl %ecx, %ebx
	movl %ebx, signal(%eax)
	incl %ecx
	pushl %ecx
	call do_signal
	popl %eax
3:	popl %eax
	popl %ebx
	popl %ecx
	popl %edx
	pop %fs
	pop %es
	pop %ds
	iret

.align 2
coprocessor_error:
	push %ds
	push %es
	push %fs
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	movl $0x17, %eax
	mov %ax, %fs
	pushl $ret_from_sys_call
	jmp math_error

.align 2
timer_interrupt:
	push %ds              # save ds, es and put kernel data space
	push %es              # into them. %fs is used by _system_call
	push %fs
	pushl %edx            # we save %eax, %ecx, %edx as gcc doesn't
	pushl %ecx            # save those across function calls. %ebx
	pushl %ebx            # is saved as we use that in ret_sys_call
	pushl %eax
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	movl $0x17, %eax
	mov %ax, %fs
	incl jiffies
	movb $0x20, %al       # EOI to interrupt controller #1
	outb %al, $0x20
	movl CS(%esp), %eax
	andl $3, %eax
	pushl %eax
	call do_timer
	addl $4, %esp
	jmp ret_from_sys_call
	ret

.align 2
device_not_available:
	push %ds
	push %es
	push %fs
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %eax
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
	pushl %ebp
	pushl %esi
	pushl %edi
	call math_emulate
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 2
hd_interrupt:
	ret

.align 2
floppy_interrupt:
	ret

.align 2
sys_fork:
	call find_empty_process
	testl %eax, %eax
	js 1f
	push %gs
	pushl %esi
	pushl %edi
	pushl %ebp
	pushl %eax
	call copy_process
	addl $20, %esp
1:  ret

parallel_interrupt:
	pushl %eax
	movb $0x20, %al
	outb %al, $0x20
	popl %eax
	iret
