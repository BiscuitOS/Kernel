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

	# BiscuitOS support boot from floppy and hard disk
    # Boot from first hard disk
	# .equ DEVICE_NR, 0x80
    # Boot from first floppy
	.equ DEVICE_NR, 0x00

	.equ SETUPLEN, 4        # nr of setup-sectors
	.equ BOOTSEG, 0x07C0    # original address of boot-sector
	.equ INITSEG, 0x9000    # we move boot here - out of the way
	.equ SETUPSEG, 0x9020   # setup starts here
	.equ SYSSEG, 0x1000     # system loaded at 0x10000 (65536).
	.equ ENDSEG, SYSSEG + SYSSIZE # where to stop loading

# ROOT_DEV: rootfs
# Device number of Rootfs. define as:
# Device number = major * 256 + minor
# dev_no = (major << 8) + minor
# Major device number:
# Memory   : 1
# floppy   : 2
# Disk     : 3
# ttyx     : 4
# tty      : 5
# parallel : 6
# non-pipo : 7
# 0x300    : /dev/hd0 - The first hard disk
# 0x301    : /dev/hd1 - The first partition on first hard disk.
# 0x302    : /dev/hd2 - The second partition on first hard disk.
# ...
# 0x304    : /dev/hd4 - The second hard disk
# 0x305    : /dev/hd5 - The first partition on second hard disk.
# 0x306    : /dev/hd6 - The second partition on second hard disk.
# ROOT_DEV = 0 ; The same device with boot device.
	.equ ROOT_DEV, 0x301

	# Normalize the start address
	ljmp $BOOTSEG, $_start

_start:
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
	mov $0xFF00, %sp # arbitrary value >> 512

# load the setup-sectors directly after the bootblock
# Note that 'es' is already set up

load_setup:
	# If use hard disk, dirver is 0x80
	mov $0x0000, %dx      # head 0
	mov $DEVICE_NR, %dl   # dirve 0
	mov $0x0002, %cx   # sector 2, track 0
	mov $0x0200, %bx   # address = 512, in INITSEG
	.equ     AX, 0x200+SETUPLEN
	mov     $AX, %ax   # service 2, nr of sectors
	int $0x13          # read it
	mov %ax, %ax
	jnc ok_load_setup  # ok -continue
	mov $0x0000, %dx
	mov $DEVICE_NR, %dl
	mov $0x0000, %ax   # reset the diskette
	int $0x13
	jmp load_setup

ok_load_setup:

# Get disk dirve parameters, specifically nr of sectors/track

	mov $DEVICE_NR, %dl
	mov $0x0800, %ax
	int $0x13
	mov $0x00, %ch
	#seg cs
	mov %cx, %cs:sectors+0
	mov $INITSEG, %ax
	mov %ax, %es

# Print some iname message

	mov $0x03, %ah     # read cursor pos
	xor %bh, %bh
	int $0x10

	mov $27, %cx
	mov $0x0007, %bx   # page 0, attribute 7 (normal)
#	lea msg1, %bp
	mov $msg1, %bp
	mov $0x1301, %ax
	int $0x10

# Ok, we've written the message, now
# we want to load the system (at 0x10000)

	mov $SYSSEG, %ax
	mov %ax, %es    # Segment of 0x010000
	call read_it
	call kill_motor

# After that we check which root-device to use. If the device is
# defined (#= 0), nothing is done and the given device is used.
# Otherwise, either /dev/PS0 (2,28) or /dev/at0 (2,8), depending
# on the number of sectors that the BIOS report currently.

	#seg cs
	mov %cs:root_dev+0, %ax
	cmp $0, %ax
	jne root_defined
	#seg cs
	mov %cs:sectors+0, %bx
	mov $0x0208, %ax        # /dev/ps0 - 1.2Mb
	cmp $15, %bx
	je root_defined
	mov $0x021c, %ax        # /dev/PS0 - 1.44Mb
	cmp $18, %bx
	je root_defined
undef_root:
	jmp undef_root
root_defined:
	#seg cs
	mov %ax, %cs:root_dev+0

# after that (everything loaded), we jump to
# the setup-routine loaded directly after
# the bootblock:
	ljmp $SETUPSEG, $0

# This routine loads the system at address 0x10000, making sure
# no 64kB boundaries are crossed. We try to load it as fast as
# possible, loading whole tracks whenever we can.
#
# in:   es - starting address segment (normally 0x1000)
#
sread:	.word 1+ SETUPLEN  # sectors read of current track
head:	.word 0			   # current head
track:	.word 0			   # current track

read_it:
	mov %es, %ax
	test $0x0fff, %ax
die:
	jne die       # es must be at 64kB boundary
	xor %bx, %bx  # bx is starting address with segment
rp_read:
	mov %es, %ax
	cmp $ENDSEG, %ax   # have we loaded all yet?
	jb ok1_read
	ret

ok1_read:
	#seg cs
	mov %cs:sectors+0, %ax
	sub sread, %ax
	mov %ax, %cx
	shl $9, %cx
	add %bx, %cx
	jnc ok2_read     # don't over 64KB
	je ok2_read
	xor %ax, %ax     # over 64KB
	sub %bx, %ax
	shr $9, %ax
ok2_read:
	call read_track
	mov  %ax, %cx
	add sread, %ax
	#seg cs
	cmp %cs:sectors+0, %ax
	jne ok3_read
	mov $1, %ax
	sub head, %ax
	jne ok4_read
	incw track
ok4_read:
	mov %ax, head
	xor %ax, %ax
ok3_read:
	mov %ax, sread
	shl $9, %cx
	add %cx, %bx
	jnc rp_read
	mov %es, %ax
	add $0x1000, %ax
	mov %ax, %es
	xor %bx, %bx
	jmp rp_read

# Read a track data
# AL: the number of read sector
# [BX:ES]: Store data
read_track:
	push %ax
	push %bx
	push %cx
	push %dx
	mov track, %dx
	mov sread, %cx
	inc %cx             # Sectors
	mov %dl, %ch        # Cylinders
	mov head, %dx
	mov %dl, %dh        # Heads
	mov $DEVICE_NR, %dl
	and $0x0100, %dx  # boot from floppy
	mov $2, %ah
	int $0x13
	jc bad_rt
	pop %dx
	pop %cx
	pop %bx
	pop %ax
	ret
bad_rt:
	mov $0, %ax
	mov $0, %dx
	int $0x13
	pop %dx
	pop %cx
	pop %bx
	pop %ax
	jmp read_track

#
# This procedure turns off the floppy dirve motor, so
# that we enter the kernel a known state, and
# don't have to worry about it later.
kill_motor:
	push %dx
	mov $0x3f2, %dx
	mov $0, %al
	outsb
	pop %dx
	ret

sectors:
	.word 0

msg1:
	.byte 13,10
	.ascii "Loading BiscuitOS ..."
	.byte 13,10,13,10

	.org 508
root_dev:
	.word ROOT_DEV
	.word 0xAA55

	.text
	endtext:
	.data
	enddata:
	.bss
	endbss:

