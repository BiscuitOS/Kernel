# rewrite with AT&T syntax by falcon <wuzhangjin@gmail.com> at 081012
# modify  for biscuitos by buddy <buddy.zhang@aliyun.com> at 170303
# rewrite for biscuitos by buddy <buddy.zhang@aliyun.com> at 180420
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
	.code16
# rewrite with AT&T syntax by falcon <wuzhangjin@gmail.com> at 081012
#
# 	setup.s		Copyright (C) 1991, 1992 Linus Torvalds
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
# Move PS/2 aux init code to psaux.c
# (troyer@saifr00.cfsat.Honeywell.COM) 03Oct92
#
# some changes and additional features by Christoph Niemann, March 1993
# (niemann@rubdv15.ETDV.Ruhr-Uni-Bochum.De)

# NOTE! These had better be the same as in bootsect.s!

	.equ INITSEG, 0x9000	# we move boot here - out of the way
	.equ SYSSEG, 0x1000	# system loaded at 0x10000 (65536).
	.equ SETUPSEG, 0x9020	# this is the current segment
	.equ NORMAL_VGA, 0xffff

	.global _start, begtext, begdata, begbss, endtext, enddata, endbss
	.text
	begtext:
	.data
	begdata:
	.bss
	begbss:
	.text

	ljmp $SETUPSEG, $_start	
_start:

# ok, the read went well so we get current cursor position and save it for
# posterity.

	mov	$INITSEG, %ax	# this is done in bootsect already, but...
	mov	%ax, %ds
	mov	$0x03, %ah
	xor	%bh, %bh
	int	$0x10
	mov	%dx, %ds:0
# Get memory size (extended mem, kB)

	mov	$0x88, %ah 
	int	$0x15
	mov	%ax, %ds:2

# set the keyboard repeat rate to the max
	mov	$0x0305, %ax
	xor	%bx, %bx
	int	$0x16

# Get video-card data:
	mov	$0x0f, %ah
	int	$0x10
	mov	%bx, %ds:4
	mov	%ax, %ds:6
	
# check for EGA/VGA and some config parameters

	mov	$0x12, %ah
	mov	$0x10, %bl
	int	$0x10
	mov	%ax, %ds:8
	mov	%bx, %ds:10
	mov	%cx, %ds:12
	jmp	novideo_check
	mov	$0x5019, %ax
	cmp	$0x10, %bl
	je	novga
	mov	$0x1a00, %ax	#Added check for EGA/VGA discrimination
	int	$0x10
	mov	%ax, %bx
	mov	$0x5019, %ax
	cmp	$0x1a, %bl	#1a means VGA, anything else EGA or lower
	jne	novga
	call	chsvga

novga:
	mov	%ax, %ds:14
	mov	$0x03, %ah	# read cursor pos
	xor	%bh, %bh	# clear bh
	int	$0x10		# save it in know place, con_init fetches
	mov	%dx, %ds:0	# it from 0x90000.

novideo_check:
	
# Get video-card data:
	mov	$0x0f, %ah
	int	$0x10
	mov	%bx, %ds:4	# bh = display page
	mov	%ax, %ds:6	# al = video mode, ah = window width

# Get hd0 data

	xor	%ax, %ax	# Clear ax
	mov	%ax, %ds
	lds	%ds:4*0x41, %si
	mov	$INITSEG, %ax
	mov	%ax, %es
	mov	$0x0080, %di
	mov	$0x10, %cx
	cld
	rep
	movsb

# Get hd1 data

	xor	%ax, %ax	# Clear ax
	mov	%ax, %ds
	lds	%ds:4*0x46, %si
	mov	$INITSEG, %ax
	mov	%ax, %es
	mov	$0x0090, %di
	mov	$0x10, %cx
	cld
	rep
	movsb

# Check that there IS a hd1 :-)

	mov	$0x01500, %ax
	mov	$0x81, %dl
	int	$0x13
	jc	no_disk1
	cmp	$3, %ah
	je	is_disk1
no_disk1:
	mov	$INITSEG, %ax
	mov	%ax, %es
	mov	$0x0090, %di
	mov	$0x10, %cx
	xor	%ax, %ax	# clear ax
	cld
	rep
	stosb
