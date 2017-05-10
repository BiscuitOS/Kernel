

.global page_fault

page_fault:
	xchgl %eax, (%esp)
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10, %edx
	mov %dx, %ds
	mov %dx, %es
	mov %dx, %fs
	movl %cr2, %edx
	pushl %edx
	pushl %eax
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret
