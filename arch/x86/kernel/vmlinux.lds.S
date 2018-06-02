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
#ifdef CONFIG_ELF_SPECIAL_SYMBOL
	__executable_start = .;
#endif
	.text	: {
		*(.text) 
	}
#ifdef CONFIG_DEBUG_DEBUGCALL
	.debugcallearly.debug : {
		*(.debugcallearly.debug)
	}
	.debugcall0.debug : {
		*(.debugcall0.debug)
	}
	.debugcall1.debug : {
		*(.debugcall1.debug)
	}
	.debugcall1s.debug : {
		*(.debugcall1s.debug)
	}
	.debugcall2.debug : {
		*(.debugcall2.debug)
	}
	.debugcall2s.debug : {
		*(.debugcall2s.debug)
	}
	.debugcall3.debug : {
		*(.debugcall3.debug)
	}
	.debugcall3s.debug : {
		*(.debugcall3s.debug)
	}
	.debugcall4.debug : {
		*(.debugcall4.debug)
	}
	.debugcall4s.debug : {
		*(.debugcall4s.debug)
	}
	.debugcall5.debug : {
		*(.debugcall5.debug)
	}
	.debugcall5s.debug : {
		*(.debugcall5s.debug)
	}
	.debugcallrootfs.debug : {
		*(.debugcallrootfs.debug)
	}
	.debugcall6.debug : {
		*(.debugcall6.debug)
	}
	.debugcall6s.debug : {
		*(.debugcall0.debug)
	}
	.debugcall7.debug : {
		*(.debugcall0.debug)
	}
	.debugcall7s.debug : {
		*(.debugcall0.debug)
	}
#endif
	.data	: { 
		*(.data) 
	}
#ifdef CONFIG_ELF_SPECIAL_SYMBOL
	__etext = .;
	_etext  = .;
	etext   = .;
#endif
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
#ifdef CONFIG_ELF_SPECIAL_SYMBOL
	_edata = .;
	edata  = .;
#endif
	__edata = .;
	.bss	: { 
		*(.bss) 
	}
	__end = .;
#ifdef CONFIG_ELF_SPECIAL_SYMBOL
	_end  = .;
#endif
	end   = .;
}
EXTERN(end)
EXTERN(__end)
EXTERN(__edata)
#ifdef CONFIG_ELF_PRIVATE_SECTION_FUNC
EXTERN(__bis_text)
EXTERN(__bis_etext)
#endif
#ifdef CONFIG_ELF_SPECIAL_SYMBOL
EXTERN(__executable_start)
EXTERN(__etext)
EXTERN(_etext)
EXTERN(etext)
EXTERN(_edata)
EXTERN(edata)
EXTERN(_end)
#endif