is_disk1:

# check for PS/2 pointing device

	mov	$INITSEG, %ax
	mov	%ax, %ds
#	mov	$0x0, %ds:0x1ff	# default is no pointing device
	int	$0x11		# int 0x11: equipment determination
	test	$0x04, %al	# check if pointing device installed
	jz	no_psmouse
#	mov	$0xaa, %ds:0x1ff	# device present
no_psmouse:
# now we want to move to protected mode ...

	cli			# no interrupts allowed ! 
	mov	$0x80, %al	# disable NMI for the bootup sequence
	out	%al, $0x70

# first we move the system to it's rightful place

	mov	$0x100, %ax	# start of destination segment
	mov	$0x1000, %bx	# start of source segment
	cld			# 'direction'=0, movs moves forward
do_move:
	mov	%ax, %es	# destination segment
	add	$0x100, %ax
	cmp	$0x9000, %ax
	jz	end_move
	mov	%bx, %ds	# source segment
	add	$0x100, %bx
	sub	%di, %di
	sub	%si, %si
	mov 	$0x800, %cx
	rep
	movsw
	jmp	do_move

# then we load the segment descriptors

end_move:
	mov	$SETUPSEG, %ax	# right, forgot this at first. didn't work :-)
	mov	%ax, %ds
	lidt	idt_48		# load idt with 0,0
	lgdt	gdt_48		# load gdt with whatever appropriate

# that was painless, now we enable A20

	#call	empty_8042	# 8042 is the keyboard controller
	#mov	$0xD1, %al	# command write
	#out	%al, $0x64
	#call	empty_8042
	#mov	$0xDF, %al	# A20 on
	#out	%al, $0x60
	#call	empty_8042
	inb     $0x92, %al	# open A20 line(Fast Gate A20).
	orb     $0b00000010, %al
	outb    %al, $0x92

# make sure any possible coprocessor is properly reset..

	xor	%ax, %ax
	out	%al, $0xf0
	call	delay
	out	%al, $0xf1
	call	delay

# well, that went ok, I hope. Now we have to reprogram the interrupts :-(
# we put them right after the intel-reserved hardware interrupts, at
# int 0x20-0x2F. There they won't mess up anything. Sadly IBM really
# messed this up with the original PC, and they haven't been able to
# rectify it afterwards. Thus the bios puts interrupts at 0x08-0x0f,
# which is used for the internal hardware interrupts as well. We just
# have to reprogram the 8259's, and it isn't fun.

	mov	$0x11, %al		# initialization sequence(ICW1)
					# ICW4 needed(1),CASCADE mode,Level-triggered
	out	%al, $0x20		# send it to 8259A-1
	call	delay
	out	%al, $0xA0		# and to 8259A-2
	call	delay
	mov	$0x20, %al		# start of hardware int's (0x20)(ICW2)
	out	%al, $0x21		# from 0x20-0x27
	call	delay
	mov	$0x28, %al		# start of hardware int's 2 (0x28)
	out	%al, $0xA1		# from 0x28-0x2F
	call	delay
	mov	$0x04, %al		# 8259-1 is master(0000 0100) --\
	out	%al, $0x21		#				|
	call	delay
	mov	$0x02, %al		# 8259-2 is slave(       010 --> 2)
	out	%al, $0xA1
	call	delay
	mov	$0x01, %al		# 8086 mode for both
	out	%al, $0x21
	call	delay
	out	%al, $0xA1
	call	delay
	mov	$0xFF, %al		# mask off all interrupts for now
	out	%al, $0xA1
	call	delay
	mov	$0xFB, %al
	out	%al, $0x21

