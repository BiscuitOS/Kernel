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
	__kernel_debug_func = .;

	__kernel_debug_func_end = .;
	.data	: { 
		*(.data) 
	}
	__edata = .;
	.bss	: { 
		*(.bss) 
	}
	__end = .;
	end = .;
}
EXTERN(end)
EXTERN(__end)
EXTERN(__edata)
