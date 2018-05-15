	.code16
# rewrite with AT&T syntax by falcon <wuzhangjin@gmail.com> at 081012
# modify for biscuitos by buddy <buddy.zhang@aliyun.com> at 170303
# rewrite for biscuitos by buddy <buddy.zhang@aliyun.com> at 180420
#
# SYS_SIZE is the number of clicks (16 bytes) to be loaded.
# 0x7F00 is 0x7F000 bytes = 508kB, more than enough for current
# versions of linux which compress the kernel
	.equ SYSSIZE, 0x8000

#
#	bootsect.s		Copyright (C) 1991, 1992 Linus Torvalds
#       modified by Drew Eckhardt
#       modified by Bruce Evans (bde)
#
# bootsect.s is loaded at 0x7c00 by the bios-startup routines, and moves
# iself out of the way to address 0x90000, and jump there.
#
# It then load's 'setup' directly after itself (0x90200), and the system
# at 0x10000, using BIOS interrupts.
#
# NOTE! currently system is at most (8*65536-4096) bytes long. This should 
# be no problem, even in the future. I want to keep it simple. This 508 kB
# kernel size should be enough, especially as this doesn't contain the
# buffer cache as in minix (and especially now that the kernel is 
# compressed :-)
#
# The loader has been make as simple as possible, and continuos
# read errors will result in a unbreakable loop. Reboot by hand. It
# loads pretty fast by getting whole sectors at a time whenever possible.

	.global _start, begtext, begdata, begbss, endtext, enddata, endbss

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

	.equ	ROOT_DEV, 0x301
	.equ	SWAP_DEV, 0x302

	# Normalize the start address
	ljmp	$BOOTSEG, $_start

_start:
	mov	$BOOTSEG, %ax
	mov	%ax, %ds
	mov	$INITSEG, %ax
	mov	%ax, %es
	mov	$256, %cx
	sub	%si, %si
	sub	%di, %di
	cld
	rep
	movsw
	ljmp	$INITSEG, $go
go:
	mov	%cs, %ax
	mov	$0x3FF4, %dx
	mov	%ax, %ds
	mov	%ax, %es
	push	%ax

# put stack at 0x9ff00 - 12
	mov	%ax, %ss
	mov	%dx, %sp

#
# Many BIOS's default disk paramenter tables will not
# recognize multi-sector reads beyond the maximum sector number
# specified in the default diskette parameter tables - this may
# mean 7 sectors in some cases.
#
# Since single sector reads are slow and out of the question,
# we must take care of this by creating new parameter tables
# (for the first disk) in RAM. We will set the maximum sector
# count to 18 - the most we will encounter on an HD 1.44.
#
# High doesn't hurt. Low does.
#
# Segments are as follows: ds=es=ss=cs - INITSEG,
#     fs = 0, es = parameter table segment
#
	push 	$0
	pop 	%fs
	mov 	$0x78, %di
	mov	%di, %bx
	mov	%fs:(%di), %si
	mov	%fs:2(%di), %ds
	mov 	%ax, %es	# ds:si is source
	mov 	%dx, %di	# es:di is distination point to parameter table

	mov	$6, %cx		# copy 12 bytes
	cld

	rep
	movsw

	mov	%dx, %di
	movb	$18, %es:4(%di)	# patch sector count

	#seg	%ds
	mov	%di, %fs:(%bx)
	#seg	%ds
	mov	%es, %ds:2(%bx)

	pop	%ax	# Bad pop operation!
	push	%cs
	pop	%ax
	mov	%ax, %ds
	mov	%ax, %es

	xor	%ah, %ah         # reset FDC
	xor	%dl, %dl
	int	$0x13

# load the setup-sectors directly after the bootblock
# Note that 'es' is already set up

load_setup:
	# If use hard disk, dirver is 0x80
	xor %dx, %dx       # head 0
	mov $0x0002, %cx   # sector 2, track 0
	mov $0x0200, %bx   # address = 512, in INITSEG
	.equ     AX, 0x200+SETUPLEN
	mov     $AX, %ax   # service 2, nr of sectors
	int $0x13          # read it
	jnc ok_load_setup  # ok -continue

	push %ax           # dump error code
	call print_nl
	mov %sp, %bp
	call print_hex
	pop %ax

	xor %dl, %dl       # reset FDC
	xor %ah, %ah
	int $0x13
	jmp load_setup

ok_load_setup:

