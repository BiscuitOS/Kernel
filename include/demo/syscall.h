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

int demo_setup(void *BIOS);
int demo_open(const char *filename, int flag, int mode);
int vfs_namei(const char *name, int flag, int mode);
int vfs_inode(const char *filename);
int demo_read(unsigned int fd, char *buf, unsigned int count);
int vfs_buffer(int fd);
int demo_write(unsigned int fd, char *buf, unsigned int count);

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

#endif // __FILE_SYS_DEBUGCALL__

#endif
