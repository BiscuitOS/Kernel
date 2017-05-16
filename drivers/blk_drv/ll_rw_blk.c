#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/blk.h>


/*
 * The request-struct contains all necessary data
 * to load a nr of sectors into memory.
 */
struct request request[NR_REQUEST];

/*
 * blk_dev_struct is:
 * do_request-address
 * next->request
 */
struct blk_dev_struct blk_dev[NR_BLK_DEV] = {
	{ NULL, NULL},    /* no_dev */	
	{ NULL, NULL},	  /* dev mem */
	{ NULL, NULL},	  /* dev fd */
	{ NULL, NULL},	  /* dev hd */
	{ NULL, NULL},	  /* dev ttyx */
	{ NULL, NULL},	  /* dev tty */
	{ NULL, NULL},	  /* dev lp */
};

/*
 * used to wait on when there are no free request.
 */
struct task_struct *wait_for_request = NULL;

void blk_dev_init(void)
{
	int i;

	for (i = 0; i < NR_REQUEST; i++) {
		request[i].dev = -1;
		request[i].next = NULL;
	}
}
