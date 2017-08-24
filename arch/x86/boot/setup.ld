/*
 * setup.ld
 *
 * Linker script for the i386 setup code.
 */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(bootsect_start)

SECTIONS
{
	. = 0;
	.bs_text 	: { *(.bs_text) }
	. = 0x0200;
	.st_text	: { *(.st_text) }
	
	. = 0x0A00;
	.text		: { *(.text) }
	.data		: { *(.data) }
	.bss		: { *(.bss) }
}
