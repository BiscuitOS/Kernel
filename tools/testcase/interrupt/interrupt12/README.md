Interrupt 11 -- Stack Fault Exception (#TS)
----------------------------------------------------

### Description

  Indicates that one of the following stack related conditions was detected:
 
  * A limit violation is detected during an operation that refers to
    the SS register. Operations that can cause a limit violation

### Exception Error Code

  An error code containing the segment selector index for the segment
  descriptor that caused the violation is pushed onto the stack of the
  exception handler. If the EXT flag is set, it indicates that the 
  exception was caused by an event external to the currently running
  program (for example, if an external interrupt handler using a
  task gate attempted a task switch to an invalid TSS).

### Saved Instruction Pointer

  If the exception condition was detected before the task switch was
  carried out, the saved contents of CS and EIP registers point to
  the instruction that invoked the task switch. If the exception condition
  was detected after the task switch was carried out, the saved
  contents of CS and EIP register point to the first instruction of
  the new task.

### Program State Change

  The ability of the invalid-TSS handler to recover from the fault depends
  on the error condition than causes the fault. 

  If an invalid TSS exception occurs during a task switch, it can occur
  before or after the commit-to-new-task point. If it occurs before the
  commit point, no program state change occurs. If it occurs after the 
  commit point (when the segment descriptor information for the new segment
  selectors have been loaded in the segment registers), the processor will
  load all the state information from the new TSS before it generates the 
  exception. During a task switch, the processor first loads all the segment
  registers with segment selectors from the TSS, then checks their contents
  for validity. If an invalid TSS exception is discovered, the remaining
  segment registers are loaded but not checked for validity and therefore 
  may not be usable for referencing memory. The invalid TSS handler should
  not rely on being able to use the segment selectors found in the CS, SS,
  DS, ES, FS and GS register without causing another exception. The exception
  handler should load all segment registers before trying to resume the new
  task. otherwise, general-protection exceptions (#GP) may result later under
  conditions that make diagnosis more difficuit. The Intel recommended way of
  dealing situation is to use a task fo the invalid TSS exception handler.
  The task switch back to the interrupted task from the invalid-TSS 
  exception-handler task will then cause the processor to check the registers
  as it loads them from the TSS.

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