# well, that certainly wasn't fun :-(. Hopefully it works, and we don't
# need no steenking BIOS anyway (except for the initial loading :-).
# The BIOS-routine wants lots of unnecessary data, and it's less
# "interesting" anyway. This is how REAL programmers do it.
#
# Well, now's the time to actually move into protected mode. To make
# things as simple as possible, we do no register set-up or anything,
# we let the gnu-compiled 32-bit programs do that. We just jump to
# absolute address 0x00000, in 32-bit protected mode.
#
# Note that the short jump isn't strictly needed, althought there are
# reasons why it might be a good idea. It won't hurt in any case.
#
	#mov	$0x0001, %ax	# protected mode (PE) bit
	#lmsw	%ax		# This is it!
	mov	%cr0, %eax	# get machine status(cr0|MSW)	
	bts	$0, %eax	# turn on the PE-bit 
	mov	%eax, %cr0	# protection enabled
				

	jmp	flush_instr
flush_instr:
	ljmp	$0x10, $0x1000	# jmp offset 1000 of segment 0x10 (cs)
# This routine checks that the keyboard command queue is empty
# (after emptying the output buffers)
#
# No timeout is used - if this hangs there is something wrong with
# the machine, and we probably couldn't proceed anyway.
empty_8042:
	call	delay
	in	$0x64, %al	# 8042 status port
	test	$0x1, %al	# output buffer?
	jz	no_output
	call	delay
	in	$0x60, %al	# read it
	jmp	empty_8042
no_output:
	test	$2, %al		# is input buffer full?
	jnz	empty_8042	# yes - loop
	ret

#
# Read a key and return the (US-)ascii code in al, scan code in ah
#
getkey:
	xor	%ah, %ah
	int	$0x16
	ret

#
# Read a key with a timeout of 30 seconds. The cmos clock is used to get
# the time.
#
getkt:
	call	gettime
	add	$30, %al	# wait 30 seconds
	cmp	$60, %al
	jl	lminute
	sub	$60, %al
lminute:
	mov	%al, %cl
again:	mov	$0x01, %ah
	int	$0x16
	jnz	getkey		# key pressed, so get it
	call	gettime
	cmp	%cl, %al
	jne	again
	mov	$0x20, %al	# timeout, return default char `space'
	ret

#
# Flush the keyboard buffer
#
flush:	mov	$0x01, %ah
	int	$0x16
	jz	empty
	xor	%ah, %ah
	int	$0x16
	jmp	flush
empty:	ret

#
# Read the cmos clock. Return the seconds in al
#
gettime:
	push	%cx
	mov	$0x02, %ah
	int	$0x1a
	mov	%dh, %al	# dh contains the seconds
	and	$0x0f, %al
	mov	%dh, %ah
	mov	$0x04, %cl
	shr	%cl, %ah
	aad
	pop	%cx
	ret

#
# Delay is needed after doing i/o
#
delay:
	.word	0x00eb		# jmp $+2
	ret

# Routine trying to recognize type of SVGA-board present (if any)
# and if it recognize one gives the choices of resolution it offsets.
# If one is found the resolution chosen is given by al,ah (row, cols).

chsvga:
	cld
	push	%ds
	push	%cs
	pop	%ds
	mov	$0xc000, %ax
	mov	%ax, %es
	lea	msg1, %si
	call	prtstr
nokey:
	call	getkey
# I don't care press buttom, so skip this routine.
#	cmp	$0x82, %al
#	jb	nokey
#	cmp	$0xe0, %al
#	ja	nokey
#	cmp	$0x9c, %al
	jmp	svga
	mov	$0x5019, %ax
	pop	%ds
	ret

svga:
	cld
	lea	idati, %si	# Check ATI 'clues'
	mov	$0x31, %di
	mov	$0x09, %cx
	repe
	cmpsb
	jne	noati
	lea	dscati, %si
	lea	moati, %di
	lea	selmod, %cx
	jmp	*%cx

noati:
	mov	$0x200f, %ax	# Check Ahead 'clues'
	mov	$0x3ce, %dx
	out	%ax, %dx
	inc	%dx
	in	%dx, %al
	cmp	$0x20, %al
	je	isahed
	cmp	$0x21, %al
	jne	noahed
isahed:
	lea	dscahead, %si
	lea	moahead, %di
	lea	selmod, %cx
