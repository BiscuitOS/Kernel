Interrupt 9 -- Coprocessor Segment Overrun
----------------------------------------------------

### Description

  Indicates that an Intel386 CPU-based system with an Intel 387 math
  coprocessor detected a page or segment violation while transferring
  the middle portion of an Intel 387 math coprocessor operand.

### Exception Error Code

  None.

### Saved Instruction Pointer

  The saved contents of CS and EIP registers point to the instruction
  that generated the exception.

### Program State Change

  A program-state following a coprocessor segment-overrun exception
  is undefined. The program or task cannot be resumed or restarted. The
  only available action of the exception handler is to save the 
  instruction pointer and reinitialize the X87 FPU using the FNINT 
  instruction.

### File list

  * interrupt9.c

    Common entry for triggering interrupt 9 to be invoked by top
    interface.
 
  * soft-interrupt.c

    Describe how to trigger interrupt 9 through soft-interrupt routine.

  * README.md

    Describe the basic information for interrupt 9.

### Usage on BiscuitOS

  The kernel of BiscuitOS supports debug Interrupt9 online. Developer utilize
  `qemu` and `Kbuild` to debug #9 that contains specific triggered condition.
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
     selecting `Interrupt 9 - Coprocessor Segment Overrun`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_TOP.png)

     Finally, set `Interrupt 9 - Coprocessor Segment Overrun` as `Y`
     and choose a specific triggered condition to trigger #9.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_MENU.png)

  2. Running and Debugging #9

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
     prevent #9. The demo code will indicate the triggered condition for #9
     on kernel, so developer should review demo code details and prevent 
     #DF on your procedure. For example:

     ```
       /*
        * trigger interrupt 9: invoke 'int $0x9'
        *   Note! whatever interrupt is enable or disable, this routine
        *   will trigger interrupt 9.
        */
       void trigger_interrupt9(void)
       {
           printk("Trigger interrupt 9: Coprocessor segment overrun "
                                       "[invoke 'int $0x9]'\n");
           __asm__ ("int $9");
       }
     ```
