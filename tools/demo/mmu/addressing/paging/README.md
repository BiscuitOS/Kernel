Paging Mechanism
----------------------------------------------

Paging can be used with any of the segmentation models described in Figure. The
processor's paging mechanism divides the linear address space (into which 
segment are mapped) into pages (as shown in Figure). These linear-address-space
pages are then mapped to pages in the physical address space. The paging 
mechanism offers several page-level protection facilities that can be used or
instead of the segment-protection facilities. For example, it lets read-write
protection be enforced on a page-by-page basis. The paging mechanism also 
provides two-level user-supervisor protection that can also be specified on
a page-by-page basis.

## Physical Address Space

In protected mode, the IA-32 architecture provides a normal physical address 
space of 4 GBytes (2^32 bytes). This is the address space that 
