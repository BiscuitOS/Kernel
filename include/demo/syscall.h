#ifndef _DEBUG_SYSCALL_H
#define _DEBUG_SYSCALL_H

/*
 * Import content into include/linux/unistd.h
 */
#ifdef __FILE_DEBUGCALL_NR__
/*
 * This file contains the system call numbers and the syscallX
 * macros
 */
#define DEBUG_SYSCALL_NR          135

#define __NR_demo_setup           (DEBUG_SYSCALL_NR + 0)
#define __NR_demo_open            (DEBUG_SYSCALL_NR + 1)
#define __NR_vfs_namei            (DEBUG_SYSCALL_NR + 2)
#define __NR_vfs_inode            (DEBUG_SYSCALL_NR + 3)
#define __NR_demo_read            (DEBUG_SYSCALL_NR + 4)
#define __NR_vfs_buffer           (DEBUG_SYSCALL_NR + 5)
#define __NR_demo_write           (DEBUG_SYSCALL_NR + 6)
#define __NR_demo_minixfs         (DEBUG_SYSCALL_NR + 7)
#define __NR_demo_syscall         (DEBUG_SYSCALL_NR + 8)
#define __NR_demo_close           (DEBUG_SYSCALL_NR + 9)
#define __NR_demo_fork            (DEBUG_SYSCALL_NR + 10)
#define __NR_demo_creat           (DEBUG_SYSCALL_NR + 11)
#define __NR_demo_chdir           (DEBUG_SYSCALL_NR + 12)
#define __NR_demo_exit            (DEBUG_SYSCALL_NR + 13)
#define __NR_vfs_ext2fs           (DEBUG_SYSCALL_NR + 14)
#define __NR_demo_paging          (DEBUG_SYSCALL_NR + 15)
#define __NR_demo_pgt_entence     (DEBUG_SYSCALL_NR + 16)

int demo_setup(void *BIOS);
int demo_open(const char *filename, int flag, int mode);
int vfs_namei(const char *name, int flag, int mode);
int vfs_inode(const char *filename);
int demo_read(unsigned int fd, char *buf, unsigned int count);
int vfs_buffer(int fd);
int demo_write(unsigned int fd, char *buf, unsigned int count);
int demo_minixfs(int fd);
int demo_syscall(const char *filename, int flag, int mode);
int demo_close(unsigned int fd);
int demo_fork(void);
int demo_creat(const char *filename, int mode);
int demo_chdir(const char *filename);
int demo_exit(int exit_code);
int vfs_ext2fs(int fd);
int demo_paging(unsigned long linear);
int demo_pgt_entence(void);

#endif // __FILE_DEBUGCALL_NR__

/*
 * Import content into include/linux/sys.h
 */
#ifdef __FILE_SYS_DEBUGCALL__

static inline int sys_null(void)
{
    return 0;
}

#ifdef CONFIG_DEBUG_POSIX_SETUP
extern int sys_demo_setup();
#else
#define sys_demo_setup sys_null
#endif

#ifdef CONFIG_DEBUG_POSIX_OPEN
extern int sys_demo_open();
#else
#define sys_demo_open sys_null
#endif

#ifdef CONFIG_DEBUG_VFS_NAMEI
extern int sys_vfs_namei();
#else
#define sys_vfs_namei sys_null
#endif

#ifdef CONFIG_DEBUG_VFS_INODE
extern int sys_vfs_inode();
#else
#define sys_vfs_inode sys_null
#endif

#ifdef CONFIG_DEBUG_POSIX_READ
extern int sys_demo_read();
#else
#define sys_demo_read sys_null
#endif

#ifdef CONFIG_DEBUG_VFS_BUFFER
extern int sys_vfs_buffer();
#else
#define sys_vfs_buffer sys_null
#endif

#ifdef CONFIG_DEBUG_POSIX_WRITE
extern int sys_demo_write();
#else
#define sys_demo_write sys_null
#endif

#ifdef CONFIG_DEBUG_VFS_MINIXFS
extern int sys_demo_minixfs();
#else
#define sys_demo_minixfs sys_null
#endif

#ifdef CONFIG_DEBUG_SYSCALL
extern int sys_demo_syscall();
#else
#define sys_demo_syscall sys_null
#endif

#ifdef CONFIG_DEBUG_POSIX_CLOSE
extern int sys_demo_close();
#else
#define sys_demo_close sys_null
#endif

#ifdef CONFIG_DEBUG_POSIX_FORK
extern int sys_demo_fork();
#else
#define sys_demo_fork sys_null
#endif

#ifdef CONFIG_DEBUG_POSIX_CREAT
extern int sys_demo_creat();
#else
#define sys_demo_creat sys_null
#endif

#ifdef CONFIG_DEBUG_POSIX_CHDIR
extern int sys_demo_chdir();
#else
#define sys_demo_chdir sys_null
#endif

#ifdef CONFIG_DEBUG_POSIX_EXIT
extern int sys_demo_exit();
#else
#define sys_demo_exit sys_null
#endif

#ifdef CONFIG_DEBUG_VFS_EXT2
extern int sys_vfs_ext2fs();
#else
#define sys_vfs_ext2fs sys_null
#endif

#ifdef CONFIG_DEBUG_32BIT_PAGING_USER
extern int sys_demo_paging();
#else
#define sys_demo_paging sys_null
#endif

#if defined CONFIG_DEBUG_PAGE_TABLE_CLEAR | \
    defined CONFIG_DEBUG_PAGE_TABLE_FREE
extern int sys_demo_pgt_entence();
#else
#define sys_demo_pgt_entence sys_null
#endif

#endif // __FILE_SYS_DEBUGCALL__

#endif
