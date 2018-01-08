Interrupt 16 -- X87 FPU Floating-Point Error (#MF)
----------------------------------------------------

### Description

  Indicates that the X87 FPU has detected a floating-point error. The NE
  flag in the reigster CR0 must be set for an interrupt 16 (floating-
  point error exception) to be generated.

### Exception Error Code

  None. The X87 FPU provides its own error information.

### Saved Instruction Pointer

  The saved contents of CS and EIP register point to the floating-point or
  WAIT/FWAIT instruction that was about to be execute when the floating-
  point-error exception was generated. This is not the faulting instruction
  in which the error condition was detected. The address of the faulting
  instruction is contained in the X87 FPU instruction pointer register.

### Program State Change

  A program-state change generally accompanies an X87 FPU floating-point
  exception because the handling of the exeception is delayed until the
  next waiting X87 FPU floating-point or WAIT/FWAIT instruction following
  the faulting instruction. The X87 FPU, howeverm saves sufficient information
  about the error condition to allow recovery from the error and re-execution
  of the faulting instruction if needed.

  In situtions where non-X87 FPU floating-point instructions depend on results
  of an x87 FPU floating-point instruction, a WAIT or FWAIT instruction can
  be inserted in front of a dependent instruction to force a pending X87 FPU
  floating-point exception to be handler before the dependent instruction
  is executed.

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
