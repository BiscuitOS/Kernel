
.text
.globl keyboard_interrupt
/*
 * con_int is the real interrupt rountine that reads the 
 * keyboard scan-code and converts it into the appropriate
 * ascii chararcter(s).
 */
keyboard_interrupt:
	iret
