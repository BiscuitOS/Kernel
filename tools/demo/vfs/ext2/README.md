Specification: Second Extended Filesystem
-----------------------------------------------------------------

  The ext2 or second extended file system is a file system for the Linux
  kernel. It was initially designed by `Rémy Card` as a replacement for
  extened system (ext). Having been designed accoring to the same principles
  as the Berkeley Fast File System from BSD, it was the first commercial-
  grade filesystem for Linux.

  The canonical implementation of ext2 is the `ext2fs` filesystm driver in
  the Linux kernel. Other implementations (of varying quality and 
  completeness) exist in GNU Hurd, MINIX3, some BSD kernels, in MiNT , and
  as third-party Microsoft Windows and macOS drivers.

  ext2 was the default filesystem in serval Linux distributions, include
  `Debian` and `Red Hat Linux` until supplanted more recently by ext3, 
  which is almost completely compatible with ext2 and is a journaling file
  system. ext2 is still the filesystem of choice for flash-based storage
  media (such as SD cards and USB flash drives) because its lack of a 
  joural increases performance and minimizes the number of writes, and flash
  devices have a limited number of writes, and flash devices have a limited
  number of write cycles. However, recent Linux kernels support a joural-less
  mode of ext4 which provides benefits not found with ext2.

## History

  The early development of the Linux kernel was made as a cross-development 
  under the MINIX operating system. The MINIX file system was used as 
  Linux's first file system. The Minix file system was mostly free of bugs,
  but used 16-bit offsets internally and thus had a maximum size limit of
  only 64 Megabytes, and there was also a filename length limit of 14 
  characters. Because of these limitations, work begon on a replacement 
  native system for Linux.

  To ease the addition of new file systems and provide a generic file API,
  VFS, a virtual file system layer, was added to the Linux kernel. The
  extended file system (ext), we released in April 1992 as the first file
  system using the VFS API and was included in Linux version 0.96c. The 
  ext file system solved the two major problems in the Minix file system(
  maximum partition size and filename length limitation to 14 characters),
  and allowed 2 gigabytes of data and filenames of up to 255 characters.
  But it still had problems: there was no support of separate timestamps
  for file access, inode modification, and data modification.

  As a solution for these problems, two new filesystems was developed in 
  january 1993 for Linux kernel 0.99: xiafs and the second extended file
  system (ext2), which was an overhaul of the extended file system
  incorporating many ideas from the Berkelely Fast File system. ext2 was
  also designed with extensibility in mind, with space left in many of its
  on-disk data structures for use by future versions.

  Since then, ext2 has been a testbed for many of the new extensions to 
  the VFS API. Features such as the withdrawn POSIX draft ACL proposal 
  and the withdrawn extended attribute proposal were generally implemented
  first on ext2 because it was relatively simple to extend and its internals
  were well understood.

  On Linux kernels prior 2.6.17 restrictions in the vlock driver mean
  that ext2 filesystem have a maximum file size of 2TiB.

  ext2 is still recommended over journaling file system on bootable USB
  flash drives and other solid-state drives. ext2 performs fewer writes
  than ext3 because there is no journaling. As the major aging factor of a
  flash chip is the number of erase cycles, and as erase cycles happen 
  frequently on writes, decreasing writes increases the life span of the
  solid-state device. Another good practice for filesystems on flash devices
  is the use of the noatime mount option, for the same reason.

## ext2 data structure

  The space in ext2 is split up into blocks. These blocks are grouped into
  block groups, analogous to cylinder groups in the Unix File System. There
  are typically thousands of blocks on a large file system. Data for any
  given file is typically contained within a single block group where possible.
  This is done to minimize the number of disk seeks when reading large
  amounts of contiguous data.

  Each block group contains a copy of the superblock and block group descrip-
  or table, and all block groups contain a block bitmap, and inode table, and
  finally the actual data blocks.

  The superblock contains important information that is crucial to the booting
  of the operating system. Thus backup copies are made in multiple block
  groups in the file system. However, typically only the first copy of it,
  which is found at the first block of the file system, is used in the 
  booting.

  The group descriptor stores the location of the block bitmap, inode bitmap,
  and the start of the inode table for every block group. These, in turn,
  as stored in a group descriptor table.

### Inodes

  Every file or directory is represented by an inode. The term `inode` comes
  from `index node` (over time, it become i-node and then inode). The inode
  includes data about the size, permission, ownership, and location on disk
  of the file or directory.

  Figure for ext2 inode structure:

  ```

                      Direct blocks
                       +-------+
                       |       |
      inode            |       |
    +-------+     o--->+-------+         Indirect blocks
    | infos |     |                        +-------+
    +-------+     |                        |       |
    |      -|-----o    +-------+           |       |
    +-------+          |      -|---------->+-------+
    |       |          +-------+
    +-------+          |       |                     
    |      -|--------->+-------+                   Double indirect blocks  
    +-------+                           +-------+      +-------+
    |       |          +-------+        |       |      |       |
    +-------+          |       |        +-------+      |       |
    |      -|--------->+-------+        |      -|----->+-------+
    +-------+          |      -|------->+-------+
                       +-------+
  ``` 

  Quto from the Linux kernel documentation for ext2:

  > There are pointers to the first 12 blocks which contains the file's
  > data in inode. There is a pointer to an indirect block (which contains
  > pointers to the next set of blocks), a pointer to a doubly indirect
  > block and a pointer to a trebly indirect block.

  
  Thus, there is a structure in ext2 that has 15 pointers. Pointer 1 to 12
  point to direct blocks, pointer 13 points to an indirect block, pointer
  14 points to doubly indirect block, and 15 points to a triply indirect
  block.

### Directories

  Each directory is a list of directory entries. Each directory entry associ-
  ates one file name with one inode number, and consists of the inode number,
  the length of the file name, and the actual text of the file name. To find
  a file, the directory is searched front-to-back for the associated filename.
  For reasonable directory sizes, this is fine. But for every large director-
  es is inefficient, and ext3 offers a second way of storing directories (
  HTree) that is more efficient than just a list of filenames.

  The root directory is always stored in inode number two, so that the file
  system code can find it at mount time. Subdirectories are implemented by
  storing the name of the subdirectory in the name field, and the inode
  number of the subdirectory in the inode field. Hard links are implemented
  by storing the same inode number with more than one file name. Accessing
  the file by either name results in the same inode number, and therefore
  the same data.

  The special directories `.` (current directory) and `..` (parent directory)
  are implemented by storing the names `.` and `..` in the directory, and 
  the inode number of the current and parent directories in the inode field.
  The only special treatment these two entries receive is that they are 
  automatically created when any new directory is made, and they cannot be
  deleted.

### Allocating Data

  When a new file or directory is created, ext2 must decide where to store
  the data. If the disk is mostly empty, then data can be stored almost 
  anywhere. However, clustering the data with related data will minimize
  seek times and maximize performance.

  ext2 attempts to allocate each new directory in the group containing its
  parent directory, on the theory that accesses to parent and children
  directories are likely to be closely related. ext2 also attempts to place
  files in the same group as their directory entries, because directory 
  accesses often lead to file accesses. However, if the group is full, then
  the new file or new directory is placed in some other non-fun group.

  The data block needed to store directories and files can be found by 
  looking in the data allocation bitmap. Any needed space in the inode table
  can be found by looking in the inode allocation bitmap.

