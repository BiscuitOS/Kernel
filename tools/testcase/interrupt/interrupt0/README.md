Interrupt 0 -- Divide Error Exception (#DE)
----------------------------------------------------

### Description

  Indicates the divisor operand for a DIV or IDIV instruction is 0 or that
  the result cannot be represented in the number of bits specified for
  the destination operand.

### Exception Error Code

  None

### Saved Instruction Pointer

  Saved contents of CS and EIP registers point to the instruction that
  generated the exception.

### Program State Change

  A program-state change does not accompany the divide error, because
  the exception occurs before the faulting instruction is executed.

### File list

  * interrupt0.c

    Common entry for triggering interrupt 0 #DE to be invoked by top
    interface.

  * soft-interrupt.c

    Describe how to trigger interrupt 0 #DE through soft-interrupt routine.

  * over_destination.c

    Describe the situation that result overflow destination register 
    when performing `add` instruction.
    
  * divi_zero.c

    Describe the situation that performing divide and divied by zero.

  * README.md

    Describe the basic information for interrupt 0 #DE.

### Usage on BiscuitOS

  The kernel of BiscuitOS supports debug Interrupt0 online. Developer utilize
  `qemu` and `Kbuild` to debug #DE that contains specific triggered condition.
  please follow these action:

  1. Specify triggered condition with `Kbuild`.

     On top of kernel source tree, utilize `make menuconfig` to configure
     specific triggered condition for #DE. as follow:

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
     selecting `Interrupt 0 - Divide Error Exception (#DE)`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_TOP.png)

     Finally, set `Interrupt 0 - Divide Error Exception` as `Y`
     and choose a specific triggered condition to trigger #DE.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT1_MENU.png)

  2. Running and Debugging #DE

     The system default enable running BiscuitOS on `qemu`, so develper can
     running and debugging #9 as follow:

     ```
       cd */BiscuitOS/kernel
       make
       make start
     ```

     The Running-Figure as follow:

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/interrupt/INT_INT1_RUN.png)

  3. Demo Code

     Developer can refer and review demo code on kernel procedure to debug or 
     prevent #DE. The demo code will indicate the triggered condition for #DE
     on kernel, so developer should review demo code details and prevent 
     #DE on your procedure. For example:

     ```
       /*
        * Tarigger Interrupt 0: Overflow EAX/AX
        * EAX or AX can't store a rightful result-value when 
        * execute a divide operation, eg:
        * _rax / 0x01 = 0xFFF, and 'AL' can't store result '0xFFF'
        */
       void trigger_interrupt0(void)
       {
           unsigned short _rax = 0xfff;
           unsigned short _res;

           printk("Test interrupt 0: over EAX/AX after divide.\n");
           __asm__ ("mov %1, %%ax\n\t"
                    "movb $0x01, %%bl\n\t"
                    "div %%bl\n\t"
                    "movb %%al, %0"
                  : "=m" (_res) : "m" (_rax));
           printk("The result %#x\n", _res);
       }
     ```
