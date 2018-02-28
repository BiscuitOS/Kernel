#ifndef _DEBUG_BLOCK_H
#define _DEBUG_BLOCK_H

#ifdef CONFIG_DEBUG_BLOCK_DEV

/* This structure define the HD's and their types */
struct hd_i_struct
{
    int head;
    int sect;
    int cyl;
    int wpcom;
    int lzone;
    int ctl;
};

struct hd_struct
{
    long start_sect;
    long nr_sects;
};

extern int debug_block_common(void);
extern struct buffer_head *HD_bread(int dev, int block);

#ifdef CONFIG_DEBUG_BLOCK_HD

#define MAX_HD       2

extern int debug_block_hd_common(void);

#ifdef CONFIG_DEBUG_BLOCK_HD_DEV
extern struct hd_struct hd2[5 * MAX_HD];
extern struct hd_i_struct hd2_info[];
extern int debug_hd_dev_common(void);
extern int NR_HD;
extern void HD_out(unsigned int driver, unsigned int nsect,
                          unsigned int sect, unsigned int head,
                          unsigned int cyl, unsigned int cmd,
                          void (*intr_addr)(void));
extern void HD_reset_controller(void);
#endif

#ifdef CONFIG_DEBUG_BLOCK_HD_INT
extern int debug_hd_interrupt_common(void);

extern void HD_recal_intr(void);
extern void HD_do_request(void);
#endif

#ifdef CONFIG_DEBUG_BLOCK_HD_USAGE
extern int debug_block_usage_common(void);
#endif

#endif // CONFIG_DEBUG_BLOCK_HD

#ifdef CONFIG_DEBUG_DISK_CHS
extern int debug_block_CHS_common(void);
extern int debug_block_disk_CHS_common(void);
#endif // CONFIG_DEBUG_BLOCK_HD

#ifdef CONFIG_DEBUG_DISK_PARTITION
extern int debug_block_partition_common(void);

#ifdef CONFIG_DEBUG_PARTITION_MBR
extern int debug_block_disk_MBR_common(void);
#endif

#endif // CONFIG_DEBUG_DISK_PARTITION

/* Userland API */
extern int debug_vfs_common_userland(void);

#endif // CONFIG_DEBUG_BLOCK_DEV

#endif
