#ifndef _FS_H_
#define _FS_H_

#include <sys/types.h>

#define NR_OPEN 20

#define NR_HASH    307
#define BLOCK_SIZE 1024
#define KB_SHIFT       10
#define MB_SHIFT       (KB_SHIFT << 1)

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define READ       0
#define WRITE      1
#define READA      2		/* read-ahead - don't pause */
#define WRITEA     3		/* "write-ahead" - silly, but somewhat useful */

struct buffer_head {
	char *b_data;
	unsigned long b_blocknr;
	unsigned short b_dev;
	unsigned char b_uptodate;
	unsigned char b_dirt;
	unsigned char b_count;
	unsigned char b_lock;
	struct task_struct *b_wait;
	struct buffer_head *b_prev;
	struct buffer_head *b_next;
	struct buffer_head *b_prev_free;
	struct buffer_head *b_next_free;
};

struct vfs_inode {
	unsigned short i_mode;
	unsigned short i_uid;
	unsigned long i_size;
	unsigned long i_mtime;
	unsigned char i_gid;
	unsigned char i_nlinks;
	unsigned short i_zone[9];
	/* These are in memory also */
	struct task_struct *i_wait;
	unsigned long i_atime;
	unsigned long i_ctime;
	unsigned long i_dev;
	unsigned long i_num;
	unsigned long i_count;
	unsigned long i_lock;
	unsigned long i_dirt;
	unsigned long i_pipe;
	unsigned long i_mount;
	unsigned long i_seek;
	unsigned long i_update;
};

struct file {
	unsigned short f_mode;
	unsigned short f_flags;
	unsigned short f_count;
	struct vfs_inode *f_inode;
	off_t f_pos;
};

#define MAJOR(a)        (((unsigned)(a)) >> 8)
#define MINOR(a)        ((a)&0xff)

void buffer_init(long);

extern int ROOT_DEV;
#endif
