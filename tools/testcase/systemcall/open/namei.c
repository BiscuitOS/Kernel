/*
 * namei.c
 *
 * (C) 2018.03 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>

#include <test/debug.h>

#include <asm/segment.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <const.h>

#define MAY_EXEC   1
#define MAY_WRITE  2
#define MAY_READ   4


#define ACC_MODE(x) ("\004\002\006\377"[(x)&O_ACCMODE])

extern int d_map(struct m_inode *inode, int block);
extern int d_iput(struct m_inode *inode);
extern int d_create_block(struct m_inode *inode, int block);

/*
 * permission()
 *
 * is used to check for read/write/execute permission on a file.
 */
static int permission(struct m_inode *inode, int mask)
{
    int mode = inode->i_mode;

    /* Special case: not even root can read/write a delete file */
    if (inode->i_dev && !inode->i_nlinks)
        return 0;
    else if (current->euid == inode->i_uid)
        mode >>= 6;
    else if (current->egid == inode->i_gid)
        mode >>= 3;
    if (((mode & mask & 0007) == mask) || suser())
        return 1;
    return 0;
}

/*
 * ok, we cannot use strncmp, as the name is not in our data space.
 * Thus we'll have to use match. No big problem. Match also makes
 * some sanity tests.
 *
 * NOTE! unlike strncmp, match returns 1 for success, 0 for failure.
 */
static int match(int len, const char *name, struct dir_entry *de)
{
    register int same;

    if (!de || !de->inode || len > NAME_LEN)
        return 0;
    if (len < NAME_LEN && de->name[len])
        return 0;
    __asm__("cld\n\t"
            "fs; repe; cmpsb\n\t"
            "setz %%al"
            : "=a" (same)
            : "0" (0), "S" ((long) name), "D" ((long) de->name),
              "c" (len));
    return same;
}

/*
 * find_entry()
 *
 * finds an entry in the specified directory with the wanted name. It
 * returns the cache buffer in which the entry wa found, and the entry
 * itself (as a parameter - res_dir). It does not read the inode of the 
 * entry - you'll have to do that yourself if you want to.
 *
 * This also takes care of the few special cases due to '..' - traversal
 * over a pseudo-root and a mount point.
 */
static struct buffer_head *find_entry(struct m_inode **dir, 
              const char *name, int namelen, struct dir_entry **res_dir)
{
    int entries, i;
    int block;
    struct super_block *sb;
    struct buffer_head *bh;
    struct dir_entry *de;

#ifdef NO_TRUNCATE
    if (namelen > NAME_LEN)
        return NULL;
#else
    if (namelen > NAME_LEN)
        namelen = NAME_LEN;
#endif

    entries = (*dir)->i_size / (sizeof (struct dir_entry));
    *res_dir = NULL;
    if (!namelen)
        return NULL;
    /* check for '..', as we might have to do some 'magic' for it */
    if (namelen == 2 && get_fs_byte(name) == '.' &&
        get_fs_byte(name + 1) == '.') {
        /* '..' in a pseudo-root results in a faked '.' 
           (just change namelen) */
        if ((*dir) == current->root)
            namelen = 1;
        else if ((*dir)->i_num == ROOT_INO) {
            /* '..' over a mount-point result in 'dir' being exchange
               for the mount */
            sb = get_super((*dir)->i_dev);
            if (sb->s_imount) {
                d_iput(*dir);
                (*dir) = sb->s_imount;
                (*dir)->i_count++;
            }
        }
    }
    if (!(block = (*dir)->i_zone[0]))
        return NULL;
    if (!(bh = bread((*dir)->i_dev, block)))
        return NULL;
    i = 0;
    de = (struct dir_entry *)bh->b_data;
    while (i < entries) {
        /* Load new block while the dentry not find on current block. */
        if ((char *)de >= BLOCK_SIZE + bh->b_data) {
            brelse(bh);
            bh = NULL;
            if (!(block = d_map(*dir, i / DIR_ENTRIES_PER_BLOCK)) ||
                !(bh = bread((*dir)->i_dev, block))) {
                i += DIR_ENTRIES_PER_BLOCK;
                continue;
            }
            de = (struct dir_entry *)bh->b_data;
        }
        if (match(namelen, name, de)) {
            *res_dir = de;
            return bh;
        }
        de++;
        i++;
    }
    brelse(bh);
    return NULL;
}

