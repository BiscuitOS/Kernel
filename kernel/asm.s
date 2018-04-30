/*
 *  linux/kernel/asm.s
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * asm.s contains the low-level code for most hardware faults.
 * page_exception is handled by the mm, so that isn't here. This
 * file also handles (hopefully) fpu-exceptions due to TS-bit, as
 * the fpu must be properly saved/resored. This hasn't been tested.
 */

.globl divide_error,debug,nmi,int3,overflow,bounds,invalid_op
.globl double_fault,coprocessor_segment_overrun
.globl invalid_TSS,segment_not_present,stack_segment
.globl general_protection,coprocessor_error,irq13,reserved
.globl alignment_check
.globl page_fault

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
	xchgl %eax, 0x2c(%esp)	# orig_eax (get the error code. )
	xorl %ebx,%ebx		# zero ebx
	mov %gs,%bx		# get the lower order bits of gs
	xchgl %ebx, 0x28(%esp)	# get the address and save gs.
	pushl %eax		# push the error code
	lea 52(%esp),%edx
	pushl %edx
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	call *%ebx
	addl $8,%esp
	popl %ebx
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
	addl $4,%esp
	iret

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
	jmp coprocessor_error

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
