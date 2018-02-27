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

#### MBR Overview

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
  
#### MBR Disk partitioning

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

#### MBR Sector layout

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

#### MBR Partition table entries

  An artifact of hard disk technology from the ear of the PC XT, the partition
  table subdivides a storatge medium using units of cylinder, heads, and 
  sectors (CHS addressing). These values no longer correspond to their 
  namesakes in modern disk drivers, as well as being irrelevant in other
  devices such as solidstate drives, which do not physically have cylinders
  or heads.

  In the CHS scheme, sector indices have (almost) always begun with sector 1
  rather then sector 0 by convention, and due to an error in all versions of
  MS-DOS/PC DOS up to including 7.10, the number of heads is generally limited
  to 255 instead of 256. When a CHS address is too large to fit into these
  fields, the tuple (1023, 254, 63) is typically used today, although on older
  systems, and with older disk tools, the cylinder value oftern wrapped
  around module the CHS barrier near 8 GB, causing ambiguity and risks of
  data corruption. (If the situation involves a "protective" MBR on a disk
  with a GPT, Intel's Extensible Firmware Interface specification requires
  that the tuple (1023, 255, 63) be used.) The 10-bit cylinder value is
  recorded within two bytes in order to facilitate making calls to the
  original/legacy INT 13h BIOS disk access routines, where 16 bits were
  divided into sector and cylinder parts, and not on byte boundaries.

  Due to the limits of CHS addressing, a translation was made to using LBA,
  or logical block addressing. Both the partition length and partition start
  address are sector value stored in partition table entries as 32-bit
  quantities. The sector size used to considered fixed at 512 bytes, and a
  broad range of important components including chipsets, boot sectors,
  operating systems, database engines, partition tools, backup and file
  system utilities and other software had this value hard-coded. Since the 
  end of 2009, disk drives employing 4096-byte sectors (4Kn or Advanced
  Format) have been available, although the size of the sector for some of 
  these drives was still reported as 512 bytes to the host system through
  conversion in the hard-drive firmware and referred to as 512 emulation
  drivers.

  ![MBR0](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/block/MBR_A0.png)

  Since block addresses and sizes are stored in the partition table of an
  MBR using 32 bits, the maximal size, as well as the highest start
  address, of a partition using drives that have 512-byte sectors gives a
  total size approaching 4-TiB, if all but one partition are located below
  the 2 TiB limit and the last one is assigned as starting at or close to
  block 2^32 - 1 and specify the size as up to 2^32 - 1, thereby defining
  a partition that requires 33 rather than 32 bits for the sector address
  to be accessed. However, in practice, only certain LBA-48-enabled
  operating system, including GNU/Linux, Free BSD and Windows7 that use
  64-bit sector address internally actually support this. Due to code space
  constraints and the nature of the MBR partition table to only support 32
  bits, boot sectors, even if enabled to support LBA-48 rather that LBA-28,
  often use 32-bit calculations, unless system using 32-bit sector addresses
  internally would cause addresses to wrap around accessing this partition
  and thereby result in serious data corruption over all partitions.

  For disks that present a sector size other than 512 bytes, such as USB 
  external drives, there are limitations as well. A sector size of 4096 
  results in an eight-fold increase in the size of a partition that can be 
  defined using MBR, allowing partitions up to 16 TiB (232 × 4096 bytes) in 
  size. Versions of Windows more recent than Windows XP support the larger 
  sector sizes, as well as Mac OS X, and Linux has supported larger sector 
  sizes since 2.6.31 or 2.6.32, but issues with boot loaders, partitioning 
  tools and computer BIOS implementations present certain limitations, since 
  they are often hard-wired to reserve only 512 bytes for sector buffers, 
  causing memory to become overwritten for larger sector sizes. This may 
  cause unpredictable behaviour as well, and therefore should be avoided 
  when compatibility and standard conformity is an issue.

  Where a data storage device has been partitioned with the GPT scheme, 
  the master boot record will still contain a partition table, but its only 
  purpose is to indicate the existence of the GPT and to prevent utility 
  programs that understand only the MBR partition table scheme from creating 
  any partitions in what they would otherwise see as free space on the disk, 
  thereby accidentally erasing the GPT.

