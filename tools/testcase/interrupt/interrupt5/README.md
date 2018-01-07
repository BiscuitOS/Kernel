Interrupt 5 -- BOUND Range Exceeded Exception (#BR)
----------------------------------------------------

### Description

  Indicates that a BOUND-range-exceeded fault occurred when a BOUND 
  instruction was executed. The BOUND instruction checks that a signed
  array index is within the upper and lower bounds of an array located
  in memory. If the array index is not within the bounds of the array,
  a BOUND-range-exceeded fault is generated.

### Exception Error Code

  None

### Saved Instruction Pointer

  The saved contents of CS and EIP registers point to the BOUND instruction
  that generated the exception.

### Program State Change

  A program-state change does not accompany the bounds-check fault, because
  the operands for the BOUND instruction are not modified. Returning from
  the BOUND-range-exceeded exception handler causes the BOUND instruction
  to be restarted.

### File list

  * interrupt5.c

    Common entry for triggering interrupt 5 (#BR) to be invoked by top
    interface.
 
  * soft-interrupt.c

    Describe how to trigger interrupt 5 (#BR) through soft-interrupt routine.

  * bound-interrupt.c

    Describe how to trigger interrupt 5 (#BR) through BOUND instrcution.

  * bndcl-interrupt.c

    Describe how to trigger interrupt 5 (#BR) through BNDCL instruction.

  * bndcu-interrupt.c

    Describe how to trigger interrupt 5 (#BR) through BNDCU/BNDCN instruction.

  * README.md

    Describe the basic information for interrupt 5 (#BR).

### Usage on BiscuitOS

  The kernel of BiscuitOS supports debug Interrupt5 online. Developer utilize
  `qemu` and `Kbuild` to debug #BR that contains specific triggered condition.
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
     selecting `Interrupt 5 - Bound Range Exceeded Exception (#BR)`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT5_TOP.png)

     Finally, set `Interrupt 5 - BOUND Range Exceeded Exception (#BR)` as `Y`
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
     prevent #BR. The demo code will indicate the triggered condition for #BR
     on kernel, so developer should review demo code details and prevent 
     #BR on your procedure. For example:

     ```
       void trigger_interrupt5(void)
       {
           int buffer[2] = { 0, 6 };
           int index = 7; /* safe value: 0, 1, 2, 3, 4, 5, 6 */

           printk("Trigger interrupt 5: bound array.\n");
           /*
            * Upper = buffer[1]
            * lower = buffer[0]
            * if index < lower || index > upper
            * Invoke Bound interrupt.
            */
           __asm__ ("lea %0, %%edx\n\t"
                    "movl %1, %%eax\n\t"
                    "boundl %%eax, (%%edx)"
                    :: "m" (buffer), "m" (index));
       }
     ```
