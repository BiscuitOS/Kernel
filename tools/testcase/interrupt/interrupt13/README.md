Interrupt 13 -- General Protection Exception (#GP)
----------------------------------------------------

### Description

  Indicates that the processor detector one of a class of protection 
  violations called `general-protection violations`. The conditions
  that cause this exception to be generated comprise all the protection
  violations that do not cause other exception to be generated (such as,
  invalid-TSS, segment-not-present, stack-fault, or page-fault exceptions).
  The following conditions cause general-protection exceptions to be 
  generated:

  * Exceeding the segment limit when asscessing the CS, DS, ES, FS, or
    GS segments.

  * Exceeding the segment limit when referencing a descriptor table (
    except during a task switch or a task switch).

  * Transferring execution to a segment that is not executable.

  * Writing to code segment or a read-only data segment.

  * Reading from an execute-only code segment.

  * Loading the SS register with a segment selector for a read-only segment(
    unless the selector comes from a TSS during a task switch, in which
    cause an invalid-TSS exception occurs).

  * Loading the SS, DS, ES, FS or GS register with a segment selector for
    system segment.

  * Loading the DS, ES, FS or GS register with a segment selector for an
    execute-only code segment.

  * Loading the CS register with a segment selector for a data segment or
    a null segment selector.

  * Loading the SS register with the segment selector of an executeable
    segment or a null segment selector.

  * Accessing memory using the DS, FS, ES, or GS register when it contains
    a null segment selector.

  * Switching to a busy task during a call or jump to a TSS.

  * Using a segment selector on a non-IRET task switch that points to a TSS
    descriptor in the current LDT. TSS descriptors can only reside in the 
    GDT. This condition causes a #TS exception during an IRET task switch.

  * Violating any of the privilege rules described.

  * Exceeding the instruction length limit of 15 bytes (this only can occur
    when redundant prefixes are placed before an instruction).

  * Loading the CR0 register with a set PG flag (paging enable) and a clear
    PE flag (protection disabled).

  * Loading the CR0 register with a set NW flag and a clear CD flag.

  * Referencing an entry in the IDT (following an interrupt or exception)
    that is not an interrupt, trap, or task gate.

  * Attempting to access an interrupt or exception handler through an 
    interrupt or trap gate from virtual-8086 mode when the handler's code
    segment DPL is greater than 0.

  * Attempting to write a 1 into a reserved bit of CR4.

  * Attempting to execute a privileged instruction when the CPL is not equal
    to 0.

  * Attempting to execute SGDT, SIDT, SLDT, SMSW, or STR when CR4.UMIP = 1
    and the CPL is not equal to 0.

  * Writing a gate that contains a null segment selector.

  * Executing the INT n instruction when the CPL is greater than the DPL
    of the referenced interrupt, trap, or task gate.

  * The segment selector in a call, interrupt, or trap gate does not point
    to a code segment.

  * The segment selector operand in the LLDT instruction is a local type (
    TI flag is set) or does not point to a segment descriptor of the LDT
    type.

  * The segment selector operand in the LTR instruction is local or points
    to a TSS that is not available.

  * The target code-segment selector for a call, jump, or return is null.

  A program or task can be restarted following any general-protection 
  exception. If the exception occurs while attempting to call an interrupt
  handler, the interrupted program can be restartable, but the interrupt
  may be lost.

### Exception Error Code

  The processor pushes an error code onto the exception handler's stack.
  If the fault condition was detected while loading a segment descriptor,
  the error code contains a segment selector to or IDT vercotr number
  for the descriptor. Otherwise, the error code is 0. The source of the
  selector in an error code may by any of the following:

  * An operand of the instruction

  * A selector from a gate which is the operand of the instruction.

  * A selector from a TSS involved in a task switch.

  * IDT vector number.

### Saved Instruction Pointer

  The saved contents of CS and EIP register point to the instruction that
  generated the exception.

### Program State Change

  In general, a program-state change does not accompany a general-protection
  exception, because the invalid instruction or operation is not executed. An
  exception handler can be designed to correct all of the conditions that
  cause general-protection exceptions and restart the program or task without
  any loss of program continuity.

  If a general-protection exception occurs during a task switch, it can occur
  before or after the commit-to-new-task point. If it occurs before the 
  commit point, no program state change occurs. If it occurs after the 
  commit point, the processor will load all the state information from
  the new TSS (without performing any additional limit, present, or type
  checks) before it generates the exception. The general-protection exception
  handler should thus not rely on being able to use the segment selectors 
  found in the CS, SS, DS, ES, FS, and GS registers without causing another
  exception.

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
