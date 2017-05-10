#ifndef _FS_H_
#define _FS_H_

#include <sys/types.h>

#define NR_OPEN 20

struct m_inode {
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
	struct m_inode *f_inode;
	off_t f_pos;
};


extern int ROOT_DEV;
#endif
