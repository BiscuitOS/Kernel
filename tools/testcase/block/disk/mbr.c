/*
 * MBR: Master Boot Record
 *
 * (C) 2018.02 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/hdreg.h>

/*
 * MBR
 *   A master boot record (MBR) is a special type of boot sector at the
 *   very begining of partition computer mass storage devices like fixed
 *   disks or removable drives intended for use with IBM PC-compatible
 *   systems and beyond. The concept of MBRs was publicly introduced in
 *   1983 with PC DOS 2.0.
 *
 *   The MBR holds the information on how logical partitions, containing
 *   file system, are organized on that medium. The MBR also contains
 *   executable code to function as a loader for the installed operating
 *   system usually by passing control over to the loader's second stage,
 *   or in conjunction with each partition's volume boot record (VBR).
 *   This MBR code is usually referred to as a boot loader.
 *
 *   The organization of the partition table in the MBR limits the maximum
 *   addressable storage space of a disk to 2Tib (232 X 512 Bytes).
 *   Approaches to slightly raise this limit assuming 33 bit arithmetics
 *   or 4096-byte sectors are not officially supported, as they fatally 
 *   break compatibility with existing boot loaders and most MBR-compliant
 *   operating systems and system tools, and can cause serious data
 *   corruption when used outside of narrowly controlled system environments.
 *   Therefore, the MBR-based partitioning scheme is in the process of being
 *   susperseded by the GUID Partition Table (GPT) scheme in new computers.
 *   A GPT can coexist with an MBR in order to provide some limited form
 *   of backward compatibility for older systems.
 *
 *   MBRs are not present on non-partitioned media such as floppies,
 *   superfloppies or other storage devices configured to behave as such.
 *
 *   Schem of MBR
 * 
 *   .-------------------------------------------------------------------.
 *   | Sector Address | Describe                                         |
 *   ---------------------------------------------------------------------
 *   | 0x00 - 0x88    | Boot Code                                        |
 *   ---------------------------------------------------------------------
 *   | 0x89 - 0x1BD   | Error information                                |
 *   ---------------------------------------------------------------------
 *   |               *** 1st Partition Table ***                         |
 *   ---------------------------------------------------------------------
 *   | 0x1BE          | Bootable Identificiation:                        |
 *   | (Offset 0x00)  | 00: Un-bootable  -- 0x80: Bootable               |
 *   ---------------------------------------------------------------------
 *   | 0x1BF - 0x1C1  | 1st partition Original partition                 |
 *   | (Offset 0x01)  | Little-endian                                    |
 *   |                | 0x1BF[7:0]: Original Head                        |
 *   |                | 0x1C0[5:0]: Original Sector                      |
 *   |                | 0x1C0[7:6]: MSB of Original Cylinder             |
 *   |                | 0x1C1[7:0]: LSB of Original Cylinder             |
 *   ---------------------------------------------------------------------
 *   | 0x1C2          | Partition Type                                   |
 *   | (Offset 0x4)   | 00H: Un-used                                     |
 *   |                | 06H: FAT16 Basic partition                       |
 *   |                | 0BH: FAT32 Basic partition                       |
 *   |                | 05H: Extend partition                            |
 *   |                | 07H: NTFS partition                              |
 *   |                | 0FH: (LBA mode) Extern partition                 |
 *   ---------------------------------------------------------------------
 *   | 0x1C3 - 0x1C5  | 1st partition End CHS                            |
 *   | (Offset 0x5)   | Little-endian                                    |
 *   |                | 0x1C3[7:0]: End Head                             |
 *   |                | 0x1C4[5:0]: End Sector                           |
 *   |                | 0x1C4[7:6]: MSB of End Cylinder                  |
 *   |                | 0x1C5[7:0]: LSB of End Cylinder                  |
 *   ---------------------------------------------------------------------
 *   | 0x1C6 - 0x1C9  | Used Sector for 1st partition                    |
 *   | (Offset 0x8)   |                                                  |
 *   ---------------------------------------------------------------------
 *   | 0x1CA - 0x1CD  | Total Sector for 1st partition                   |
 *   | (Offset 0xC)   |                                                  |
 *   ---------------------------------------------------------------------
 *   |               *** 2nd Partition Table ***                         |
 *   ---------------------------------------------------------------------
 *   | 0x1CE          | Bootable Identificiation:                        |
 *   | (Offset 0x00)  | 00: Un-bootable  -- 0x80: Bootable               |
 *   ---------------------------------------------------------------------
 *   | 0x1CF - 0x1D1  | 2nd partition Original partition                 |
 *   | (Offset 0x01)  | Little-endian                                    |
 *   |                | 0x1CF[7:0]: Original Head                        |
 *   |                | 0x1D0[5:0]: Original Sector                      |
 *   |                | 0x1D0[7:6]: MSB of Original Cylinder             |
 *   |                | 0x1D1[7:0]: LSB of Original Cylinder             |
 *   ---------------------------------------------------------------------
 *   | 0x1D2          | Partition Type                                   |
 *   | (Offset 0x4)   | 00H: Un-used                                     |
 *   |                | 06H: FAT16 Basic partition                       |
 *   |                | 0BH: FAT32 Basic partition                       |
 *   |                | 05H: Extend partition                            |
 *   |                | 07H: NTFS partition                              |
 *   |                | 0FH: (LBA mode) Extern partition                 |
 *   ---------------------------------------------------------------------
 *   | 0x1D3 - 0x1D5  | 2nd partition End CHS                            |
 *   | (Offset 0x5)   | Little-endian                                    |
 *   |                | 0x1D3[7:0]: End Head                             |
 *   |                | 0x1D4[5:0]: End Sector                           |
 *   |                | 0x1D4[7:6]: MSB of End Cylinder                  |
 *   |                | 0x1D5[7:0]: LSB of End Cylinder                  |
 *   ---------------------------------------------------------------------
 *   | 0x1D6 - 0x1D9  | Used Sector for 2nd partition                    |
 *   | (Offset 0x8)   |                                                  |
 *   ---------------------------------------------------------------------
 *   | 0x1DA - 0x1DD  | Total Sector for 2nd partition                   |
 *   | (Offset 0xC)   |                                                  |
 *   ---------------------------------------------------------------------
 *   |               *** 3th Partition Table ***                         |
 *   ---------------------------------------------------------------------
 *   | 0x1DE          | Bootable Identificiation:                        |
 *   | (Offset 0x00)  | 00: Un-bootable  -- 0x80: Bootable               |
 *   ---------------------------------------------------------------------
 *   | 0x1DF - 0x1E1  | 3th partition Original partition                 |
 *   | (Offset 0x01)  | Little-endian                                    |
 *   |                | 0x1DF[7:0]: Original Head                        |
 *   |                | 0x1E0[5:0]: Original Sector                      |
 *   |                | 0x1E0[7:6]: MSB of Original Cylinder             |
 *   |                | 0x1E1[7:0]: LSB of Original Cylinder             |
 *   ---------------------------------------------------------------------
 *   | 0x1E2          | Partition Type                                   |
 *   | (Offset 0x4)   | 00H: Un-used                                     |
 *   |                | 06H: FAT16 Basic partition                       |
 *   |                | 0BH: FAT32 Basic partition                       |
 *   |                | 05H: Extend partition                            |
 *   |                | 07H: NTFS partition                              |
 *   |                | 0FH: (LBA mode) Extern partition                 |
 *   ---------------------------------------------------------------------
 *   | 0x1E3 - 0x1E5  | 3th partition End CHS                            |
 *   | (Offset 0x5)   | Little-endian                                    |
 *   |                | 0x1E3[7:0]: End Head                             |
 *   |                | 0x1E4[5:0]: End Sector                           |
 *   |                | 0x1E4[7:6]: MSB of End Cylinder                  |
 *   |                | 0x1E5[7:0]: LSB of End Cylinder                  |
 *   ---------------------------------------------------------------------
 *   | 0x1E6 - 0x1E9  | Used Sector for 3th partition                    |
 *   | (Offset 0x8)   |                                                  |
 *   ---------------------------------------------------------------------
 *   | 0x1EA - 0x1ED  | Total Sector for 3th partition                   |
 *   | (Offset 0xC)   |                                                  |
 *   ---------------------------------------------------------------------
 *   |               *** 4th Partition Table ***                         |
 *   ---------------------------------------------------------------------
 *   | 0x1EE          | Bootable Identificiation:                        |
 *   | (Offset 0x00)  | 00: Un-bootable  -- 0x80: Bootable               |
 *   ---------------------------------------------------------------------
 *   | 0x1EF - 0x1F1  | 4th partition Original partition                 |
 *   | (Offset 0x01)  | Little-endian                                    |
 *   |                | 0x1EF[7:0]: Original Head                        |
 *   |                | 0x1F0[5:0]: Original Sector                      |
 *   |                | 0x1F0[7:6]: MSB of Original Cylinder             |
 *   |                | 0x1F1[7:0]: LSB of Original Cylinder             |
 *   ---------------------------------------------------------------------
 *   | 0x1F2          | Partition Type                                   |
 *   | (Offset 0x4)   | 00H: Un-used                                     |
 *   |                | 06H: FAT16 Basic partition                       |
 *   |                | 0BH: FAT32 Basic partition                       |
 *   |                | 05H: Extend partition                            |
 *   |                | 07H: NTFS partition                              |
 *   |                | 0FH: (LBA mode) Extern partition                 |
 *   ---------------------------------------------------------------------
 *   | 0x1F3 - 0x1F5  | 4th partition End CHS                            |
 *   | (Offset 0x5)   | Little-endian                                    |
 *   |                | 0x1F3[7:0]: End Head                             |
 *   |                | 0x1F4[5:0]: End Sector                           |
 *   |                | 0x1F4[7:6]: MSB of End Cylinder                  |
 *   |                | 0x1F5[7:0]: LSB of End Cylinder                  |
 *   ---------------------------------------------------------------------
 *   | 0x1F6 - 0x1F9  | Used Sector for 4th partition                    |
 *   | (Offset 0x8)   |                                                  |
 *   ---------------------------------------------------------------------
 *   | 0x1FA - 0x1FD  | Total Sector for 4th partition                   |
 *   | (Offset 0xC)   |                                                  |
 *   ---------------------------------------------------------------------
 *   | 0x1FE          | 0x55: End identification                         |
 *   ---------------------------------------------------------------------
 *   | 0x1FF          | 0xAA                                             |
 *   .-------------------------------------------------------------------.
 */