/*
 * get_dir()
 *
 *   Getdir traverses the pathname until it hits the topmost directory.
 *   It returns NULL on failure.
 */
static struct m_inode *get_dir(const char *pathname)
{
    char c;
    const char *thisname;
    struct m_inode *inode;
    struct buffer_head *bh;
    struct dir_entry *de;
    int namelen, inr, idev;

    if (!current->root || !current->root->i_count)
        panic("No root inode");
    if (!current->pwd || !current->pwd->i_count)
        panic("No pwd inode");
    /* The pathname is absolute pathname, we need serch inode from root */
    if ((c = get_fs_byte(pathname)) == '/') {
        inode = current->root;
        pathname++;
    } else if (c)
        inode = current->pwd;
    else
        return NULL; /* empty name is bad */
    inode->i_count++;
    while (1) {
        thisname = pathname;
        if (!S_ISDIR(inode->i_mode) || !permission(inode, MAY_EXEC)) {
            d_iput(inode);
            return NULL;
        }
        for (namelen = 0; (c = get_fs_byte(pathname++)) && (c != '/');
                 namelen++);
            /* nothing */;
        if (!c)
            return inode;
         if (!(bh = find_entry(&inode, thisname, namelen, &de))) {
             d_iput(inode);
             return NULL;
         }
         inr = de->inode;
         idev = inode->i_dev;
         brelse(bh);
         d_iput(inode);
         if (!(inode = d_iget(idev, inr)))
             return NULL;
    }
}

/*
 * dir_namei()
 *
 * @oathname: the pathname to the file.
 * @namelen:  the length to the pahtname.
 * @name:     Top-direntory name on pathname.
 */
static struct m_inode *dir_namei(const char *pathname,
      int *namelen, const char **name)
{
    struct m_inode *dir;
    const char *basename;
    char c;

    if (!(dir = get_dir(pathname)))
        return NULL;
    basename = pathname;
    while ((c = get_fs_byte(pathname++)))
        if (c == '/')
            basename = pathname;
    *namelen = pathname - basename - 1;
    *name = basename;
    return dir;
}

/*
 * add_entry()
 *
 * Adds a file entry to the specified directory. using the same
 * semantics as find_entry(0. It returns NULL if it failed.
 *
 * NOTE!! The inode part of 'de' is left at 0 - which means you
 * may not sleep between calling this and putting something into
 * the entry, as someone else might have used it while you slept.
 */
static struct buffer_head *add_entry(struct m_inode *dir,
       const char *name, int namelen, struct dir_entry **res_dir)
{
    int block, i;
    struct buffer_head *bh;
    struct dir_entry *de;

    *res_dir = NULL;
#ifdef NO_TRUNCATE
    if (namelen > NAME_LEN)
        return NULL;
#else
    if (namelen > NAME_LEN)
        namelen = NAME_LEN;
#endif
    if (!namelen)
        return NULL;
    if (!(block = dir->i_zone[0]))
        return NULL;
    if (!(bh = bread(dir->i_dev, block)))
        return NULL;
    i = 0;
    de = (struct dir_entry *)bh->b_data;
    while (1) {
        if ((char *)de >= BLOCK_SIZE + bh->b_data) {
            brelse(bh);
            bh = NULL;
            block = d_create_block(dir, i / DIR_ENTRIES_PER_BLOCK);
            if (!block)
                return NULL;
            if (!(bh = bread(dir->i_dev, block))) {
                i += DIR_ENTRIES_PER_BLOCK;
                continue;
            }
            de = (struct dir_entry *)bh->b_data;
        }
        if (i * sizeof(struct dir_entry) >= dir->i_size) {
            de->inode = 0;
            dir->i_size = (i + 1) * sizeof(struct dir_entry);
            dir->i_dirt = 1;
            dir->i_ctime = CURRENT_TIME;
        }
        if (!de->inode) {
            dir->i_mtime = CURRENT_TIME;
            for (i = 0; i < NAME_LEN; i++)
                de->name[i] = (i < namelen) ? get_fs_byte(name + i) : 0;
            bh->b_dirt = 1;
            *res_dir = de;
            return bh;
        }
        de++;
        i++;
    }
    brelse(bh);
    return NULL;
}

