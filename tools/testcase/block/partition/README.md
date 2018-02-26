Disk Partition
-----------------------------------------------------------

## Contents

  * MBR

### MBR

  A master boot record (MBR) is a special type of boot sector at the very
  beginning of partitioned computer mass storage device like fixed disks or
  removeable drives intended for use with IBM PC-compatible systems and 
  beyond. The concept of MBRs was publicly introduced in 1983 with PC DOS
  2.0.

  The MBR holds the information on how the logical partitions, containing
  file systems, are organized on that medium. The MBR also contains executable
  code to function as a loader for the installed operating system -- usually
  by passing control over to the loader's second stage, or in conjunction
  with each partition's volume boot record (VBR). This MBR code is usually
  referred to as a boot loader.

  The organization of the partition table in the MBR limits the maximum 
  addressable storage space of a disk to 2 TiB. Approaches to slightly raise
  this limit assuming 33-bit arithmetics or 4096-byte sectors are not
  offcially supported, as they fatally break compatibility with existing
  boot loaders and most MBR-compliant operating systems and system tools,
  and can cause serious data corruption when used outside of narrowly 
  controlled system environments. Therefore, the MBR-based partitioning
  scheme is in the process of being superseded by the GUID Partition Table (
  GPT) scheme in new computer. A GPT can coexist with an MBR in order to 
  provide some limited form of backward compatibility for older systems.

  MBRs are not present on non-partitioned media such as floppies,
  superfloppies or other storage device configure to behave as such.

##### MBR Overview

  Support for partition media, and thereby the master boot record (MBR), was
  introduced with IBM PC DOS 2.0 in March 1983 in order ot support the 10
  MB hard disk of the then-new IBM Personal Computer XT, still using the 
  FAT12 file system. The original version of the MBR was written by David
  Litton of IBM in June 1982. The partition table supported up to four
  primary partitions, of which DOS could only use one. This did not change
  when FAT16 was introduced as new file system with DOS 3.0. Support for an
  extended partition, a special primary partition type used as a container
  to hold other partition, was added with DOS 3.2, and nested logical drives
  inside an extended partition came with DOS 3.30. Since MS-DOS, PC DOS, OS/2
  and Windows were never enabled to boot off them, the MBR format and boot 
  code remained almost unchanged in functionality, except for in some third-
  party implementations, throughout the eras of DOS and OS/2 up to 1996.

  In 1996, support for logical block addressing (LBA) was introduced in 
  Windows 95B and DOS 7.10 in order to support disks larger then 8 GB. Disk
  timestamps were also introduced. This also reflected the idea that the MBR
  is meant to be operating system and file system independent. However, this
  design rule was partially compromised in more recent Microsoft
  implementations of the MBR, which enforce CHS access for FAT16B and FAT32
  partition types 06Hex/0BHex, whereas LBA is used for 0Ehex/0Chex.

  Despite sometimes poor documentation of certain intrinsic details of the 
  MBR format (which occasinally caused compatibility problems), it has been
  widely adopted as a de facto induatry standard, due to the broad popularity
  of PC-compatible computers and its semi-static nature over decades. This
  was even to the extent of being supported by computer operating systems
  for other platforms. Sometimes this was in addition to other pre-existing
  or corss-platform standards for bootstrapping and partitioning.

  MBR partition entries and the MBR boot code used in commercial operating
  systems,
  
