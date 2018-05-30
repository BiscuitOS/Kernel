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
#ifdef CONFIG_ELF_PRIVATE_SECTION_FUNC
	__bis_text = .;
	.bis.text : {
		*(.bis.text)
	}
	__bis_etext = .;
#endif
#ifdef CONFIG_ELF_PRIVATE_SECTION_DATA
	.bis.data : {
		*(.bis.data)
	}
#endif
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
#ifdef CONFIG_ELF_PRIVATE_SECTION_FUNC
EXTERN(__bis_text)
EXTERN(__bis_etext)
#endif
