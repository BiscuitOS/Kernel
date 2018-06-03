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
	PROVIDE(debugcall_start = .);
	.debugcallnull.debug : {
		*(.debugcallnull.debug)
	}
	PROVIDE(debugcallearly_start = .);
        .debugcallearly.debug : { 
                *(.debugcallearly.debug) 
        } 
        PROVIDE(debugcall0_start = .); 
        .debugcall0.debug : { 
                *(.debugcall0.debug) 
        } 
        PROVIDE(debugcall1_start = .); 
        .debugcall1.debug : { 
                *(.debugcall1.debug) 
        } 
        PROVIDE(debugcall1s_start = .); 
        .debugcall1s.debug : { 
                *(.debugcall1s.debug) 
        } 
        PROVIDE(debugcall2_start = .); 
        .debugcall2.debug : { 
                *(.debugcall2.debug) 
        } 
        PROVIDE(debugcall2s_start = .); 
        .debugcall2s.debug : { 
                *(.debugcall2s.debug) 
        } 
        PROVIDE(debugcall3_start = .); 
        .debugcall3.debug : { 
                *(.debugcall3.debug) 
        } 
        PROVIDE(debugcall3s_start = .); 
        .debugcall3s.debug : { 
                *(.debugcall3s.debug) 
        } 
        PROVIDE(debugcall4_start = .); 
        .debugcall4.debug : { 
                *(.debugcall4.debug) 
        } 
        PROVIDE(debugcall4s_start = .); 
        .debugcall4s.debug : { 
                *(.debugcall4s.debug) 
        } 
        PROVIDE(debugcall5_start = .); 
        .debugcall5.debug : { 
                *(.debugcall5.debug) 
        } 
        PROVIDE(debugcall5s_start = .); 
        .debugcall5s.debug : { 
                *(.debugcall5s.debug) 
        } 
        PROVIDE(debugcallrootfs_start = .); 
        .debugcallrootfs.debug : { 
                *(.debugcallrootfs.debug) 
        } 
        PROVIDE(debugcall6_start = .); 
        .debugcall6.debug : { 
                *(.debugcall6.debug) 
        } 
        PROVIDE(debugcall6s_start = .); 
        .debugcall6s.debug : { 
                *(.debugcall6s.debug) 
        } 
        PROVIDE(debugcall7_start = .); 
        .debugcall7.debug : { 
                *(.debugcall7.debug) 
        } 
        PROVIDE(debugcall7s_start = .); 
        .debugcall7s.debug : { 
                *(.debugcall7s.debug) 
        } 
	PROVIDE(.debugcall_end = .);
#endif
#ifdef CONFIG_ELF_PRIVATE_SECTION_FUNC
	__bis_text = .;
	.bis.text : {
		*(.bis.text)
	}
	__bis_etext = .;
#endif
#ifdef CONFIG_ELF_SPECIAL_SYMBOL
	__etext = .;
	_etext  = .;
	etext   = .;
#endif
	.data	: { 
		*(.data) 
	}
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
