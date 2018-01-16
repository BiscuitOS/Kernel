#ifndef _FS_H_
#define _FS_H_

#include <linux/types.h>
#include <sys/types.h>

/*
 * devices are as follows: (same as minix, so we can use the minix
 * file system. These are major numbers.)
 *
 * 0 - unused (nodev)
 * 1 - /dev/mem
 * 2 - /dev/fd
 * 3 - /dev/hd
 * 4 - /dev/ttyx
 * 5 - /dev/tty
 * 6 - /dev/lp
 * 7 - unnamed pipes
 */

#define IS_SEEKABLE(x) ((x) >=1 && (x)<=3)

#define READ       0
#define WRITE      1
#define READA      2   /* read-ahead - don't pause */
#define WRITEA     3   /* "write-ahead" -silly, but somewhat useful */

#define NR_OPEN    20
#define NR_SUPER   8
#define NR_INODE   32
#define NR_FILE    64
#define NR_HASH    307
#define NR_BUFFERS nr_buffers

#define BLOCK_SIZE 1024
#define KB_SHIFT       10
#define MB_SHIFT       20

#define NAME_LEN  14
#define ROOT_INO  1

#ifndef NULL
#define NULL ((void *) 0)
#endif

#define I_MAP_SLOTS   8
#define Z_MAP_SLOTS   8
/* 
 * Magic for MINIX 
 *  Legacy minix: 0x137f
 *  MINIX V1:     0x138f
 */
#define SUPER_MINIX_MAGIC      0x137F
#define SUPER_MINIX_MAGIC_V1   0x138f

#define BLOCK_SIZE_BITS 10

#define PIPE_HEAD(inode) ((inode).i_zone[0])
#define PIPE_TAIL(inode) ((inode).i_zone[1])
#define PIPE_SIZE(inode) ((PIPE_HEAD(inode)-PIPE_TAIL(inode))&(PAGE_SIZE-1))
#define PIPE_EMPTY(inode) (PIPE_HEAD(inode) == PIPE_TAIL(inode))
#define PIPE_FULL(inode) (PIPE_SIZE(inode)==(PAGE_SIZE - 1))
#define INC_PIPE(head) \
__asm__("incl %0\n\tandl $4095,%0"::"m" (head))

#define INODES_PER_BLOCK ((BLOCK_SIZE) / (sizeof(struct d_inode)))
#define DIR_ENTRIES_PER_BLOCK ((BLOCK_SIZE) / sizeof(struct dir_entry))

struct buffer_head {
    char *b_data;                 /* pointer to data block (1024 bytes) */
    unsigned long   b_blocknr;    /* block number */
    unsigned short  b_dev;        /* device (0 = free) */
    unsigned char   b_uptodate;
    unsigned char   b_dirt;       /* 0-clean, 1-dirty */
    unsigned char   b_count;      /* users using this block */
    unsigned char   b_lock;       /* 0 - ok, 1 - locked */
    struct task_struct *b_wait;
    struct buffer_head *b_prev;
    struct buffer_head *b_next;
    struct buffer_head *b_prev_free;
    struct buffer_head *b_next_free;
};

struct d_inode {
    unsigned short i_mode;
    unsigned short i_uid;
    unsigned long  i_size;
    unsigned long  i_time;
    unsigned char  i_gid;
    unsigned char  i_nlinks;
    unsigned short i_zone[9];
};

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
	unsigned long  i_atime;
	unsigned long  i_ctime;
	unsigned short i_dev;
	unsigned short i_num;
	unsigned short i_count;
	unsigned char  i_lock;
	unsigned char  i_dirt;
	unsigned char  i_pipe;
	unsigned char  i_mount;
	unsigned char  i_seek;
	unsigned char  i_update;
};

struct file {
	unsigned short f_mode;
	unsigned short f_flags;
	unsigned short f_count;
	struct m_inode *f_inode;
	off_t f_pos;
};

struct super_block {
    unsigned short s_ninodes;
    unsigned short s_nzones;
    unsigned short s_imap_blocks;
    unsigned short s_zmap_blocks;
    unsigned short s_firstdatazone;
    unsigned short s_log_zone_size;
    unsigned long  s_max_size;
    unsigned short s_magic;
    /* There are only in memory */
    struct buffer_head *s_imap[8];
    struct buffer_head *s_zmap[8];
    unsigned short s_dev;
    struct m_inode *s_isup;
    struct m_inode *s_imount;
    unsigned long s_time;
    struct task_struct *s_wait;
    unsigned char s_lock;
    unsigned char s_rd_only;
    unsigned char s_dirt;
};

struct d_super_block {
    unsigned short s_ninodes;
    unsigned short s_nzones;
    unsigned short s_imap_blocks;
    unsigned short s_zmap_blocks;
    unsigned short s_firstdatazone;
    unsigned short s_log_zone_size;
    unsigned long  s_max_size;
    unsigned short s_magic;
};

struct dir_entry {
    unsigned short inode;
    char name[NAME_LEN];
};

#define MAJOR(a)        (((unsigned)(a)) >> 8)
#define MINOR(a)        ((a)&0xff)

void buffer_init(long);
extern void ll_rw_block(int rw, struct buffer_head *bh);
extern int ROOT_DEV;
extern void iput(struct m_inode *inode);
extern void sync_inodes(void);
extern int sync_dev(int dev);
extern void truncate(struct m_inode *inode);
extern void free_inode(struct m_inode *inode);
extern struct super_block *get_super(int dev);
extern struct buffer_head *bread(int dev, int block);
extern void brelse(struct buffer_head *buf);
extern void free_block(int dev, int block);
extern struct buffer_head *get_hash_table(int dev, int block);
extern struct buffer_head *getblk(int dev, int block);
extern struct buffer_head *breada(int dev, int first, ...);
extern void invalidate_inodes(int);
extern int floppy_change(unsigned int nr);
extern struct m_inode * iget(int dev,int nr);
extern void check_disk_change(int dev);
extern void mount_root(void);
extern int new_block(int dev);
extern int bmap(struct m_inode * inode,int block);
extern int create_block(struct m_inode *inode, int block);
extern int file_write(struct m_inode *inode, struct file *filp, char *buf, int count);
extern int block_write(int dev, long *pos, char *buf, int count);
extern int write_pipe(struct m_inode *inode, char *buf, int count);
extern struct m_inode *new_inode(int dev);
extern struct m_inode * get_empty_inode(void);
extern int open_namei(const char *pathname, int flag, int mode,
               struct m_inode **res_inode);
extern struct m_inode *namei(const char *pathname);
extern struct m_inode *get_pipe_inode(void);
extern void bread_page(unsigned long address, int dev, int b[4]);

extern struct file file_table[NR_FILE];
extern struct super_block super_block[NR_SUPER];
extern struct m_inode inode_table[NR_INODE];
extern int nr_buffers;
#endif
