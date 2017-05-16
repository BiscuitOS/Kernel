
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/system.h>
#include <linux/head.h>

extern void rs1_interrupt(void);
extern void rs2_interrupt(void);
/*
 * This routine gets called when tty_write has put something into the 
 * write_queu. It must check wheter the queue is empty, and set the 
 * the interrupt register accordingly.
 */
void rs_write(struct tty_struct *tty)
{
	cli();
	if (!EMPTY(tty->write_q))
		outb(inb_p(tty->write_q.data + 1) | 0x02, tty->write_q.data + 1);
	sti();
}

static void init(int port)
{
	outb_p(0x80, port + 3);
	outb_p(0x30, port);
	outb_p(0x00, port + 1);
	outb_p(0x03, port + 3);
	outb_p(0x0b, port + 4);
	outb_p(0x0d, port + 1);
	(void)inb(port);
}

void rs_init(void)
{
	set_intr_gate(0x24, rs1_interrupt);
	set_intr_gate(0x23, rs2_interrupt);
	init(tty_table[1].read_q.data);
	init(tty_table[2].read_q.data);
	outb(inb_p(0x21) & 0xE7, 0x21);
}