noahed:
	mov	$0x3c3, %dx	# Check Chips & Tech. 'clues'
	in	%dx, %al
	or	$0x10, %al
	out	%al, %dx
	mov	$0x104, %dx
	in	%dx, %al
	mov	%al, %bl
	mov	$0x3c3, %dx
	in	%dx, %al
	and	$0xef, %al
	out	%al, %dx
	cmp	%ds:idcandt, %bl
	jne	nocant
	lea	dsccandt, %si
	lea	mocandt, %di
	lea	selmod, %cx
	jmp	*%cx
nocant:
	mov	%ax, %ax
	mov	%bx, %bx
	mov	$0x3d4, %dx	# Check Cirrus 'clues'
	mov	$0x0c, %al
	out	%al, %dx
	inc	%dx
	in	%dx, %al
	mov	%al, %bl
	xor	%al, %al
	out	%al, %dx
	dec	%dx
	mov	$0x1f, %al
	out	%al, %dx
	inc	%dx
	in	%dx, %al
	mov	%al, %bh
	xor	%ah, %ah
	shl	$4, %al
	mov	%ax, %cx
	mov	%bh, %al
	shr	$4, %al
	add	%ax, %cx
	shl	$8, %cx
	add	$6, %cx
	mov	%cx, %ax
	mov	$0x3c4, %dx
	out	%ax, %dx
	inc	%dx
	in	%dx, %al
	and	%al, %al
	jnz	nocirr
	mov	%bh, %al
	out	%al, %dx
	in	%dx, %al
	cmp	$0x01, %al
	jne	nocirr
	call	rst3d4
	lea	dsccirrus, %si
	lea	mocirrus, %di
	lea	selmod, %cx
rst3d4:
	mov	$0x3d4, %dx
	mov	%bl, %al
	xor	%ah, %ah
	shl	$8, %ax
	add	$0x0c, %ax
	out	%ax, %dx
	ret
nocirr:
	call	rst3d4		# Check Everex 'clues'
	mov	$0x7000, %ax
	xor	%bx, %bx
	int	$0x10
	cmp	$0x70, %al
	jne	noevrx
	shr	$4, %dx
	cmp	$0x678, %dx
	je	istrid
	cmp	$0x236, %dx
	je	istrid
	lea	dsceverex, %si
	lea	moeverex, %di
	lea	selmod, %cx
	jmp	*%cx

istrid:
	lea	ev2tri, %cx
	jmp	*%cx
noevrx:
	lea	idgenoa, %si
	xor	%ax, %ax
	mov	%es:0x37, %al
	mov	%ax, %di
	mov	$0x04, %cx
	dec	%si
	dec	%di
l1:
	inc	%si
	inc	%di
	mov	%ds:(%si), %al
	#seg	%es
	and	%es:(%di), %al
	cmp	%ds:(%si), %al
	loope	l1
	cmp	$0x00, %cx
	jne	nogen
	lea	dscgenoa, %si
	lea	mogenoa, %di
	lea	selmod, %cx
	jmp	*%cx
nogen:
	cld
	lea	idoakvga, %si
	mov	$0x08, %di
	mov	$0x08, %cx
	repe
	cmpsb
	jne	nooak
	lea	dscoakvga, %si
	lea	mooakvga, %di
	lea	selmod, %cx
	jmp	*%cx
nooak:	cld
	lea	idparadise, %si	# Check Paradise 'clues'
	mov	$0x7d, %di
	mov	$0x04, %cx
	repe
	cmpsb
	jne	nopara
	lea	dscparadise, %si
	lea	moparadise, %di
	lea	selmod, %cx
	jmp	*%cx
nopara:
	mov	$0x3c4, %dx	# Check Trident 'clues'
	mov	$0x0e, %al
	out	%al, %dx
	inc	%dx
	in	%dx, %al
	xchg	%al, %ah
	mov	$0x00, %al
	out	%al, %dx
	in	%dx, %al
	xchg	%ah, %al
	mov	%al, %bl	# Strange thing ... in the book this wasn't
	and	$0x02, %bl	# necessary but it worked on my card which
	jz	setb2		# is a trident. Without it the screen goes
	and	$0xfd, %al
	jmp	clrb2
setb2:
	or	$0x02, %al
