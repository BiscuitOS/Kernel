Access Right mechanism on Paging
--------------------------------------------------

  There is a translation for a linear address if the processes described in
  `paging/README.md` completes and produces a physical address. Whether
  an access is permitted by a translation is determined by the access right
  spcified by the paging-structure entries controlling the translation.
  paging-mode modifies in CR0, CR4, and the IA32_EFER MSR, EFLAG.AC and 
  the mode of the access.

### Determination of Access Rights

  Every access to a linear address is either a `supervisor-mode access` or
  a `user-mode access`. For all instruction fetches and most data accesss,
  this distinction is determined by the current privilege level (CPL):
  accesses made while CPL < 3 are supervisor-mode accesses, while accesses 
  made while CPL = 3 are user-mode accesses.

  Some operations implicitly access system data structures with linear
  addresses. the resulting accesses to those data structures are supervisor-
  mode accesses regardless of CPL. Examples of such accesses include the 
  following: accesses to the global descriptor table (IDT) or local 
  descriptor table (LDT) to load a segment descriptor. Accesses to the 
  interrupt descriptor table (IDT) when delivering an interrupt or exception.
  And accesses to the task-state segment (TSS) as port of a task switch
  or change of CPL. All these accesses are called `implict supervisor-mode
  accesses` regardless of CPL. Other accesses made while CPL < 3 are
  called `explicit supervisor-mode accesses`.

  Access rights are also controlled by the mode of a linear address as
  specified by the paging-structure entries controlling the translation of 
  the linear address. If the U/S flag (bit 2) is 0 in at least one of the
  paging-structure entries, the address is a supervisor-mode address. 
  Otherwise, the address is a user-mode address.

  The following items detail how paging determines access rights:

  * For supervisor-mode accesses

    -- Data may be read (implicitly or explicity) from any supervisor-mode
         address.

    -- Data reads from user-mode pages.

       Access right depends on the value of CR4.SMAP

       ```
         1) If CR4.SMAP = 0, data may be read from any user-mode address with
            a protection key for which read access is permitted.

         2) If CR4.SMAP = 1, access rights depends on the value of EFLAGS.AC
            and whether the access is implicit or explicit:

            -- If EFLAGS.AC = 1 and access is explicit, data may be read from
               any user-mode address with a protection key for which read
               access is permitted.

            -- If EFLAGS.AC = 0 or the access is implicit, data may not be
               read from any user-mode address.

            Another section explains how protection keys are associated with
            user-mode addresses and the accesses that are permitted for each
            protection key.
       ``` 
      
    -- Data writes to supervisor-mode address.

       Access right depends on the value of CR0.WP:

       ```
         1) If CR0.WP = 0, data may be written to any supervisor-mode address.

         2) If CR0.WP = 1, data may be written to any supervisor-mode address
            with a translation for which the R/W flag(bit 1) is 1 in every
            paging-structure entry controlling the translation. Data may not
            be written to any supervisor-mode address with a translation for
            which the R/W flag is 0 in any paging-structure entry controlling
            the translation.
       ```

    -- Data writes to user-mode address.

       Access right depend on the value of CR0.WP

       ```
         1) If CR0.WP = 0, access rights depends on the value of CR4.SMAP:
            
            -- If CR4.SMAP = 0. data may be written to any user-mode 
               address with protection key for which write access is
               permitted.

            -- If CR4.SMAP = 1. access rights depend on the value of
               EFLAGS.AC and whether the access is implicit or explicit:

               * If EFLAGS.AC = 1 and the access is explicit, data may be
                 written to any user-mode address with a protection key
                 for which write access is permitted.

               * If EFLAGS.AC = 0 or the access is implicit, data may not
                 be written to any user-mode address.

         2) If CR0.WP = 1, access rights depend on the value of CR4.SMAP:

            -- If CR4.SMAP = 0, data may be written to any user-mode
               address with a translation for which the R/W flag is 1 in
               every paging-structure entry controlling the translation and
               with a protection key for which write access is permitted.
               Data may not be written to any user-mode address with a
               translation for which the R/W flag is 0 in any paging-structure
               entry controlling the translation.

            -- If CR4.SMAP = 1, access rights depend on the value of 
               EFLAGS.AC and whether the access is implicit and explicit:

               * If EFLAGS.AC = 1 and the access is explicit, data may be
                 written to any user-mode address with a translation for
                 which the R/W flag is 1 in every paging-structure entry
                 controlling the translation and with a protection key for
                 which write access is permitted. Data may not by written to
                 any user-mode address with a translation for which the R/W
                 flag is 0 in any paging-structure entry controlling the 
                 translation.

               * If EFLAGS.AC = 0 or the access is implicit, data may not be
                 written to any user-mode address.

               Another setion explainsh how protection keys are associated
               with user-mode addresses and the accesses that are permitted
               for each protection key.
       ```
    
    -- Instruction fetches from supervisor-mode address.

       ```
         * For 32-bit paging or IA32_EFER.NXE = 0, instructions may be
           fetched from any supervisor-mode address.
       ```

    -- Instruction fetches from user-mode address.

       Access rights depend on the value of CR4.SMAP:

       ```
         * If CR4.SMAP = 0, access rights depend on the paging mode and the
           value of IA32_EFER.NXE

           -- For 32-bit paging or IA32_EFER.NXE = 0, instructions may be
              fetched from any user-mode address.

       ```

  * For user-mode accesses:

    -- Data reads.

       Access rights depend on the mode of the linear address:

       ```
         * Data may be read from any user-mode address with a protection key
           which read access is permitted. Another section explains how 
           protection keys are associated with user-mode addresses and the
           accesses that are permitted for each protection key.

         * Data may not be read from any supervisor-mode address.
       ```

    -- Data writes.

       Access rights depend on the mode of the linear address:

       ```
         * Data may be written to any user-mode address with a translation
           for which R/W flag is 1 in every paging-structure entry controlling
           the translation and with a protection key for which write access
           is permitted. Another section explains how protection keys are
           associated with user-mode address and the accesses that are
           permitted for each protection key.

         * Data may not be written to any supervisor-mode address.
       ```

    -- Instruction fetches.

       Access right depend on the mode of the linear address, the paging mode,
       and the value of IA32_EFER.NXE:

       ```
         * For 32-bit paging or if IA32_EFER.NXE = 0, instructions may be 
           fetched from any user-mode address.

         * Instructions may not be fetched from any supervisor-mode address.

       ```

  A processor may cache information from the paging-structure entries in TLBs
  and paging-structure caches (See Another Section). These structures may
  include information about access rights. The processor may enforce access
  rights based on the TLBs and paging-structure caches instead of on the
  paging structures in memory.

  The fact implicit that, if software modifies a paging-structure entry to
  change access right, the processor might not use that change for a 
  subsequent access to an affected linear address.

### ACCESSED and DIRTY flags

  For any paging-structure entry that is used during linear-address
  translation, bit 5 is the accessed flag. For paging-structure entries that
  map a page (as opposed to referenncing another paging structure), bit 6 is
  the dirty flag. These flags are provided for use by memory-management 
  software to manage the transfer of pages and paging structures into and
  out of physical memory.

  Whenever the processor uses a paging-structure entry as part of 
  linear-address translation, it sets the accessed flag in that entry (if it
  is not already set).

  Whenever there is a write to a linear address, the processor sets the dirty
  flag (if it is not already set) in the paging-structure entry that identifies
  the final physical address for the linear address (either a PTE or a paing-
  structure entry in which the PS flag is 1).

  Memeory-Management software may clear these flags when a page or paging 
  structure is initially loaded into physical memory. These flags are `sticky`,
  meaning that, once set, the processor does not clear them, only software
  can clear them.

  A processor may cache information from the paging-structure entry in TLBs
  and paging-structure caches. This fact implies that, if software changes
  an accessed flag or a dirty flag from 1 to 0, the processor might not set
  the corresponding bit in memory on a subsequent access using an affected
  linear address.