static struct partition *MBR_partition0;
static struct partition *MBR_partition1;
static struct partition *MBR_partition2;
static struct partition *MBR_partition3;

/*
 * Obtain Metadata of HD0
 *
 * .-----------------------------------------------------------.
 * | 1st hard-disk information (start: 0x90080)                |
 * -------------------------------------------------------------
 * | 0x90080: Cylinder number                                  |
 * | 0x90081:                                                  |
 * -------------------------------------------------------------
 * | 0x90082: Head number                                      |
 * -------------------------------------------------------------
 * | 0x90083: Cylinder number for begining reduce write        |
 * | 0x90084: current (Only PC XT used, other reserved 0)      |
 * -------------------------------------------------------------
 * | 0x90085: Cylinder number for Write Precompensation        |
 * | 0x90086:                                                  |
 * -------------------------------------------------------------
 * | 0x90087: Max trigger length for ECC (Only XT used)        |
 * -------------------------------------------------------------
 * | 0x90088: Control byte                                     |
 * |          bit 0    Unused                                  |
 * |          bit 1    Reserved 0 (close IRQ)                  |
 * |          bit 2    Permit Reset                            |
 * |          bit 3    Set 1(head > 8)                         |
 * |          bit 4    Unused                                  |
 * |          bit 5    If bad block map of vendor exists       |
 * |                   on Cylinder+1, and set this bit.        |
 * |          bit 6    Forbid ECC retry                        |
 * |          bit 7    Forbid Access retry                     |
 * -------------------------------------------------------------
 * | 0x90089: Standard timeout value (Only XT used)            |
 * -------------------------------------------------------------
 * | 0x9008a: Format timeout value (Only XT used)              |
 * -------------------------------------------------------------
 * | 0x9008b: Check driver timeout value (Only XT used)        |
 * -------------------------------------------------------------
 * | 0x9008c: Cylinder number that head location on            |
 * | 0x9008d:                                                  |
 * -------------------------------------------------------------
 * | 0x9008e: Sector number for track                          |
 * -------------------------------------------------------------
 * | 0x9008f: Reserved                                         |
 * .-----------------------------------------------------------.
 *
 */

