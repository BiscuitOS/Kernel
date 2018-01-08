Interrupt 14 -- Page-Fault Exception (#PF)
----------------------------------------------------

### Description

  Indicates that, with paging enable (the PG flag in the CR0 register is
  set), the processor detected one of the following conditions while
  using the page-translation mechanism to translate a linear address to
  a physical address:

  * The P (present) flag in a page-director or page-table entry needed
    for the address translation is clear, indicating that a page table
    or the page containing that operand is not present in physical 
    memory.

  * The procedure does not have sufficient privilege to access the indicated
    page (that is, a procedure running in user mode attempts to access a
    supervisor-mode page). If the SMAP flag is set in CR4, a page fault
    may also be triggered by code running in supervisor mode that tries
    to access data at a user-mode address. If the PKE flag is set in CR4,
    the PKRU register may cause page faults on data accesses to user-mode
    addresses with certain protection keys.

  * Code running in user mode attempts to write to a read-only page. If 
    the WP flag is set in CR0, the page fault will also be triggered by 
    code running in supervisor mode that tries to write to a read-only 
    page.

  * An instruction fetch to a linear address that translates to a physical
    address in a memory page with the execute-disable bit set (for
    information about the execute-disable bit). If the SMEP flag is
    set in CR4, a page fault will also be triggered by code running
    in supervisor mode that tries to fetch an instruction from a user-
    mode address.

  * One or more reserved bits in paging-structure entry are set to 1.
    See description below of RSVD error code flag.

  The exception handler can recover from page-not-present conditions and
  restart the program or task without any loss of program continuity.
  It can also restart the program or task after a privilege violation,
  but the problem that caused that privilege violation may be uncorrectable.

### Exception Error Code

  Yes (special format). The processor provides that page-fault handler with
  two item of information to aid in diagnosing the exception and recovering
  from it:

  An error code on the stack. The error code for a page fault has a 
  format different from that for other exceptions. The processor establishes
  the bits in the error code as follow:

  * P flag (bit 0)

    This flag is 0 if there is no translation for the linear address because
    the P flag was 0 in one of the paging-structure entries used to 
    translate that address.

  * W/R (bit 1)

    If the access causing the page-fault exception was a write, this flag is
    one. Otherwise, it is 0. This flag describes the access causing the 
    page-fault exception, not the access rights specified by paging.

  * U/S (bit 2)

    If a user-mode access caused the page-fault exception, this flag is one.
    It is 0 if a supervisor-mode access did so. This flag describes the 
    access causing the page-fault exception, not the access right specified
    by paging.

  * RSVD flag (bit 3)

    This flag is one if there is no translation for the linear address
    because a reserved bit was set in one of the paging-structure entries
    used to translate that address.

  * I/D flag (bit 4)
    
    This flag is one if the access causing the page-fault exception was
    an instruction fetch. This flag describes the access causing the
    page-fault exception, not the access rights specified by paging.

  * PK flag (bit 5)

    This flag is 1 if the access causing the page-fault exception was an
    instruction fetch. This flag describes the access causing the page-fault
    exception, not the access rights specified by paging.

  * SGX flag (bit 15)

    This flag is 1 if the exception is unrelated to paging and resulted 
    from violation of SGX-specific access-control requirements. Because
    such a violation can occur only if there is no ordinary page fault,
    this flag is set only if the P flag (bit 0) is 1 and the RSVD flag (
    bit 3) and the PK flag (bit 5) are both 0.

  The contents of the CR2 register. The processor loads the CR2 register
  with the 32-bit linear address that generated the exception. The page-
  fault handler can use this address to locate the corresponding page
  directory and page-table entires. Another page fault can potentially
  occur during execution of the page-fault handler. The handler should
  save the contents of the CR2 register before a second page fault can occur.
  If a page fault is caused by a page-level protectuion violation, the
  access flag in the page-directory entry is set when the fault occurs.
  The behavior of IA-32 processors regarding the access flag in the 
  corresponding page-table entry is model specific and not architecturally
  defined.

### Saved Instruction Pointer

  The saved contents of CS and EIP register generally point to the 
  instruction that generated the exception. If the page-fault exception
  occurred during a task switch, the CS and EIP register may point to the
  first instruction of the new task.
 
### Program State Change

  A program-state change does not normally accompany a page-fault exception,
  because the instrcution that causes the exception to be generated is not
  exceuted. After the page-fault exception handler has corrected the 
  violation (for example, loaded the missing page into memory), execution of
  the program or task can be resumed.

  When a page-fault exception is generated during a task switch, the program-
  state may change, as follows. During a task switch, a page-fault exception
  can occur during any of following operations:

  * While writing the state of the original task into the TSS of that task.

  * While reading the GDT to locate the TSS descriptor of the new task.

  * While reading the TSS of the new task.

  * While reading segment descriptors associated with segment selector
    from the new task.

  * While reading the LDT of the new task to verify the segment register
    stored in the new TSS.

  In the last two cases the exception occurs in the contents of the new task.
  The instrcution pointer refers to the first instruction of the new task,
  not to the instruction which caused the task switch (or the last instruction
  to be executed, in the cause of an interrupt). If the design of the 
  operating system permits page faults to occur during task-switches, the 
  page-fault handler should be called through a task gate.

  If a page fault occur during a task switch, the processor will load all
  the state information from the new TSS (without performing any additional
  limit, present, or type checks) before it generates the exception. The 
  page-fault handler should thus not rely on being able to use the segment
  selectors found in the CS, SS, DS, ES, FS and GS registers without causing
  another exception.

### Additional Exception-Handling Information

  Specical care should be taken to ensure that an exception that occurs
  during an explicit stack switch does not cause the processor to use an
  invalid stack pointer (SS:ESP). Software written for 16-bit IA-32 
  processors often use a pair of instruction to change to a new stack, for
  example:

  ```
    MOV SS, AX
    MOV SP, Stack Top
  ```
  
  When executing this code on one of the 32-bit IA-32 processor, it is 
  possible to get a page fault, general-protection fault (#GP), or 
  alignment check fault (#AC) after the segment selector has been loaded
  into the SS register but before the ESP register has been loaded. At
  this point, the two parts of the stack pointer (SS and ESP) are 
  inconsistent. The new stack segment is being used with the old stack
  pointer.

  The processor does not use the inconsistent stack pointer if the exception
  handler switches to a well defined stack (that is, the handler is a task
  or more privileged procedure). However, if the exception handler is called
  at the same privilege level and from the same task, the processor will
  attempt to use the inconsistent stack pointer.

  In systems that handler page-fault, general-protection, or alignment check
  exceptions with the faulting task (with trap or interrupt gates), software
  exception at the same privilege level as the exception handler should
  initialize a new stack by using the LSS instruction handler is running at
  privilege level 0 (the normal case), the problem is limited to procedures
  or tasks that run at privilege level 0, typically the kernel of the 
  operating system.

### File list

  * interrupt10.c

    Common entry for triggering interrupt 10 (#TS) to be invoked by top
    interface.
 
  * soft-interrupt.c

    Describe how to trigger interrupt 10 (#TS) through soft-interrupt routine.

  * README.md

    Describe the basic information for interrupt 10 (#TS).

### Usage on BiscuitOS

  The kernel of BiscuitOS supports debug Interrupt10 online. Developer utilize
  `qemu` and `Kbuild` to debug #TS that contains specific triggered condition.
  please follow these action:

  1. Specify triggered condition with `Kbuild`.

     On top of kernel source tree, utilize `make menuconfig` to configure
     specific triggered condition for #TS. as follow:

     ```
       cd */BiscuitOS/kernel
       make menuconfig
     ```

     Then, developer will obtain figure as follow, please choose `Kernel 
     hacking`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/BiscuitOS_common_Kbuild.png)

     The next figure, set `Debug/Running kernel` as `Y` and select `TestCase
     configuration`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/kernel_hacking.png)

     Now, set `Testcase for kernel function` as `Y` and select `Interrupt 
     Machanism on X86 Arichitecture`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/TestCase.png)

     Then, set `Debug Interrupt Machanism on X86 Architecture` and go on
     selecting `Interrupt 10 - Invalid TSS Exception (#TS)`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_TOP.png)

     Finally, set `Interrupt 10 - Invalid TSS Exception (#TS)` as `Y`
     and choose a specific triggered condition to trigger #TS.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_MENU.png)

  2. Running and Debugging #TS

     The system default enable running BiscuitOS on `qemu`, so develper can
     running and debugging #9 as follow:

     ```
       cd */BiscuitOS/kernel
       make
       make start
     ```

     The Running-Figure as follow:

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_RUN.png)

  3. Demo Code

     Developer can refer and review demo code on kernel procedure to debug or 
     prevent #TS. The demo code will indicate the triggered condition for #TS
     on kernel, so developer should review demo code details and prevent 
     #TS on your procedure. For example:

     ```
       /*
        * Trigger interrupt 10: invoke 'int $0xA'
        *   Note! whatever interrupt is enable or disable, this routine
        *   will trigger interrupt 10.
        */
       void trigger_interrupt10(void)
       {
           printk("Trigger interrupt 10: invalid TSS segment "
                               "[invoke 'int $0xA']\n");
           __asm__ ("int $0xA");
       }
     ```
