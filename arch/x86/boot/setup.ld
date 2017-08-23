/*
 * setup.ld
 *
 * Linker script for the i386 setup code.
 */
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 0x0000;
	.bs_text	: { *(.bs_text) }
	
	. = 0x0200;
	.st_text	: { *(.st_text) }
	
	. = 0x0A00;
	__end_init = .;
}