/*
 * Parse MBR from HD0, and establish 4 partition tables.
 */
static struct buffer_head *obtain_MBR(void)
{
    struct buffer_head *bh;

    /* Obtain first hard disk MBR */
    bh = bread(0x300, 0);
    if (bh == NULL) {
        printk("Unable to obtain HD0 MBR\n");
        return NULL;
    }
    
    if (bh->b_data[0x1FE] != 0x55 || (unsigned char)
        bh->b_data[0x1FF] != 0xAA) {
        printk("Unable to obtain correct MBR\n");
        brelse(bh);
        return NULL;
    }

    /* Obtain Partition Table 0 */
    MBR_partition0 = (struct partition *)&bh->b_data[0x1BE];
    /* Obtian Partition Table 1 */
    MBR_partition1 = (struct partition *)&bh->b_data[0x1CE];
    /* Obtain Partition Table 2 */
    MBR_partition2 = (struct partition *)&bh->b_data[0x1DE];
    /* Obtain Partition Table 3 */
    MBR_partition3 = (struct partition *)&bh->b_data[0x1EE];        
    return bh;
}

/*
 * Partition Type
 *
 *   Type e.g.
 * 
-------------------------------------------------------------------------------
 0  Empty           24  NEC DOS         81  Minix / old Lin bf  Solaris        
 1  FAT12           27  Hidden NTFS Win 82  Linux swap / So c1  DRDOS/sec (FAT-
 2  XENIX root      39  Plan 9          83  Linux           c4  DRDOS/sec (FAT-
 3  XENIX usr       3c  PartitionMagic  84  OS/2 hidden C:  c6  DRDOS/sec (FAT-
 4  FAT16 <32M      40  Venix 80286     85  Linux extended  c7  Syrinx         
 5  Extended        41  PPC PReP Boot   86  NTFS volume set da  Non-FS data    
 6  FAT16           42  SFS             87  NTFS volume set db  CP/M / CTOS / .
 7  HPFS/NTFS/exFAT 4d  QNX4.x          88  Linux plaintext de  Dell Utility   
 8  AIX             4e  QNX4.x 2nd part 8e  Linux LVM       df  BootIt         
 9  AIX bootable    4f  QNX4.x 3rd part 93  Amoeba          e1  DOS access     
 a  OS/2 Boot Manag 50  OnTrack DM      94  Amoeba BBT      e3  DOS R/O        
 b  W95 FAT32       51  OnTrack DM6 Aux 9f  BSD/OS          e4  SpeedStor      
 c  W95 FAT32 (LBA) 52  CP/M            a0  IBM Thinkpad hi eb  BeOS fs        
 e  W95 FAT16 (LBA) 53  OnTrack DM6 Aux a5  FreeBSD         ee  GPT            
 f  W95 Ext'd (LBA) 54  OnTrackDM6      a6  OpenBSD         ef  EFI (FAT-12/16/
10  OPUS            55  EZ-Drive        a7  NeXTSTEP        f0  Linux/PA-RISC b
11  Hidden FAT12    56  Golden Bow      a8  Darwin UFS      f1  SpeedStor      
12  Compaq diagnost 5c  Priam Edisk     a9  NetBSD          f4  SpeedStor      
14  Hidden FAT16 <3 61  SpeedStor       ab  Darwin boot     f2  DOS secondary  
16  Hidden FAT16    63  GNU HURD or Sys af  HFS / HFS+      fb  VMware VMFS    
17  Hidden HPFS/NTF 64  Novell Netware  b7  BSDI fs         fc  VMware VMKCORE 
18  AST SmartSleep  65  Novell Netware  b8  BSDI swap       fd  Linux raid auto
1b  Hidden W95 FAT3 70  DiskSecure Mult bb  Boot Wizard hid fe  LANstep        
1c  Hidden W95 FAT3 75  PC/IX           be  Solaris boot    ff  BBT            
1e  Hidden W95 FAT1 80  Old Minix
-------------------------------------------------------------------------------
 */