clrb2:
	out	%al, %dx
	and	$0x0f, %ah
	cmp	$0x02, %ah
	jne	notrid
ev2tri:
	lea	dsctrident, %si
	lea	motrident, %di
	lea	selmod, %cx
	jmp	*%cx
notrid:
	mov	$0x3cd, %dx	# Check Tseng 'clues'
	in	%dx, %al	# Could thing be this simple ! :-)
	mov	%al, %bl
	mov	$0x55, %al
	out	%al, %dx
	in	%dx, %al
	mov	%al, %ah
	mov	%bl, %al
	out	%al, %dx
	cmp	$0x55, %ah
	jne	notsen
	lea	dsctseng, %si
	lea	motseng, %di
	lea	selmod, %cx
	jmp	*%cx
notsen:
	mov	$0x3cc, %dx	# Check Video7 'clues'
	in	%dx, %al
	mov	$0x3b4, %dx
	and	$0x01, %al
	jz	even7
	mov	$0x3d4, %dx
even7:
	mov	$0x0c, %al
	out	%al, %dx
	inc	%dx
	in	%dx, %al
	mov	%al, %bl
	mov	$0x55, %al
	out	%al, %dx
	in	%dx, %al
	dec	%dx
	mov	$0x1f, %al
	out	%al, %dx
	inc	%dx
	in	%dx, %al
	mov	%al, %bh
	out	%al, %dx
	inc	%dx
	mov	%bl, %al
	out	%al, %dx
	mov	$0x55, %al
	mov	$0xea, %al
	cmp	%bh, %al
	jne	novid7
	lea	dscvideo7, %si
	lea	movideo7, %di
selmod:
	push	%si
	lea	msg2, %si
	call	prtstr
	xor	%cx, %cx
	mov	%ds:(%di), %cl
	pop	%si
	push	%si
	push	%cx
tbl:
	pop	%bx
	push	%bx
	mov	%bl, %al
	sub	%cl, %al
	call	dprnt
	call	spcing
	lodsw
	xchg	%ah, %al
	call	dprnt
	xchg	%ah, %al
	push	%ax
	mov	$0x78, %al
	call	prnt1
	pop	%ax
	call	dprnt
	call	docr
	loop	tbl
	pop	%cx
	call	docr
	lea	msg3, %si
	call	prtstr
	pop	%si
	add	$0x80, %cl
nonum:
	call	getkey
	cmp	$0x82, %al
	jb	nonum
	cmp	$0x8b, %al
	je	zero
	cmp	%cl, %al
	ja	nonum
	jmp	nozero
zero:
	sub	$0x0a, %al
nozero:
	sub	$0x80, %al
	dec	%al
	xor	%ah, %ah
	add	%ax, %di
	inc	%di
	push	%ax
	mov	%ds:(%di), %al
	int	$0x10
	pop	%ax
	shl	$1, %ax
	add	%ax, %si
	lodsw
	pop	%ds
	ret
novid7:
	mov	$0x1112, %ax
	mov	$0x0, %bl
	int	$0x10		# use 8x8 font set (50 lines on VGA)

	mov	$0x1200, %ax
	mov	$0x20, %bl
	int	$0x10		# use alternate print screen

	mov	$0x1201, %ax
	mov	$0x34, %bl
	int	$0x10		# turn off cursor emulation

	mov	$0x01, %ah
	mov	$0x0607, %cx
	int	$0x10		# turn on cursor (scan lines 6 to 7)

	pop	%ds
	mov	$0x5032, %ax	# return 80x50
	ret

# Routine that 'tabs' to next col

spcing:
	mov	$0x2e, %al
	call	prnt1
	mov	$0x20, %al
	call	prnt1
	mov	$0x20, %al
	call	prnt1
	mov	$0x20, %al
	call	prnt1
	mov	$0x20, %al
	call	prnt1
	ret

# Routine to print asciiz-string at DS:SI

prtstr:
	lodsb
	and	%al, %al
	jz	fin
	call	prnt1
	jmp	prtstr
fin:
	ret

# Routine to print a decimal value on screen, the value to be
# printed is put in al (i.e 0-255).

