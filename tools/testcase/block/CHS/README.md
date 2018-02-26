Disk Storage
--------------------------------------------------------

  Disk Storage (also sometimes called the drive storage) is a general
  category of storage mechanisms where data is recorded by various 
  electronic, magnetic, optical, or mechanical changes to a surface
  layer of one or more rotating disks. A disk drive is a device
  implementing such a storage mechanism. Notable types are the hard
  disk drive (HDD) containing a nonremovable disk, the floppy disk,
  and various optical disk drive (ODD) and associated optical disk
  media.

  The spelling disk and disk are used interchangeably except where
  trademarks preclude one usage, e.g. the Compact Disk logo. The 
  choice of a particular form is frequently historical, as in IBM's
  usage of the disk form begining in.

#### Cylinder - Hear - Sector (CHS)

  Cyliner-Head-Sector (CHS) is an early method for giving address to
  each physical block of data on a hard disk drive.

  Early hard drives didn't come with an embedded disk controller. A
  separate controller card was used, and the operating system had to
  know the exact physical `geometry` of the drive behind the controller
  to use it. As the geometry became more complicated and drive sizes
  grew over time, the CHS addressing method became restrictive. Since
  1980s, hard drive begun shipping with an embedded disk controller 
  that had good knowledge of the physical geometry. They would report
  a false geometry to the computer, e.g. A larger number of heads than
  actually present, to gain more addressable space. These logical CHS
  values would be translated by the controller, thus CHS addressing no
  longer corresponded to any physical attributes of the drive.

  Soon after, hard drive interfaces replaced the CHS scheme with logical
  block addressing, but many tools for manipulating the master boot
  record (MBR) partition table still aligned partitions to cylinder
  boundaries. Thus, artifacts of the CHS addressing were still seen in
  partiting programs in the 2010s.

  In early 2010s, the disk size limitations imposed by MBR became
  problematic and the GUID Partition table (GPT) was designed as a
  replacement, modern computers without MBR no longer use any notions
  from CHS addressing.

  Disk contains multip `Platters` where like chip of watermelon. At some time,
  `Platter` divie into multip concentric circles along the Radii, and it named
  `Track`. Each `Track` are cut in multip region that name `Sector` (`Sector`
  is base metadata for Disk Reading/Writing, that 512 Bytes). Cylinder 
  is comprised with track that same radii and different Platter. More 
  metadata of Disk e.g:

  * Head Number

    Each Platter contains positive and negative, that corresponding Head 0 
    and Head 1, total 2 head for each Platter. So Head Number is number of
    head on all Platter.

  * Track Number

    Track is numbered from outside to inside in Platter, the most outside track
    is Track 0, then Track 1 ..... The most inside track is used to place
    head not store data. So Track Number is number of tack on a Platter.

  * Cylinder Number

    The number of track that comprises same radii on all Platter.

  * Sector number

    Each track is cut in multip area that contain same byte (512 Byte).
    It is metadata for Reading/Writing on Disk. Sector number is number of
    sector on a track.

  * Platter Number

    Platter Number is number of Platter on Disk.

  So, the volume of disk is:

  ```
    Volume = Head * Cylinder * Sector * 512Bytes
    or
    Volume = Head * Track * Sector * 512Bytes 
  ```
