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
	/* Linux0.11 bootloader*/
	. = 0;
	.bs_text 	: { *(.bs_text) }
	. = 0x0200;
	.st_text	: { *(.st_text) }
	
	/* System segment */
	. = 0x0A00;
	.text		: { *(.text) }
	.data		: { *(.data) }
	.bss		: { *(.bss) }
	PROVIDE (end = .);
}