dprnt:
	push	%ax
	push	%cx
	mov	$0x00, %ah
	mov	$0x0a, %cl
	idiv	%cl
	cmp	$0x09, %al
	jbe	lt100
	call	dprnt
	jmp	skip10
lt100:
	add	$0x30, %al
	call	prnt1
skip10:
	mov	%ah, %al
	add	$0x30, %al
	call	prnt1
	pop	%cx
	pop	%ax
	ret

# Part of above routine, this one just prints ascii al
prnt1:
	push	%ax
	push	%cx
	mov	$0x00, %bh
	mov	$0x01, %cx
	mov	$0x0e, %ah
	int	$0x10
	pop	%cx
	pop	%ax
	ret

# Prints <CR> + <LF>

docr:
	push	%ax
	push	%cx
	mov	$0x00, %bh
	mov	$0x0e, %ah
	mov	$0x0a, %al
	mov	$0x01, %cx
	int	$0x10
	mov	$0x0d, %al
	int	$0x10
	pop	%cx
	pop	%ax
	ret

msg1:		.ascii	"Press <RETURN> to see SVGA-modes available or any other key to continue."
		.byte	0x0d, 0x0a, 0x0a, 0x00
msg2:		.ascii	"Mode:  COLSxROWS:"
		.byte	0x0d, 0x0a, 0x0a, 0x00
msg3:		.ascii	"Choose mode by pressing the corresponding number."
		.byte	0x0d, 0x0a, 0x00
		
idati:		.ascii	"761295520"
idcandt:	.byte	0xa5
idgenoa:	.byte	0x77, 0x00, 0x66, 0x99
idparadise:	.ascii	"VGA="
idoakvga:	.ascii  "OAK VGA "

# Manufacturer:	  Numofmodes:	Mode:

moati:		.byte	0x02,	0x23, 0x33 
moahead:	.byte	0x05,	0x22, 0x23, 0x24, 0x2f, 0x34
mocandt:	.byte	0x02,	0x60, 0x61
mocirrus:	.byte	0x04,	0x1f, 0x20, 0x22, 0x31
moeverex:	.byte	0x0a,	0x03, 0x04, 0x07, 0x08, 0x0a, 0x0b, 0x16, 0x18, 0x21, 0x40
mogenoa:	.byte	0x0a,	0x58, 0x5a, 0x60, 0x61, 0x62, 0x63, 0x64, 0x72, 0x74, 0x78
moparadise:	.byte	0x02,	0x55, 0x54
motrident:	.byte	0x07,	0x50, 0x51, 0x52, 0x57, 0x58, 0x59, 0x5a
motseng:	.byte	0x05,	0x26, 0x2a, 0x23, 0x24, 0x22
movideo7:	.byte	0x06,	0x40, 0x43, 0x44, 0x41, 0x42, 0x45
mooakvga:	.byte	0x05,	0x00, 0x07, 0x4f, 0x50, 0x51

# msb = Cols lsb = Rows:

dscati:		.word	0x8419, 0x842c
dscahead:	.word	0x842c, 0x8419, 0x841c, 0xa032, 0x5042
dsccandt:	.word	0x8419, 0x8432
dsccirrus:	.word	0x8419, 0x842c, 0x841e, 0x6425
dsceverex:	.word	0x5022, 0x503c, 0x642b, 0x644b, 0x8419, 0x842c, 0x501e, 0x641b, 0xa040, 0x841e
dscgenoa:	.word	0x5020, 0x642a, 0x8419, 0x841d, 0x8420, 0x842c, 0x843c, 0x503c, 0x5042, 0x644b
dscparadise:	.word	0x8419, 0x842b
dsctrident:	.word 	0x501e, 0x502b, 0x503c, 0x8419, 0x841e, 0x842b, 0x843c
dsctseng:	.word	0x503c, 0x6428, 0x8419, 0x841c, 0x842c
dscvideo7:	.word	0x502b, 0x503c, 0x643c, 0x8419, 0x842c, 0x841c
dscoakvga:	.word	0x2819, 0x5019, 0x843c, 0x8419, 0x842C

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
	
.text
endtext:
.data
enddata:
.bss
endbss:
