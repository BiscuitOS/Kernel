Page-Fault Exceptions
--------------------------------------------------

  Accesses using linear addresses may cause page-fault exceptions (#PF, 
  exception 14). An access to a linear address may cause a page-fault
  exception for either of two reason. (1) there is no translation for the 
  linear address or (2) there is a translation for the linear address, but
  its access rights do not permit the access.

  As noted in Another `mmu/*/README.md`, there is no translation for a
  linear address if the translation process for that address would use
  a paging-structure entry in which the P flag (bit 0) is 0 or one that
  sets a reserved bit. If there is a translation for a linear address, its
  access rights are determined as specified in Another Section.

  Figure 4-12 illustrates the error code that the processor provides on
  delivery of a page-fault exception. The following items explain how the
  bits in the error code describe the nature of the page-fault exception.

  ```
    31-------16--15---------------5-----4------3-----2-----1----0--
    | Reserved | SGX | Reserved | PK | I/D | RSVD | U/S | W/R | P |
    ---------------------------------------------------------------
  ```

  * P flag (bit 0)

    This flag is 0 if there is no translation for the linear address because
    the P flag was 0 in one of the paging-structure entries used to translate
    that address.

    ```
      0 -> The fault was caused by a non-present page.

      1 -> The fault was caused by a page-level protection violation. 
    ```

  * W/R (bit 1)

    If the access causing the page-fault exception was a write, the flag is 1.
    Otherwise, it is 0. This flag describes the access causing the page-fault
    exception, not the access rights specified by paging.

    ```
      0 -> The access causing the fault was a read.

      1 -> The access causing the fault was a write.
    ```

  * U/S (bit 2)

    If a user-mode access caused the page-fault exception, this flag is 1.
    It is 0 if a supervisor-mode access did so. This flag describes the 
    access causing the page-fault exception, not the access rights specified
    by paging. User-mode and supervisor-mode accesses are defined in Another
    Section.

    ```
      0 -> A supervisor-mode access caused the fault.

      1 -> A user-mode access caused the fault.
    ```

  * RSVD flag (bit 3)

    This flag is 1 if there is no translation for the linear address because
    a reserved bit was set in one of the paging-structure entries used to 
    that address. (Because reserved bits are not checked in a paging-structure
    entry whose P flag is 0, bit 3 of the error code can be set only if bit
    0 is also set.)

    Bits reserved in the paging-structure entries are reserved for future 
    functionality. Sotfware developers should be aware that such bits may be
    used in the future and that paging-structure entry that cause a page-fault
    exception on one processor might not do so in the future.

    ```
      0 -> The fault was not caused by reserved bit violation

      1 -> The fault was caused by a reserved bit set to 1 in some 
           paging-structure entry.
    ```

  * I/D flag (bit 4)

    This flag is 1 if (1) the access causing the page-fault exception was an
    instruction fetch. (2) either (a) CR4.SMEP = 1 or (b) both (i) CR4.PAE = 1
    (either PAE paging or 4-level paging is in use) and (ii) IA32_EFER.NXE = 1
    Otherwise, the flag is 0. This flag describes the access causing the
    page-fault exception, not the access rights specified by paging.

    ```
      0 -> The fault was not caused by an instruction fetch.

      1 -> The fault was caused by an instruction fetch.
    ```  

  * PK flag (bit 5)
 
    ```
      0 -> The fault was not caused by protection keys
  
      1 -> There was a protection-key violation.
    ```

  * SGX flag (bit 15)

    ```
      0 -> The fault is not related to SGX.

      1 -> The fault resulted from violation of SGX-specific access-control
           requirements.
    ```

  Page-falut exceptions occur only due to an attempt to use a linear address.
  Failures to load the PDPTE registers with PAE paging cause general-protection
  exceptions (#GP(0)) and not page-fault exceptions.
