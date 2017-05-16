#ifndef _ASM_SYSTEM_H_
#define _ASM_SYSTEM_H_

#define sti()  __asm__("sti"::)
#define cli()  __asm__("cli"::)
#define nop()  __asm__("nop"::)

#define iret() __asm__("iret"::)

#define _set_gate(gate_addr, type, dpl, addr)  \
	__asm__ ("movw %%dx, %%ax\n\t"             \
			 "movw %0, %%dx\n\t"               \
			 "movl %%eax, %1\n\t"              \
			 "movl %%edx, %2"              \
			 :                                 \
			 : "i" ((short) (0x8000 + (dpl << 13) + (type << 8))), \
			 "o" (*((char *) (gate_addr))),    \
			 "o" (*(4+(char *) (gate_addr))),  \
			 "d" ((char *) (addr)), "a" (0x00080000))

#define set_trap_gate(n, addr)  \
	_set_gate(&idt[n], 15, 0, addr)

#define set_intr_gate(n, addr)  \
	_set_gate(&idt[n], 15, 0, addr)

#define set_system_gate(n, addr) \
	_set_gate(&idt[n], 15, 3, addr)


#define _set_tssldt_desc(n, addr, type)  \
	__asm__("movw $104, %1\n\t"          \
			"movw %%ax, %2\n\t"          \
			"rorl $16, %%eax\n\t"        \
			"movb %%al, %3\n\t"          \
			"movb $" type ", %4\n\t"     \
			"movb $0x00, %5\n\t"         \
			"movb %%ah, %6\n\t"          \
			"rorl $16, %%eax"            \
			:: "a" (addr), "m" (*(n)), "m" (*(n + 2)), "m" (*(n + 4)), \
			"m" (*(n + 5)), "m" (*(n + 6)), "m" (*(n + 7))  \
			)

#define set_tss_desc(n, addr) \
	_set_tssldt_desc(((char *) (n)), ((int)(addr)), "0x89")

#define set_ldt_desc(n, addr) \
	_set_tssldt_desc(((char *) (n)), ((int)(addr)), "0x82")

#endif