static int parse_partition_type(struct partition *p)
{
    return p->sys_ind;
}

/*
 * Parse a partition
 *
 * Schem of Partition e.g.
 *
 *   .-------------------------------------------------------------------.
 *   | 0x1BE          | Bootable Identificiation:                        |
 *   | (Offset 0x00)  | 00: Un-bootable  -- 0x80: Bootable               |
 *   ---------------------------------------------------------------------
 *   | 0x1BF - 0x1C1  | 1st partition Original partition                 |
 *   | (Offset 0x01)  | Little-endian                                    |
 *   |                | 0x1BF[7:0]: Original Head                        |
 *   |                | 0x1C0[5:0]: Original Sector                      |
 *   |                | 0x1C0[7:6]: MSB of Original Cylinder             |
 *   |                | 0x1C1[7:0]: LSB of Original Cylinder             |
 *   ---------------------------------------------------------------------
 *   | 0x1C2          | Partition Type                                   |
 *   | (Offset 0x4)   | 00H: Un-used                                     |
 *   |                | 06H: FAT16 Basic partition                       |
 *   |                | 0BH: FAT32 Basic partition                       |
 *   |                | 05H: Extend partition                            |
 *   |                | 07H: NTFS partition                              |
 *   |                | 0FH: (LBA mode) Extern partition                 |
 *   ---------------------------------------------------------------------
 *   | 0x1C3 - 0x1C5  | 1st partition End CHS                            |
 *   | (Offset 0x5)   | Little-endian                                    |
 *   |                | 0x1C3[7:0]: End Head                             |
 *   |                | 0x1C4[5:0]: End Sector                           |
 *   |                | 0x1C4[7:6]: MSB of End Cylinder                  |
 *   |                | 0x1C5[7:0]: LSB of End Cylinder                  |
 *   ---------------------------------------------------------------------
 *   | 0x1C6 - 0x1C9  | Used Sector for 1st partition                    |
 *   | (Offset 0x8)   |                                                  |
 *   ---------------------------------------------------------------------
 *   | 0x1CA - 0x1CD  | Total Sector for 1st partition                   |
 *   | (Offset 0xC)   |                                                  |
 *   .-------------------------------------------------------------------.
 *
 */
