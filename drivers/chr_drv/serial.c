
#include <linux/tty.h>
#include <asm/io.h>
#include <asm/system.h>

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
