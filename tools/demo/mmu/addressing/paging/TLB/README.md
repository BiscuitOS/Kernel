Caching Translation Information
----------------------------------------------

The IA-32 architecture may accelerate the address-translation process by 
caching data from the paging structures on the processor. Because the processor
does not ensure that the data it caches are always consistent with the 
structure in memory. It is important for software developers to understand
how and when the processor may cache such data. They should also understand
what actions software can take to remove cached data that may be inconsistent
and when it should do so. This section provides software developers information
about the relevant processor operation.

And next section introduces process-context identifiers (PCIDs), which a 
logical processor may use to distinguish information cached for different
linear-address space.

# Translation Lookaside Buffer (TLBs)

A processor may cache information about the translation of linear addresses in
translation lookaside buffer (TLBs). In general, TLBs contain entries that map
page numbers to page frames; these terms are defined in follow setion. Follow
section describes how information may be caching in TLBs, and gives details of 
TLB usage. The last section explains the globl-page feature, which allow 
sofeware to indicate that certain translation should receive special treatment
when caching in the TLBs.

### Page Number, Page Frames, and Page Offset

This section give details of how the different paging modes translate linear
addresses to physical addresses. Specifically, the upper bits of a linear 
address (called the page number) determine the upper bits of the physcial 
address (called the page frame); the lower bit of linear linear address (called
the page offset) determine the lower bit of the physical address. The boundary
between the page number and the page offset is determined by the page size.
Specifically:

##### 32-bit paging

If the translation does use a PTE, the page size is 4 KBytes and the page 
number comprises bits 32:12 of the linear address.

```
Linear address:
31                                          12 11                 0
+---------------------------------------------+-------------------+
|                                             |                   |
+---------------------------------------------+-------------------+
| <-------------- Page Number --------------> |

Physical address:
31                                          12 11                 0
+---------------------------------------------+-------------------+
|                                             |                   |
+---------------------------------------------+-------------------+
| <-------------- Page Frames --------------> |

Page Size: 4-KBytes
```

### Caching Translations in TLBs

The processor may accelerate the paging process by caching individual
translations in translation lookaside buffers (TLBs). Each entry in a TLB is an
individual translation. Each translation is referenced by a page number. It 
contains the following information from the paging-structure entries used to 
translate linear addresses with the page number:

* The Physical address corresponding to the page number (the page frame).

* The access rights from the paging-structure enties used to translate linear 
  addresses with the page number.

  The logical-AND of the R/W flags.

  The logical-AND of the U/S flags.

* Attributes from a paging-structure entry that identifies the final page frame
  for the page number (either a PTE or paging-structure entry in which the PS
  flag is 1):

  The dirty flag

  The memroy type

TLB entries may contain other information as well. A processor may implement
multiple TLBs, and some of these may be for special purposes, e.g., only for
instruction fetches. Such special-purpose TLBs may not contain some of this
information if it is not necessary. For example, a TLB used only for 
instructure fetches need not contain information about the R/W and dirty flags.

Any TLB entries created by a logical processor are associated with the current
PCID.

Processors need not implement any TLBs. Processors that do implement TLBs may
invalidate any TLB entry at any time. Software should not rely on the existence
of TLBs or on the retention of TLB entries.

### Details of TLB Use

Because the TLBs cache entries only for linear addresses with translations, 
there can be a TLB entry for a page number only if the P flag is 1 and the 
reserved bits are 0 in each of the paging-structure entries used to translate
that page number. In addition, the processor does not cache a translation for
a page number unless the accessed flag is 1 in each of the paging-structure
entries used during translation; before caching a translation, the processor
sets any of these accessed flag that is not already 1.

The processor may cache translations required for prefetches and for accesses
that are a result of speculative exceution that would never actually occur in
the executed code path.

If the page number of a linear address corresponds to a TLB entry associated
with the current PCID, the processor may use that TLB entry to determine the 
page frame, access rights, and other attributes for accesses to that linear
address. In this case, the processor may not actually consult the paging 
structures in memory. The processor may retain a TLB entry unmodified even if
software subsequently modifies the relevant paging-structure enties in memory.

If the paging structures specify a translation using a page larger than 
4 KBytes, some processors may cache multiple smaller-page TLB enties for that 
translation. Each such TLB entry would be associated with a page number 
corresponding to the smaller page size (e.g., bits 47:12 of a linear address
with 4-level paging), even though part of that page number (e.g., bits 20:12)
is part of the offset with respect to the page specified by the paging 
structures. The upper bits of the physical address in such a TLB entry are
dervied from the physical address in the PDE used to create the translation,
while the lower bits come from the linear address of the access for which the 
translation is created. These is no way for software to be aware that multiple
translation for smaller pages have been used for a large page. For example, an
execution of INVLPG for a linear address on such a page invalidates any and all
smaller-page TLB entries for the translation of any linear address on that 
page.

If software modifies the paging structures so that the page size used for a 
4-KByte range of linear addresses changes, the TLBs may subsequently contain
multiple translations for the address range (one for each page size). A 
reference to a linear address in the address range may use any of these
translation. Which translation is used may vary from one execution to another,
and the choice may be implementation-specific.

# Process-Context Identifiers (PCIDs)

Process-context identifiers (PCIDs) are a facility by which a logical processor
may cache information for multiple linear-address space. The processor may 
retain cached information when software switches to a different linear-address
space with a different PCID (e.g., by loading CR3).

A PCID is a 12-bit identifier. Non-zero PCIDs are enabled by setting the PCIDE
flag (bit 17) of CR4. If CR4.PCIDE = 0, the current PCID is always 000H; 
otherwise, the current PCID is the value of bits 11:0 of CR3. Not all 
processors allow CR4.PCIDE to be set to 1.

The processor ensures that CR4.PCIDE can be 1 only in IA-32e mode (thus, 32-bit
paging and PAE paging use only PCID 000H). In addition, software can change
CR4.PCIDE from 0 to 1 only if CR3[11:0] = 000H. These requirement are enforced
by the following limitations on the MOV CR instruction:

* MOV to CR4 causes a general-protection exception (#GP) if it would change
  CR4.PCIDE from 0 to 1 and either IA32_EFER.LMA = 0 or CR3[11:0] != 000H.

* MOV to CR0 causes a general-protection exception if it would clear CR0.PG to 
  0 while CR4.PCIDE = 1.

When a logical processor creates entries in the TLBs and paging-structure 
caches, it associates those entries with the current PCID. When using entires
in the TLB and paging-structure caches to translate a linear address, a logcial
processor user only those entries associcated with the current PCID.

If CR4.PCIDE = 0, a logical processor does not cache information for any PCID
other than 000H. This is because: (1) If CR4.PCIDE = 0, the logical processor
will assoicate any newly cached information with the current PCID, 000H. 
And (2) if MOV to CR4 clears CR4.PCIDE, all cached information is invalidated.

# Global Pages

The In-32 architecture allow for globl pages when the PGE flag (bit 7) is 1 in
CR4. If the G flag (bit 8) is in a paging-structure entry that maps a page (
either a PTE or a paging-structure entry in which the PS flag is 1), any TLB
entry cached for a linear address using that paging-strucutre entry is 
considered to be global. Because the G flag is used only in paging-structure
entries that map a page, and because information from such entries is not cached
in the paging-structure caches, the global-page feature does not affect the 
behavior of the paging-structure caches.

A logical processor may use a globl TLB entry to translation a linear address,
even if the TLB entry is associated with a PCID different from the current 
PCID.

# Invalidation of TLBs and Paging-Structure Caches

The processor may create entries in the TLBs and the paging-structure caches 
when linear addresses are translated,




























