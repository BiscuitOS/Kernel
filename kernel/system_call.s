

CS        = 0x20
OLDSS     = 0x2C

signal    = 12
blocked   = (33 * 16)

.globl coprocessor_error, parallel_interrupt
.globl device_not_available

.align 2
bad_sys_call:
	movl $-1, %eax
	iret
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
device_not_available:
	ret

parallel_interrupt:
	pushl %eax
	movb $0x20, %al
	outb %al, $0x20
	popl %eax
	iret
