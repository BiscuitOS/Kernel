Interrupt 8 -- Double Fault Exception (#DF)
----------------------------------------------------

### Description

  Indicates that the processor detected a second exception while calling
  an exception handler for a prior exception. Normally, when the processor
  detects another exception while trying to call an exception handler,
  the two exceptions can be handler serially. If, however, the processor
  cannot handle them serially, it signals the double-fault exception. To
  determine when two faults need to be signalled as a duble fault, the 
  processor divides the exceptions into three class.

  If another contributory or page fault exception occurs while attempting
  to call the double-fault handler, the processor enters shutdown mode. 
  The mode is similar to the state following execution of an HLT instruction.
  In this mode, the processor stops executing instructions until an NMI 
  interrupt, SMI interrupt, hardware reset, or INT # is received. The 
  processor generates a special bus cycle to indicate that it has entered
  shutdown mode. Software designers may need to be aware of the response
  of hardware when it goes into shutdown mode. For example, hardware may
  turn on an indicator light on the front panel, generate an NMI interrupt
  to record diagnostic information, invoke reset initialization, generate
  an INIT initialization, or generate an SMI. If any events are pending
  during shutdown, they will be handled after an wake event from shutdown
  is processed.

  If a shutdown occurs while the processor is executing an NMI interrupt
  handler, then only a hardware reset can restart the processor. Likewise,
  if the shutdown occurs while executing in SMM, a hardware reset must
  be used to restart the processor.

### Exception Error Code

  Zero. The processor always pushes an error code of 0 onto the stack
  of the double-fault handler.

### Saved Instruction Pointer

  The saved contents of CS and EIP registers are undefined.

### Program State Change

  A program-state following a double-fault exception is undefined. The
  program or task cannot be resumed or restared. The only available action
  of the double-fault exception handler is to collect all possible context
  information for use in diagnostics and then close the application and/or
  shut down or reset the processor.

  If the double fault occurs when any portion of the excetpion handling
  machines state is corrupted, the handler cannot be invoked and the 
  processor must be reset.

### File list

  * interrupt8.c

    Common entry for triggering interrupt 8 (#DF) to be invoked by top
    interface.
 
  * soft-interrupt.c

    Describe how to trigger interrupt 8 (#DF) through soft-interrupt routine.

  * README.md

    Describe the basic information for interrupt 8 (#DF).

### Usage on BiscuitOS

  The kernel of BiscuitOS supports debug Interrupt8 online. Developer utilize
  `qemu` and `Kbuild` to debug #DF that contains specific triggered condition.
  please follow these action:

  1. Specify triggered condition with `Kbuild`.

     On top of kernel source tree, utilize `make menuconfig` to configure
     specific triggered condition for #BR. as follow:

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
     selecting `Interrupt 8 - Double Fault Execption (#DF)`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_TOP.png)

     Finally, set `Interrupt 8 - Double Fault Exception (#DF)` as `Y`
     and choose a specific triggered condition to trigger #BR.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_MENU.png)

  2. Running and Debugging #DF

     The system default enable running BiscuitOS on `qemu`, so develper can
     running and debugging #DF as follow:

     ```
       cd */BiscuitOS/kernel
       make
       make start
     ```

     The Running-Figure as follow:

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_RUN.png)

  3. Demo Code

     Developer can refer and review demo code on kernel procedure to debug or 
     prevent #DF. The demo code will indicate the triggered condition for #DF
     on kernel, so developer should review demo code details and prevent 
     #DF on your procedure. For example:

     ```
       /*
        * trigger interrupt 8: invoke 'int $0x8'
        *   Note! This routine will trigger interrupt 8 whatever interrupt is
        *   enable or disable. 
        */
       void trigger_interrupt8(void)
       {
           printk("Trigger interrupt 8: duble fault.\n");
           __asm__ ("int $8");
       }

     ```
