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
  systems, however, are limited to 32 bits. Therefore, the maximum disk size
  supported on disks using 512-byte sectors (whether real or emulated) by the
  MBR partitioning scheme (without using non-standard methods) is limited to
  2-TiB. Consequently, a different partitioning scheme must be used for larger
  disks, as they have become widely available since 2010. The MBR partitioning
  scheme is therefore in the process of being superseded by the GUID Partition
  Table (GPT). The official approach does little more than ensuring data
  integrity by employing a protective MBR. Specifically, it does not provide
  backward compatibility with operating systems that do not support the GPT
  scheme as well. Meanwhile, multiple forms of hybrid MBRs have been designed
  and implemented by third parties in order to maintain partitions located
  in the first physical 2 TiB of a disk in both partitioning schemes "in 
  parallel" and/or  to allow older operating systems to boot off GTP partitions
  as well. The present non-standard nature of these solution causes various
  compatibility problems in certain scenarios.

  The MBR consists of 512 or more bytes located in the first sector of the 
  drive.

  * A partition table describing the partitions of a storage device. In this
    context the boot sector may also be called a partition sector.

  * Bootstrap code: Instructions to identify the configured bootable
    partition, then load and execute its volume boot record (VBR) as a 
    chain loader.

  * Optional 32-bit disk timestamp.

  * Optional 32-bit disk signature.
  
##### MBR Disk partitioning

  IBM PC DOS 2.0 introduced the `FDISK` utility to setup and maintain MBR
  partitions. When a storage device has been partitioned according to this
  scheme, its MBR contains a partition table describing the locations table
  describing the locations, sizes, and other attributes of linear regions
  referred to as partitions.

  The partitions themselves may also contain data to descirbe more complex
  partitioning schemes, such as extended boot record (EBRs). BSD disklabels,
  or Logical Disk Manager metadata partitions.

  The MBR is not located in a partition. It is located at a first sector of
  the device (physical offset 0), preceding the first partition. (The boot
  sector present on a non-partitioned device or within an individual
  partition is called a volume boot recode insted.) In cases where the 
  computer is running a DDO BIOS overlay or boot manager, the partition table
  may be moved to some other physical location on the device. e.g., Ontrack
  Disk Manager oftern placed a copy of the original MBR contents in the 
  second sector, then hid itself from any subsequently booted OS or
  application, so the MBR copy was treated as if it were still residing in
  the first sector.

##### MBR Sector layout

  By convention, there are exactly four primary partition table entries in
  the MBR partition table scheme, although some operating systems and system
  tools extended this to five (Advanced Active Partitions (AAP) with PTS-DOS
  6.60 and DR-DOS), eight (AST and NEC MS-DOS 3.x as well as Storage 
  Dimensions SpeedStor), or even sixteen entries (with Ontrack Disk Manager)

  ![MBR0](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/block/MBR_P0.png)

  ![MBR1](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/block/MBR_P1.png)

  ![MBR2](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/block/MBR_P2.png)

  ![MBR3](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/block/MBR_P3.png)

  ![MBR4](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/block/MBR_P4.png)

  ![MBR5](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/block/MBR_P5.png)