#### MBR System Bootstrapping

  On IBM PC-compatible computers, the bootstrapping firmware (contained 
  within the ROM BIOS) loads and executes the master boot record. The PC/XT
  (type 5160) used an Intel 8088 microprocessor. In order to remain 
  compatible, all x86 architecture systems start with the microprocessor in
  an operating mode referred to as real mode. The BIOS reads the MBR from 
  the storage device into physical memory, and then it directs the 
  microprocessor to the start of the boot code. Since the BIOS runs in 
  real mode, the processor is in real mode when the MBR program begins to 
  execute, and so the beginning of the MBR is expected to contain real-mode 
  machine code.

  Due to the restricted size of the MBR's code section, it typically contains 
  only a small program that copies additional code (such as a boot loader) 
  from the storage device into memory. Control is then passed to this code, 
  which is responsible for loading the actual operating system. This process 
  is known as chain loading.

  Popular MBR code programs were created for booting PC DOS and MS-DOS, and 
  similar boot code remains in wide use. These boot sectors expect the fdisk 
  partition table scheme to be in use and scans the list of partitions in the 
  MBR's embedded partition table to find the only one that is marked with the 
  active flag. It then loads and runs the volume boot record (VBR) of the 
  active partition.

  There are alternative boot code implementations, some of which are installed 
  by boot managers, which operate in a variety of ways. Some MBR code loads 
  additional code for a boot manager from the first track of the disk, which 
  it assumes to be "free" space that is not allocated to any disk partition, 
  and executes it. A MBR program may interact with the user to determine which 
  partition on which drive should boot, and may transfer control to the MBR of 
  a different drive. Other MBR code contains a list of disk locations (often 
  corresponding to the contents of files in a filesystem) of the remainder of 
  the boot manager code to load and to execute. (The first relies on behavior 
  that is not universal across all disk partitioning utilities, most notably 
  those that read and write GPTs. The last requires that the embedded list 
  of disk locations be updated when changes are made that would relocate the 
  remainder of the code.)

  On machines that do not use x86 processors, or on x86 machines with non-BIOS 
  firmware such as Open Firmware or Extensible Firmware Interface (EFI) 
  firmware, this design is unsuitable, and the MBR is not used as part of the 
  system bootstrap. EFI firmware is instead capable of directly understanding 
  the GPT partitioning scheme and the FAT filesystem format, and loads and 
  runs programs held as files in the EFI System partition. The MBR will be 
  involved only insofar as it might contain a partition table for 
  compatibility purposes if the GPT partition table scheme has been used.

  There is some MBR replacement code that emulates EFI firmware's bootstrap, 
  which makes non-EFI machines capable of booting from disks using the GPT 
  partitioning scheme. It detects a GPT, places the processor in the correct 
  operating mode, and loads the EFI compatible code from disk to complete this 
  task.

