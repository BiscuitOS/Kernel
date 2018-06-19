MINIXFS
-------------------------------------------------------------

  MINIX was written from scratch by Andrew S. Tanenbaum in the 1980s, as a 
  Unix-like operating system whose source code could be used freely in 
  education. The MINIX file system was designed for use with MINIX. it copies 
  the basic structure of the Unix File System but avoids any complex features
  in the interest of keeping the source code clean, clear and simple, to meet
  the overall goal of MINIX to be a useful teaching aid.

  When Linus Torvalds first started writing his Linux operating system kernel 
  (1991), he was working on a machine running MINIX, and adopted its file 
  system layout. This soon proved problematic, since MINIX restricted filename 
  lengths to fourteen characters (thirty in later versions), it limited 
  partitions to 64 megabytes, and the file system was designed for teaching 
  purposes, not performance. The Extended file system (ext; April 1992) was 
  developed to replace MINIX's, but it was only with the second version of 
  this, ext2, that Linux obtained a commercial-grade file system. As of 1994, 
  the MINIX file system was "scarcely in use" among Linux users.

### Design and implementation

  A MINIX file system has six components:

  * The Boot Block which is always stored in the first block. 
    It contains the boot loader that loads and runs an operating system 
    at system startup.

  * The second block is the Superblock which stores data about the file system,
    that allows the operating system to locate and understand other file 
    system structures. For example, the number of inodes and zones, the size 
    of the two bitmaps and the starting block of the data area.

  * The inode bitmap is a simple map of the inodes that tracks which ones are 
    in use and which ones are free by representing them as either a one (in 
    use) or a zero (free).

  * The zone bitmap works in the same way as the inode bitmap, except it 
    tracks the zones.

  * The inodes area. Each file or directory is represented as an inode, 
    which records metadata including type (file, directory, block, char, 
    pipe), IDs for user and group, three timestamps that record the date and 
    time of last access, last modification and last status change. An inode 
    also contains a list of addresses that point to the zones in the data 
    area where the file or directory data is actually stored.

  * The data area is the largest component of the file system, using the 
    majority of the space. It is where the actual file and directory data 
    are stored.

  ```
   MINIX FS:

   +------------+-------------+--------------+-------------+-------+------+
   |            |             |              |             |       |      |
   | Boot Block | Super Block | inode BitMap | Zone BitMap | Inode | Data |
   |            |             |              |             |       |      |
   +------------+-------------+--------------+-------------+-------+------+
  ```