# Get disk dirve parameters, specifically nr of sectors/track

	xor %dl, %dl
	mov $0x08, %ah      # AH=8 is get drive parameters
	int $0x13
	xor %ch, %ch
	#seg cs
	mov %cx, %cs:sectors+0
	mov $INITSEG, %ax
	mov %ax, %es

# Print some iname message

	mov $0x03, %ah     # read cursor pos
	xor %bh, %bh
	int $0x10

	mov $20, %cx
	mov $0x0007, %bx   # page 0, attribute 7 (normal)
	xor %dx, %dx
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
	call print_nl

# After that we check which root-device to use. If the device is
# defined (#= 0), nothing is done and the given device is used.
# Otherwise, either /dev/PS0 (2,28) or /dev/at0 (2,8), depending
# on the number of sectors that the BIOS report currently.

	#seg cs
	mov %cs:root_dev+0, %ax
	or %ax, %ax
	jne root_defined
	#seg cs
	mov %cs:sectors+0, %bx
	mov $0x0208, %ax        # /dev/ps0 - 1.2Mb
	cmp $15, %bx
	je root_defined
	mov $0x021c, %ax        # /dev/PS0 - 1.44Mb
	cmp $18, %bx
	je root_defined
	mov $0x200, %ax
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
	sub $SYSSEG, %ax
	cmp $SYSSIZE, %ax   # have we loaded all yet?
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
	add $0x10, %ah
	mov %ax, %es
	xor %bx, %bx
	jmp rp_read

# Read a track data
# AL: the number of read sector
# [BX:ES]: Store data
read_track:
	pusha
	pusha
	mov $0xe2e, %ax     # loading... message 2e= .
	mov $7, %bx
	int $0x10
	popa

	mov track, %dx
	mov sread, %cx
	inc %cx             # Sectors
	mov %dl, %ch        # Cylinders
	mov head, %dx
	mov %dl, %dh        # Heads
	and $0x0100, %dx  # boot from floppy
	mov $2, %ah
	
	push %dx            # save for error dump
	push %cx
	push %bx
	push %ax

	int $0x13
	jc bad_rt
	add $8, %sp
	popa
	ret
bad_rt:
	push %ax            # save error code
	call print_all      # ah = error, al = read
	xor %ah, %ah
	xor %dl, %dl
	int $0x13
	add $10, %sp
	popa
	jmp read_track

/*
 * print_all is for debugging purposes.
 * It will print out all of the registers. The assumption is that this is
 * called from a routine, with a stack frame like
 * dx
 * cx
 * ax
 * error
 * ret <- sp
 */
print_all:
	mov $5, %cx       # error code + 4 registers
	mov %sp, %bp

print_loop:
	push %cx          # save count left
	call print_nl     # nl for readability
	jae no_reg        # see if register name is needed

	mov $0xe45, %ax
	sub %cl, %al
	int $0x10

	mov $0x58, %al
	int $0x10

	mov $0x3a, %al
	int $0x10

no_reg:
	add $2, %bp       # next register
	call print_hex
	pop %cx
	loop print_loop
	ret

print_nl:
	mov $0xe0d, %ax   # CR
	int $0x10
	mov $0xa, %al     # LF
	int $0x10
	ret

/*
 * print_hex is for debugging purposes, and prints the word
 * pointed to by ss:bp in hexadecmial.
 */
print_hex:
	mov $4, %cx       # 4 hex digits
	mov (%bp), %dx    # load word into dx
print_digit:
	rol $4, %dx       # rotate so that lowest 4 bits are used
	mov $0xe, %ah
	mov %dl, %al      # mask off so we have only next nibble
	and $0xf, %al
	add $0x30, %al    # convert to 0 based digit, '0'
	cmp $0x39, %al    # check for overflow
	jbe good_digit
	add $0x7, %al     # 'A' - '0' - 0xa

good_digit:
	int $0x10
	loop print_digit
	ret

#
# This procedure turns off the floppy dirve motor, so
# that we enter the kernel a known state, and
# don't have to worry about it later.
kill_motor:
	push %dx
	mov $0x3f2, %dx
	xor %al, %al
	outsb
	pop %dx
	ret

sectors:
	.word 0

msg1:
	.ascii "Loading BiscuitOS ..."

	.org 502
swap_dev:
	.word SWAP_DEV
ram_size:
	.word 0
vid_mode:
	.word 0
root_dev:
	.word ROOT_DEV
boot_flag:
	.word 0xAA55

	.text
	endtext:
	.data
	enddata:
	.bss
	endbss:
