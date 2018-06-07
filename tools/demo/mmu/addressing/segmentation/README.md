Segmentation Mechanism on X86 architecture
---------------------------------------------------

  The segmentation mechanism support by the IA-32 architecture can be used
  to implement a wide variety of system designs. These designs range from
  flat models that make only minimal use of segmentation to protect 
  programs to multi-segmented models that employ segmentation to create
  a robust operating environment in which multiple programs and tasks can
  be executed reliably.

  The following sections give several examples of how segmentation can
  by employed in a system to improve memory management performance and
  reliability.

### Basic Flat Mode

  The simplest memory model for a system is the basic `flat model`, in which
  the operating system and application program have access to a continuous,
  unsegment address space. To the greatest extent possible, this basic flat
  model hides that segmentation mechanism of the architecture from both the
  system designer and the application programmer.

  To implement a basic flat memory model with the IA-32 architecture, at
  least two segment descriptor must be created, one for referencing a code
  segment and one for referencing a data segment (See Figure 3-2). Both of 
  these segments, however, are mapped to entire linear address space: that
  is, both segment descriptors have the same address value of 0 and the
  same segment limit of 4 GBytes. By setting the segment limit ot 4 GBytes,
  the segmentation mechanism is kept from generating exception for out of
  limit memory references, even if no physical memory resides at a particular
  address. ROM (EPROM) is generally located at the top of the physical
  address space, because the processor begins execution at FFFF_FFF0H.
  RAM (DRAM) is placed at the bottom of the address space because the initial
  space base address for the DS data segment after reset initialization is 0.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/mmu/FLAT_MODEL.png)

### Protected Flat Model

  The protected flat model is similar to the basic flat model, except the
  segment limits are set to include only the range of addresses for which
  physical memory actually exists (See Figure 3-3). A general-protection 
  exception (#GP) is then generated on any attempt to access nonexistent
  memory. This model provides a minimum level of hardware protection 
  against some kinds of program bugs.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/mmu/P_FLAT_MODEL.png)

  More complexity can be added to this protected flat model to provide more
  protection. For example, for the paging mechanism to provide isolation 
  between user and supervisor code and data, four segments need to be defined:
  code and data segment at privilege level 3 for the user, and code and data
  segments at privilege level 0 for the supervisor. Usually these segments all
  overly each other and start at address 0 in the linear address space. This
  flat segmentation model along with a simple paging structure can protect
  the operating system from applications, and by adding a separate paging
  structure for each task or process, it can also protect applications from
  each other. Similar designs are used by several popular multitasking 
  operating system.

### Multi-Segment Model

  A multi-segment model (such as the one shown in Figure 3-4) uses the full
  capabilities of the segmentation mechanism to provide hardware enforced
  protection of code, data structures, and programs and tasks. Here, each
  program (or task) is given its own table of segment descriptors and its
  own segments. The segments can be completely private to their assigned
  programs or shared among programs. Access to all segments and to the 
  execution environments of individual programs running on the system is
  controlled by hardware.

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/mmu/MULTI_SEG_MODEL.png)

  Access checks can be used to protect not only against referencing an address
  outside the limit of a segment, but also against performing disallowed
  operations in certain segments. For example, since code segments are 
  designated as read-only segments, hardware can be used to prevent writes
  into code segments. The access rights information created for segments
  can also be used to set up protection rings or levels. Protection levels
  can be used to protect operating-system procedures from unauthorized access
  by application programs.

### Paging and Segmentation

  Paging can be used with any of the segmentation models described in Figure
  3-2, 3-3, and 3-4. The processor's paging mechanism divides the linear
  address space (into which segment are mapped) into pages (as shown in Figure
  3-1). These linear-address-space pages are then mapped to pages in the
  physical address space. The paging mechanism offers several page-level
  protection facilities that can be used with or instead of the segment-
  protection facilities. For example, it lets read-write protection be enforced
  on a page-by-page basis. The paging mechanism also provides two-level
  user-supervisor protection that can also be specified on a page-by-page
  basis.
