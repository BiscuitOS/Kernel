Page-Fault Exception (#PF)
-------------------------------------------------------

Indicates that, with paging enabled (the PG flag in the CR0 register is set),
the processor detected one of the following conditions while using the
page-translation mechanism to translate a linear address to a physical address:

* The P (present) flag in a page-directory or page-table entry needed for the
  address translation is clear, indicating that a page table or the page 
  containing the operand is not present in physical memory.

```
(1) Loss P flag on Page Directory:
            



                                   Page Directory
                                   +------------+
                                   |            |
                                   +----------+-+
                           o------>|          |0|
                           |       +----------+-+
Linear address             |       |            |
+--------------------+     |       +------------+
|                   -|----(+)      |            |
+--------------------+     |       +------------+
                           |       |            |
CR3                        |       +------------+
+--------------------+     |       |            |
|                   -|-----o------>+------------+
+--------------------+

(2) Loss P flag on Page Table:
                                                             Page Table
                                                            +-----------+
                                                            |           |
                                                            +-----------+
                        o--------------------------(+)----->|         |0|
                        |                           |       +-----------+
                        |                           |       |           |
                        |                           |       +-----------+
                        |                           |       |           |
                        |          Page Directory   |       +-----------+
                        |          +------------+   |       |           |
                        |          |            |   |       +-----------+
                        |          +------------+   |       |           |
                        |  o------>|           -|---o------>+-----------+
                        |  |       +------------+
Linear address          |  |       |            |
+--------------------+  |  |       +------------+
|                   -|--o-(+)      |            |
+--------------------+     |       +------------+
                           |       |            |
CR3                        |       +------------+
+--------------------+     |       |            |
|                   -|-----o------>+------------+
+--------------------+
```

* The procedure does not have sufficient privilege to access the indicate 
  page (that is, a procedure running in user mode attempts to access a 
  supervisor-mode page). If the SMAP flag is set in CR4, a page fault may also
  be triggered by code running in supervisor mode that tries to access data at
  a user-mode address. If the PKE flag is set in CR4, the PKRU register may
  cause page faults on data accesses to user-mode addresses with certain 
  protection keys.

* Code running in user mode attempts to write to a read-only page. If the WP
  flag is set in CR0, the page fault will also triggered by code running in
  supervisor mode that tries to write to a read-only page.

* An instruction fetch to a linear address that translates to a physical 
  address in a memory page with the execute-disable bit set (for information
  about the execute-disable bit). If the SMEP flag is set in CR4, a page fault
  will also be triggered by code running in supervisor mode that tries to 
  fetch an instruction from a user-mode access.

* One or more reserved bit in paging-structure entry are set to 1.

* An enclave access violates one of the specified a access-control requirement.

The exception handler can recover from page-not-present conditions and restart
the program or task without any loss of program continuity. It can also restart
the program or task after a privilege violation, but the problem that caused
the privilege violation may be uncorrectable.

## Exception Error Code

Yes (special format). The processor provides the page-fault handler with two
items of the information to aid in diagnosing the exception and recovering 
from it:

* An error code on the stack. The error code for a page fault has a format 
  different from that for other exceptions. The processor establishes the bits
  in the error code as follow:

  **P flag (bit 0)**  -- This flag is 0 if there is no translation for the 
  linear address because the P flag was 0 in one of the paging-structure 
  entries used to translate that address.

  **W/R flag (bit 1)** -- If the access causing the page-fault exception was a
  write, this flag is 1; otherwise, it is 0. This flag describes the access
  casusing the page-fault exception, not the access rights specified by paging.

  **U/S flag (bit 2)** -- If a user-mode access caused the page-fault
  exception, this flag is 1; it is 0 if a supervisor-mode access did so. This
  flag describes the access causing the page-fault exception, not the access
  rights specified by paging.

  **RSVD flag (bit 3)** -- This flag is 1 if there is no translation for the
  linear address because a reserved bit was set in one of the paging-structure
  entries used to translate that address.

  **I/D flag (bit 4)** -- This flag is 1 if the access causing the page-fault
  exception was an instruction fetch. This flag describes the access causing
  the page-fault exception, not the access rights specified by paging.

  **PK flag (bit 5)** -- This flag is 1 if the access causing the page-fault
  exception was a data access to a user-mode address with protection key 
  disallowed by the value of the PKRU register.

  **SGX flag (bit 15)** -- This flag is 1 if the exception is unrelated to 
  paging and result from violation of SGX-specific access-control requirements.
  Because such a violation can occur only if there is no ordinary page fault,
  this flag is set only if the P flag (bit 0) is 1 and the RSVD flag (bit 3)
  and the PK flag (bit 5) are both 0.
