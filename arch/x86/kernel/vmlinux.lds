/*
 * vmlinux.ld
 *
 * Linker script for the i386 vmlinux.
 */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(startup_32)

SECTIONS
{
	. = 0x00;
	. = ALIGN(4);
	.text	: {
		*(.text) 
		PROVIDE (end = .);
	}
	. = ALIGN(4);
	.data	: { 
		*(.data) 
		PROVIDE (__edata = .);
	}
	.bss	: { 
		*(.bss) 
	}
	PROVIDE (__end = .);
}
