Interrupt 6 -- Invalid Opcode Exception (#UD)
----------------------------------------------------

### Description

  Indicate that the processor did one of the following things:

  * Attempted to execute an invalid or reserved opcode.

  * Attempted to execute an instruction with an operand type that is
    invalid for its accompanying opcode. for example, the source 
    operand for a LES instruction is not a memory location.

  * Detected a LOCK prefix that precedure an instruction that may
    not be locked or one that may be locked but the destination
    operand is not a memory location.

  * Attempted to execute an LLDT, SLDT, LTR, STR, LSL, LAR, VERR,
    VERW, or ARPL instruction while in read-address or virtual-8086 mode.

### Exception Error Code

  None

### Saved Instruction Pointer

  The saved contents of CS and EIP registers point to instruction that
  generated the exception.

### Program State Change

  A program-state change does not accompany the bounds-check fault, because
  the invalid instruction is not executed.

### File list

  * interrupt6.c

    Common entry for triggering interrupt 6 (#UD) to be invoked by top
    interface.
 
  * soft-interrupt.c

    Describe how to trigger interrupt 6 (#UD) through soft-interrupt routine.

  * bound-interrupt.c

    Describe how to trigger interrupt 6 (#UD) through BOUND instrcution.

  * README.md

    Describe the basic information for interrupt 5 (#BR).

### Usage on BiscuitOS

  The kernel of BiscuitOS supports debug Interrupt6 online. Developer utilize
  `qemu` and `Kbuild` to debug #UD that contains specific triggered condition.
  please follow these action:

  1. Specify triggered condition with `Kbuild`.

     On top of kernel source tree, utilize `make menuconfig` to configure
     specific triggered condition for #UD. as follow:

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
     selecting `Interrupt 6 - Invalid Opcode Exception (#UD)`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_TOP.png)

     Finally, set `Interrupt 6 - Invalid Opcode Exception (#UD)` as `Y`
     and choose a specific triggered condition to trigger #BR.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_MENU.png)

  2. Running and Debugging #BR

     The system default enable running BiscuitOS on `qemu`, so develper can
     running and debugging #BR as follow:

     ```
       cd */BiscuitOS/kernel
       make
       make start
     ```

     The Running-Figure as follow:

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_RUN.png)

  3. Demo Code

     Developer can refer and review demo code on kernel procedure to debug or 
     prevent #UD. The demo code will indicate the triggered condition for #UD
     on kernel, so developer should review demo code details and prevent 
     #BR on your procedure. For example:

     ```
       /*
        * trigger interrupt 6: invoke 'int $0x6'
        *   Note! whatever interrupt is enable or disable, this routine
        *   will trigger interrupt 6.
        */
       void trigger_interrupt6(void)
       {
           printk("Trigger interrupt 6: invoke 'int $0x6'\n");
           __asm__ ("int $6");
       }
     ```
