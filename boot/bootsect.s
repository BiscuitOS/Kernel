# ----------------------------------------------------------
# Bootsect.s
# Maintainer: Buddy <buddy.zhang@aliyun.com>
#
# Copyright (C) 2017 BiscuitOS
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
	.code16
#
# SYS_SIZE is the number of clicks (16 bytes) to be loaded.
# 0x3000 is 0x30000 bytes = 196KB, more than enough for current
# version of linux
	.equ SYSSIZE, 0x3000

	.global _start, begtext, begdata, begbss, endtxt, enddata, endbss
	.text
	begtext:
	.data
	begdata:
	.bss
	begbss:
	.text

	.equ SETUPLEN, 4        # nr of setup-sectors
	.equ BOOTSEG, 0x07C0    # original address of boot-sector
	.equ INITSEG, 0x9000    # we move boot here - out of the way
	.equ SETUPSEG, 0x9020   # setup starts here
	.equ SYSSEG, 0x1000     # system loaded at 0x10000 (65536).
	.equ ENDSEG, SYSSEG + SYSSIZE # where to stop loading


	.equ ROOT_DEV, 0x301


	# Normalize the start address
	ljmp $BOOTSEG, $_start

_start:
	nop
	mov $0x0000, %dx
	mov $0x0000, %ax
	int $0x13
	nop
	mov $0x00, %dl
	mov $0x0800, %ax
	int $0x13
	nop
	nop

	mov $BOOTSEG, %ax
	mov %ax, %ds
	mov $INITSEG, %ax
	mov %ax, %es
	mov $256, %cx
	sub %si, %si
	sub %di, %di
	rep
	movsw
	ljmp $INITSEG, $go
go:
	mov %cs, %ax
	mov %ax, %ds
	mov %ax, %es
# put stack at 0x9ff00
	mov %ax, %ss
	mov $0xFF00, %sp # (cs << 4) + sp 

# load the setup-sectors directly after the bootblock.
# Note that 'es' is already set up.

load_setup:
	mov $0x0000, %dx     # drive 0, head 0
	mov $0x0002, %cx     # sector 2, track 0
	xorw %sp, %sp
	sti
	cld

	movw $bugger_off_msg, %si

msg_loop:
	lodsb
	andb %al, %al
	jz bs_die
	movb $0xe, %ah
	movw $7, %bx
	int $0x10
	jmp msg_loop

bs_die:
	# Allow the user to press a key, then reboot
	xorw %ax, %ax
	int $0x16
	int $0x19

	# int 0x19 should never return. In case it does anyway,
	# invoke the BIOS reset code...
	ljmp $0xf000, $0xfff0

bugger_off_msg:
	.ascii "Hello World!\r\n"
	.ascii "Bye next world!\r\n"
	.ascii "\n"
	.byte 0

	.org 510
	.word 0xAA55
