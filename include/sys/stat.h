#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/types.h>

struct stat {
    dev_t    st_dev;
    ino_t    st_ino;
    umode_t  st_mode;
    nlink_t  st_nlink;
    uid_t    st_uid;
    gid_t    st_gid;
    dev_t    st_rdev;
    off_t    st_size;
    time_t   st_atime;
    time_t   st_mtime;
    time_t   st_ctime;
};

#define S_IFMT           0170000
#define S_IFSOCK         0140000    /* Socket */
#define S_IFLNK          0120000    /* Symbol link */
#define S_IFREG          0100000    /* Regular file */
#define S_IFBLK          0060000    /* Block device */
#define S_IFDIR          0040000    /* Directory */
#define S_IFCHR          0020000    /* Character device */
#define S_IFIFO          0010000    /* FIFO/Named pipe */
#define S_ISUID          0004000    /* Set user id upon execution */
#define S_ISGID          0002000    /* Set group id upon execution */
#define S_ISVTX          0001000    /* Sticky bit */

#define S_ISREG(m)       (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)       (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)       (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)       (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)      (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)      (((m) & S_IFMT) == S_IFSOCK)
#define S_ISLNK(m)       (((m) & S_IFMT) == S_IFLNK)

#define S_IRWXU  00700
#define S_IRUSR  00400   /* User readable */
#define S_IWUSR  00200   /* User writable */
#define S_IXUSR  00100   /* User executable */

#define S_IRWXG  00070
#define S_IRGRP  00040   /* Group readable */
#define S_IWGRP  00020   /* Group writable */
#define S_IXGRP  00010   /* Group executable */

#define S_IRWXO  00007
#define S_IROTH  00004   /* World/Other readable */
#define S_IWOTH  00002   /* World/Other writable */
#define S_IXOTH  00001   /* World/Other executable */

extern int fstat(int fildes, struct stat *stat_buf);

#endif
