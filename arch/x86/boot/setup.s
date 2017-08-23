# ----------------------------------------------------------
# Setup.s
# Maintainer: Buddy <buddy.zhang@aliyun.com>
#
# Copyright (C) 2017 BiscuitOS
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
	.code16
#	setup.s		(C) 1991 Linus Torvalds
#
# setup.s is responsible for getting the system data from the BIOS,
# and putting them into the appropriate places in system memory.
# both setup.s and system has been loaded by the bootblock.
#
# This code asks the bios for memory/disk/other parameters, and
# puts them in a "safe" place: 0x90000-0x901FF, ie where the
# boot-block used to be. It is then up to the protected mode
# system to read them from there before the area is overwritten
# for buffer-blocks.
#

# NOTE! These had better be the same as in bootsect.s!

	.equ INITSEG, 0x9000	# we move boot here - out of the way
	.equ SYSSEG, 0x1000	# system loaded at 0x10000 (65536).
	.equ SETUPSEG, 0x9020	# this is the current segment

	.global st_start
	.section ".st_text", "x"

	ljmp $SETUPSEG, $st_start
st_start:

# Get current cursor position and save it on 0x90000
	mov $INITSEG, %ax
	mov %ax, %ds
	mov $0x03, %ah
	xor %bh, %bh
	int $0x10
	mov %dx, %ds:0

# Get Memory size

	mov $0x88, %ah
	int $0x15
	mov %ax, %ds:2

# Get video-card data:

	mov $0x0f, %ah
	int $0x10
	mov %bx, %ds:4
	mov %ax, %ds:6
	mov %ax, %ax
	mov %bx, %bx

# Get hd0 data

	mov $0x0000, %ax
	mov %ax, %ds
	lds %ds:4*0x41, %si
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0080, %di
	mov $0x10, %cx
	rep
	movsb

# Get hd1 data

	mov $0x0000, %ax
	mov %ax, %ds
	lds %ds:4*0x46, %si
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0090, %di
	mov $0x10, %cx
	rep
	movsb

# Check that there IS a hd1 :)

	mov $0x01500, %ax
	mov $0x81, %dl
	int $0x13
	jc no_disk1
	cmp $3, %ah
	je is_disk1
no_disk1:
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0090, %di
	mov $0x10, %cx
	mov $0x00, %ax
	rep
	stosb
is_disk1:

# now we want to move to protected mode ...

	cli	     # Forbidden interrupt

# first mov system to rightful place.
	mov $0x0000, %ax
	cld      # clear direction
do_move:
	mov %ax, %es         # Destination segment
	add $0x1000, %ax
	cmp $0x9000, %ax
	jz end_move
	mov %ax, %ds         # Source segment
	sub %di, %di
	sub %si, %si
	mov $0x8000, %cx
	rep
	movsw
	jmp do_move

# Then load the segment desciptors
end_move:
	mov $SETUPSEG, %ax
	mov %ax, %ds
	lidt idt_48         # load IDT
	lgdt gdt_48         # load GDT


# Enable A20

	inb $0x92, %al    # Open A20 line(Fast Gate A20)
	orb $0b00000010, %al
	outb %al, $0x92

# Reprogram interrupt

	mov	$0x11, %al		# initialization sequence(ICW1)
	out	%al, $0x20		# send it to 8259A-1
	.word	0x00eb,0x00eb		# jmp $+2, jmp $+2
	out	%al, $0xA0		# and to 8259A-2
	.word	0x00eb,0x00eb
	mov	$0x20, %al		# start of hardware int's (0x20)(ICW2)
	out	%al, $0x21		# from 0x20-0x27
	.word	0x00eb,0x00eb
	mov	$0x28, %al		# start of hardware int's 2 (0x28)
	out	%al, $0xA1		# from 0x28-0x2F
	.word	0x00eb,0x00eb		#               IR 7654 3210
	mov	$0x04, %al		# 8259-1 is master(0000 0100) --\
	out	%al, $0x21		#				|
	.word	0x00eb,0x00eb		#			 INT	/
	mov	$0x02, %al		# 8259-2 is slave(       010 --> 2)
	out	%al, $0xA1
	.word	0x00eb,0x00eb
	mov	$0x01, %al		# 8086 mode for both
	out	%al, $0x21
	.word	0x00eb,0x00eb
	out	%al, $0xA1
	.word	0x00eb,0x00eb
	mov	$0xFF, %al		# mask off all interrupts for now
	out	%al, $0x21
	.word	0x00eb,0x00eb
	out	%al, $0xA1

# Simple jmp to 0x00000

	mov %cr0, %eax    # Get machine status
	bts $0, %eax      # Turn on the PE-bit
	mov %eax, %cr0    # Protection enable

	# Segment-desciptor  (INDEX:IT:RPL)
	.equ sel_cs0, 0x0008 # Select for code segment 0 (001:0:00)
	ljmp $sel_cs0, $0    # jmp offset 0 of code segment 0 in GDT


gdt:
	.word	0,0,0,0		# dummy

	.word	0x07FF		# 8Mb - limit=2047 (2048*4096=8Mb)
	.word	0x0000		# base address=0
	.word	0x9A00		# code read/exec
	.word	0x00C0		# granularity=4096, 386

	.word	0x07FF		# 8Mb - limit=2047 (2048*4096=8Mb)
	.word	0x0000		# base address=0
	.word	0x9200		# data read/write
	.word	0x00C0		# granularity=4096, 386

idt_48:
	.word	0			# idt limit=0
	.word	0,0			# idt base=0L

gdt_48:
	.word	0x800			# gdt limit=2048, 256 GDT entries
	.word   512+gdt, 0x9		# gdt base = 0X9xxxx,
	# 512+gdt is the real gdt after setup is moved to 0x9020 * 0x10
	.section ".st_text", "x"
