#ifndef _ASM_SYSTEM_H_
#define _ASM_SYSTEM_H_

#define sti()  __asm__("sti"::)
#define cli()  __asm__("cli"::)
#define nop()  __asm__("nop"::)

#define iret() __asm__("iret"::)

#endif