#### MBR Programming considerations

  The MBR originated in the PC XT. IBM PC-compatible computers are 
  little-endian, which means the processor stores numeric values spanning two 
  or more bytes in memory least significant byte first. The format of the MBR 
  on media reflects this convention. Thus, the MBR signature will appear in a 
  disk editor as the sequence 55hex AAhex.

  The bootstrap sequence in the BIOS will load the first valid MBR that it 
  finds into the computer's physical memory at address 0000hex:7C00hex. The 
  last instruction executed in the BIOS code will be a "jump" to that address, 
  to direct execution to the beginning of the MBR copy. The primary validation 
  for most BIOSes is the signature at offset +1FEhex, although a BIOS 
  implementer may choose to include other checks, such as verifying that the 
  MBR contains a valid partition table without entries referring to sectors 
  beyond the reported capacity of the disk.

  While the MBR boot sector code expects to be loaded at physical address 
  `0000hex:7C00hex`, all the memory from physical address 0000hex:0501hex 
  (address 0000hex:0500hex is the last one used by a Phoenix BIOS) to 
  `0000hex:7FFFhex`, later relaxed to 0000hex:FFFFhex (and sometimes up to 
  9000hex:FFFFhex)—the end of the first 640 KB—is available in real mode. The
  INT 12h BIOS interrupt call may help in determining how much memory can be
  allocated safely (by default, it simply reads the base memory size in KB 
  from segment:offset location 0040hex:0013hex, but it may be hooked by other 
  resident pre-boot software like BIOS overlays, RPL code or viruses to reduce 
  the reported amount of available memory in order to keep other boot stage 
  software like boot sectors from overwriting them).

  The last 66 bytes of the 512-byte MBR are reserved for the partition table 
  and other information, so the MBR boot sector program must be small enough 
  to fit within 446 bytes of memory or less. The MBR code may communicate 
  with the user, examine the partition table. Eventually, the MBR will need to 
  perform its main task, and load the program that will perform the next stage 
  of the boot process, usually by making use of INT 13h BIOS calls. While it 
  may be convenient to think of the MBR and the program that it loads as 
  separate and discrete, a clear distinction between the MBR and the loaded 
  OS is not technically required—the MBR, or parts of it, could stay resident
  in RAM and be used as part of the loaded program, after the MBR transfers 
  control to that program. The same is true of a volume boot record, whether 
  that volume is a floppy disk or a fixed disk partition. However, in practice,
  it is typical for the program loaded by a boot record program to discard 
  and overwrite the RAM image of the latter, so that its only function is as
  the first link of the boot loader chain.

  From a technical standpoint, it is important to note that the distinction 
  between an MBR and a volume boot record exists only at the user software 
  level, above the BIOS firmware. (Here, the term "user software" refers to 
  both operating system software and application software.) To the BIOS, 
  removable (e.g. floppy) and fixed disks are essentially the same. For either,
  the BIOS reads the first physical sector of the media into RAM at absolute 
  address 7C00hex, checks the signature in the last two bytes of the loaded 
  sector, and then, if the correct signature is found, transfers control to 
  the first byte of the sector with a jump (JMP) instruction. The only real 
  distinction that the BIOS makes is that (by default, or if the boot order 
  is not configurable) it attempts to boot from the first removable disk 
  before trying to boot from the first fixed disk. From the perspective of 
  the BIOS, the action of the MBR loading a volume boot record into RAM is 
  exactly the same as the action of a floppy disk volume boot record loading 
  the object code of an operating system loader into RAM. In either case, the
  program that BIOS loaded is going about the work of chain loading an 
  operating system. The distinction between an MBR and a volume boot record 
  is an OS software-level abstraction, designed to help people to understand
  the operational organization and structure of the system. That distinction
  doesn't exist for the BIOS. Whatever the BIOS directly loads, be it an MBR
  or a volume boot record, is given total control of the system, and the BIOS
  from that point is solely at the service of that program. The loaded program
  owns the machine (until the next reboot, at least). With its total control,
  this program is not required to ever call the BIOS again and may even shut
  BIOS down completely, by removing the BIOS ISR vectors from the processor
  interrupt vector table, and then overwrite the BIOS data area. This is 
  mentioned to emphasize that the boot program that the BIOS loads and runs 
  from the first sector of a disk can truly do anything, so long as the 
  program does not call for BIOS services or allow BIOS ISRs to be invoked
  after it has disrupted the BIOS state necessary for those services and ISRs
  to function properly.

  As stated above, the conventional MBR bootstrap code loads and runs (boot 
  loader- or operating system-dependent) volume boot record code that is 
  located at the beginning of the "active" partition. A conventional volume 
  boot record will fit within a 512-byte sector, but it is safe for MBR code 
  to load additional sectors to accommodate boot loaders longer than one 
  sector, provided they do not make any assumptions on what the sector size 
  is. In fact, at least 1 KB of RAM is available at address 7C00hex in every 
  IBM XT- and AT-class machine, so a 1 KB sector could be used with no 
  problem. Like the MBR, a volume boot record normally expects to be loaded 
  at address 0000hex:7C00hex. This derives from the fact that the volume boot 
  record design originated on unpartitioned media, where a volume boot 
  record would be directly loaded by the BIOS boot procedure; as mentioned 
  above, the BIOS treats MBRs and volume boot records (VBRs) exactly alike. 
  Since this is the same location where the MBR is loaded, one of the first 
  tasks of an MBR is to relocate itself somewhere else in memory. The 
  relocation address is determined by the MBR, but it is most often 
  0000hex:0600hex (for MS-DOS/PC DOS, OS/2 and Windows MBR code) or 
  0060hex:0000hex (most DR-DOS MBRs). (Even though both of these segmented 
  addresses resolve to the same physical memory address in real mode, for 
  Apple Darwin to boot, the MBR must be relocated to 0000hex:0600hex instead 
  of 0060hex:0000hex, since the code depends on the DS:SI pointer to the 
  partition entry provided by the MBR, but it erroneously refers to it via 
  0000hex:SI only.[34]) While the MBR code relocates itself it is still 
  important not to relocate to other addresses in memory because many VBRs 
  will assume a certain standard memory layout when loading their boot file.

  The Status field in a partition table record is used to indicate an active 
  partition. Standard-conformant MBRs will allow only one partition marked 
  active and use this as part of a sanity-check to determine the existence 
  of a valid partition table. They will display an error message, if more 
  than one partition has been marked active. Some non-standard MBRs will not
  treat this as an error condition and just use the first marked partition 
  in the row.

  Traditionally, values other than 00hex (not active) and 80hex (active) 
  were invalid and the bootstrap program would display an error message upon
  encountering them. However, the Plug and Play BIOS Specification and BIOS 
  Boot Specification (BBS) allowed other devices to become bootable as well 
  since 1994. Consequently, with the introduction of MS-DOS 7.10 (Windows 95B)
  and higher, the MBR started to treat a set bit 7 as active flag and showed 
  an error message for values 01hex..7Fhex only. It continued to treat the 
  entry as physical drive unit to be used when loading the corresponding 
  partition's VBR later on, thereby now also accepting other boot drives 
  than 80hex as valid, however, MS-DOS did not make use of this extension 
  by itself. Storing the actual physical drive number in the partition table
  does not normally cause backward compatibility problems, since the value 
  will differ from 80hex only on drives other than the first one (which have 
  not been bootable before, anyway). However, even with systems enabled to 
  boot off other drives, the extension may still not work universally, for 
  example, after the BIOS assignment of physical drives has changed, for 
  example when drives are removed, added or swapped. Therefore, per the BIOS 
  Boot Specification (BBS), it is best practice for a modern MBR accepting 
  bit 7 as active flag to pass on the DL value originally provided by the 
  BIOS instead of using the entry in the partition table.


#### MBR Link

  WIKI: https://en.wikipedia.org/wiki/Master_boot_record
