/* 
 * head.s
 *
 * Copyright (C) 1991 Linus Torvalds
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * head.s contains the 32-bit startup code.
 *
 * NOTE!!! Startup happens at absolute address 0x00000000, which is also
 * where the page directory will exist. The startup code will be
 * overwritten by the page directory.
 */

	.text
	.globl idt, gdt, pg_dir, tmp_floppy_area

pg_dir:
	.globl startup_32

startup_32:
	movl $0x10, %eax  # 0x10, Global data segment.
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	lss stack_start, %esp
	call setup_idt
	call setup_gdt
	movl $0x10, %eax         # Reload all the segment registers
	mov %ax, %ds             # after changing gdt. CS was already
	mov %ax, %es             # reloaded in 'setup_gdt'
	mov %ax, %fs
	mov %ax, %gs
	lss stack_start, %esp
	xorl %eax, %eax
1:
	incl %eax                # Check that A20 really is enabled
	movl %eax, 0x000000      # loop forever if it isn't
	cmpl %eax, 0x100000
	je 1b

/*
 * NOTE! 486 should set bit 16, to check for write-protect in
 * supervisor mode. Then it would be unnecessary with the
 * "verify_area()" -calls. 486 users probably want to set the
 * NE (#5) bit also, so as to use int 16 for math errors.
 */
	movl %cr0, %eax          # Check math chip
	andl $0x80000011, %eax   # Save PG, PE, ET
	/* "orl $0x10020, %eax" here for 486 might be good */
	orl $2, %eax             # set MP
	movl %eax, %cr0
	call check_x87
	jmp after_page_tables

/*
 * We depend on ET to be correct. This checks for 286/387.
 */
check_x87:
	fninit
	fstsw %ax
	cmpb $0, %al
	je 1f              /* no coprocessor: have to set bits */
	movl %cr0, %eax
	xorl $6, %eax      /* reset MP, set EM */
	movl %eax, %cr0
	ret

.align 2
1:
	.byte 0xDB, 0xE4    /* fsetpm for 287, ignored by 387 */
	ret

/*
 * setup_idt
 *
 * Sets up IDT with 256 entries pointing to
 * ignore_int, interrupt gates. It then loads IDT.
 * Everything that wants to install itself in the
 * idt-table may do so themselves. Interrupts
 * are enabled elsewhere, when we can be relatively
 * sure everthing is ok. This routine will be
 * over-written by the page tables.
 */
setup_idt:
	lea ignore_int, %edx
	movl $0x00080000, %eax
	movw %dx, %ax
	movw $0x8E00, %dx

	lea idt, %edi
	mov $256, %ecx
rp_sidt:
	movl %eax, (%edi)
	movl %edx, 4(%edi)
	addl $8, %edi
	dec %ecx
	jne rp_sidt
	lidt idt_descr
	ret

/*
 * setup_gdt
 *
 * This routines sets up a new gdt and loads it.
 * Only two entries are currently built, the same
 * ones that were built in init.s. The routine
 * is VERY complicated at two whole lines, so this
 * rather long comment is certainly needed :-)
 * This routine will be overwritten by the page tables.
 */
setup_gdt:
	lgdt gdt_descr
	ret

/*
 * I put the kernel page tables right after the page directory,
 * using 4 of them to span 16 Mb of physical memory. People with
 * more than 16Mb will have to expand this.
 */
.org 0x1000
pg0:

.org 0x2000
pg1:

.org 0x3000
pg2:

.org 0x4000
pg3:

.org 0x5000

/*
 * tmp_floppy_area is used by the floppy-driver when DMA cannot
 * reach to a buffer-block. It needs to be aligned, so that it isn't
 * on a 64KB border.
 */
tmp_floppy_area:
	.fill 1024,1,0

after_page_tables:
	pushl $0         # These are the parameters to main :-)
	pushl $0
	pushl $0
	pushl $L6
	pushl $main
	jmp setup_paging

L6:
	jmp L6           # main should never return here, but
	                 # Just in case, we know what happens.

/* This is the default interrupt "handler" */
int_msg:
	.asciz "Unknown Interrupt\n\r"
.align 2
ignore_int:
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	pushl $int_msg
	call printk
	popl %eax
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret

/*
 * Setup_paging
 *
 * This routine sets up paging by setting the page bit
 * in cr0. The page tables are set up, identity-mapping
 * the first 16MB. The pager assmes that no illegal
 * addresses are produced (ie >4Mb on a 4Mb machine).
 *
 * NOTE! Although all physical memory should be identity
 * mapped by this routine, only the kernel page functions
 * use the > 1Mb address directly. All "normal" functions
 * use just the lower 1Mb, or the local data space, which
 * will be mapped to some other place - mm keeps track of
 * that.
 *
 * For those with more momory than 16 Mb - tough luck. I've
 * not got it, why should you :-) The source is here. Chnage
 * it. (Seriously - it shouldn't be too difficult. Mostly
 * change some constants etc. I left it at 16Mb, as my machine
 * even cannot be extended past that (OK, but it was cheap :-)
 * I've tried to show which constants to change by having
 * some kind of marker at them (search for "16Mb"), but I
 * won't guarantee that's all :-( )
 */
.align 2
setup_paging:
	movl $1024*5, %ecx      /* 5 pages - pg_dir+4 page tables */
	xorl %eax, %eax
	xorl %edi, %edi         /* pg_dir is at 0x00 */
	cld;rep;stosl
	movl $pg0+7, pg_dir     /* set present bit/user r/w */
	movl $pg1+7, pg_dir+4   /* ------------ " " -------------- */
	movl $pg2+7, pg_dir+8   /* ------------ " " -------------- */
	movl $pg3+7, pg_dir+12  /* ------------ " " -------------- */
	movl $pg3+4092, %edi
	movl $0xfff007, %eax    /* 16Mb - 4096 + 7 (r/w user, p) */
	std
1:  stosl                   /* fill pages backwards - more efficient :-) */
	subl $0x1000, %eax
	jge 1b
	cld
	xorl %eax, %eax         /* pg_dir is at 0x0000 */
	movl %eax, %cr3         /* cr3 - page directory start */
	movl %cr0, %eax
	orl  $0x80000000, %eax
	movl %eax, %cr0         /* set paging (PG) bit */
	ret                     /* This also flushes prefetch-queue */

.align 2
.word 0
idt_descr:
	.word 256*8-1            # IDT contains 256 entries
	.long idt
.align 2
.word 0
gdt_descr:
	.word 256*8-1            # so does gdt (not that that's any
	.long gdt                # magic number, but it works for me:^)

	.align 8
idt:
	.fill 256,8,0            # IDT is uninitialized

gdt:
	.quad 0x0000000000000000  /* NULL descriptor */
	.quad 0x00c09a0000000fff  /* 16Mb */
	.quad 0x00c0920000000fff  /* 16Mb */
	.quad 0x0000000000000000  /* TEMPORARY - don't use */
	.fill 252,8,0             /* space for LDT's and TSS's etc */
