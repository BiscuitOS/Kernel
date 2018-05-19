/*
 * vmlinux.lds.S
 *
 * Linker script for the i386 vmlinux.
 */
#include <linux/config.h>

OUTPUT_FORMAT(CONFIG_OUTPUT_FORMAT, CONFIG_OUTPUT_FORMAT, CONFIG_OUTPUT_FORMAT)
OUTPUT_ARCH(i386)
ENTRY(startup_32)

SECTIONS
{
	. = 0x00;
	.text	: {
		*(.text) 
	}
	.data	: { 
		*(.data) 
	}
	PROVIDE (__edata = .);
	.bss	: { 
		*(.bss) 
	}
	PROVIDE (end = .);
	PROVIDE (__end = .);
}