static void parse_partition_table(struct partition *p)
{
    if (!parse_partition_type(p)) {
        printk("Partition doesn't exist\n");
        return;
    }
    printk("===== Partition Table =====\n");
    if (p->boot_ind)
        printk("Partition is Bootable\n");
    else
        printk("Partition is Un-Bootable\n");
    printk("Partition type:    %#x\n", parse_partition_type(p));
    printk("Original Head:     %#x\n", p->head);
    printk("Original Cylinder: %#x\n",
          ((p->sector & 0xC0) << 0x2) + p->cyl);
    printk("Original Sector:   %#x\n", p->sector & 0x3F);
    printk("End Head:          %#x\n", p->end_head);
    printk("End Cylinder:      %#x\n", 
          ((p->end_sector & 0xC0) << 0x2) + p->end_cyl );
    printk("End Sector:        %#x\n", (p->end_sector) & 0x3F);
    printk("Used sectors:      %#x\n", p->end_sector);
    printk("Total sector:      %#x\n", p->nr_sects);
    printk("==========================\n");
}

/* common MBR entry */
int debug_block_disk_MBR_common(void)
{
    if (1) {
        struct buffer_head *bh;    
    
        bh = obtain_MBR();
        parse_partition_table(MBR_partition0);
        brelse(bh);
    } else {
        struct buffer_head *bh;    
    
        bh = obtain_MBR();
        parse_partition_table(MBR_partition0);
        brelse(bh);
    }
    return 0;
}
