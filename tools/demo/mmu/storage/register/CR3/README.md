CR3: Control Register 3
---------------------------------------------

CR3 contains the physcial address of the base of the paging-structure 
hierarchy and two flags (PCD and PWT). Only the most-significant bits (less the
lower 12 bits) of the base address are specified; the lower 12 bit of the 
address are assumed to be 0. The first paging structure must thus be alignment
to a page (4-KByte) boundary. The PCD and PWT flags control caching of that
paging structure in the processor's internal data caches (they do not control
TLB caching of page-directory information).

When using the physical address extension, the CR3 register contains the base
address of the page-directory-pointer table.

![CR3_Register](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000420.png)

* Page-Directory Base field

  The physcial address of the base of the paging-structure hierarchy.

* CR3.PCD

  **Page-level Cache Disable (bit 4 of CR3)** -- Control the memory type used
  to access the first paging structure of current paging-structure hierarchy.

* CR3.PWT

  **Page-level Write-Through (bit 3 of the CR3)** -- Control the memory type 
  used to access the first paging structure of the current paging-structure
  hierarchy.