/*
 * d_open_namei: Obtain inode via pathname.
 *
 * @filename:  The pathname to the file.
 * @flag:      The kind of access requested on the file (read, write, append 
 *             etc.).
 * @mode:      The initial file permission is requested using the third 
 *             argument called mode. This argument is relevant only when 
 *             a new file is being created.
 * @res_inode: inode for pathname.
 *
 * @return:
 *
 * After using the file, the process should close the file using close call, 
 * which takes the file descriptor of the file to be closed. Some filesystems
 * include a disposition to permit releasing the file.
 * 
 * Some computer languages include run-time libraries which include additional
 * functionality for particular filesystems. The open (or some auxiliary 
 * routine) may include specifications for key size, record size, connection 
 * speed. Some open routines include specification of the program code to be 
 * executed in the event of an error.
 *
 * Open flag: (flag argument) 
 *   O_RDONLY: Only read file
 *   O_WRONLY: Only write file
 *   O_ORDWR:  Read or Write file
 *   O_CREAT:  Create the file if it does not exist; otherwise the open fails
 *             setting errno to ENOENT.
 *   O_EXCL:   Used with O_CREAT if the file already exists, then fail, setting
 *             errno to EEXIST.
 *   O_APPEND: data written will be appended to the end of the file. The file
 *             operations will always adjust the position pointer to the end 
 *             of the file.
 *   O_TRUNC:  If the file already exists then discard its previous contents, 
 *             reducing it to an empty file. Not applicable for a device or 
 *             named pipe.
 *
 * mode:
 *   Optional and relevant only when creating a new file, defines the file 
 *   permissions. These include read, write or execute the file by the owner, 
 *   group or all users. The mode is masked by the calling process's umask: 
 *   bits set in the umask are cleared in the mode.
 *
 *   S_IRUSR: User readable.
 *   S_IRGRP: Group readable.
 *   S_IROTH: Other readable.
 *
 *   S_IWUSR: User writeable.
 *   S_IWGRP: Group writeable.
 *   S_IWOTH: Other writeable.
 *
 *   S_IXUSR: User executable.
 *   S_IXGRP: Group executable.
 */
int d_open_namei(const char *pathname, int flag, int mode,
                 struct m_inode **res_inode)
{
    const char *basename;
    struct m_inode *dir;
    struct dir_entry *de;
    struct buffer_head *bh;
    struct m_inode *inode;
    int inr, dev, namelen;

    if ((flag & O_TRUNC) && !(flag & O_ACCMODE))
        flag |= O_WRONLY;
    mode &= 0777 & ~current->umask;
    mode |= I_REGULAR;
    /* Obtain dentory inode */
    if (!(dir = dir_namei(pathname, &namelen, &basename)))
        return -ENOENT;
    /* open a dentory directly */
    if (!namelen) {
        if (!(flag & (O_ACCMODE | O_CREAT | O_TRUNC))) {
            *res_inode = dir;
            return 0;
        }
        d_iput(dir);
        return -EISDIR;
    }

    bh = find_entry(&dir, basename, namelen, &de);
    if (!bh) {
        if (!(flag & O_CREAT)) {
            d_iput(dir);
            return -ENOENT;
        }
        if (!permission(dir, MAY_WRITE)) {
            d_iput(dir);
            return -EACCES;
        }
        inode = d_new_inode(dir->i_dev);
        if (!inode) {
            d_iput(dir);
            return -ENOSPC;
        }
        inode->i_uid  = current->euid;
        inode->i_mode = mode;
        inode->i_dirt = 1;
        bh = add_entry(dir, basename, namelen, &de);
        if (!bh) {
            inode->i_nlinks--;
            d_iput(inode);
            d_iput(dir);
            return -ENOSPC;
        }
        de->inode = inode->i_num;
        bh->b_dirt = 1;
        brelse(bh);
        d_iput(dir);
        *res_inode = inode;
        return 0;
    }
    inr = de->inode;
    dev = dir->i_dev;
    brelse(bh);
    d_iput(dir);
    if (flag & O_EXCL)
        return -EEXIST;
    if (!(inode = d_iget(dev, inr)))
        return -EACCES;
    if ((S_ISDIR(inode->i_mode) && (flag & O_ACCMODE)) ||
         !permission(inode, ACC_MODE(flag))) {
        d_iput(inode);
        return -EPERM;
    }
    inode->i_atime = CURRENT_TIME;
    if (flag & O_TRUNC) {
        /* un-known-routine */;
    }
    *res_inode = inode;
    return 0;
}
