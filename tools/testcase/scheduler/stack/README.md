Stack
-----------------------------------------------

  The stack is a contiguous array of memory locations. It is contained
  in a segment and identified by the segment selector in the SS register.
  When using the flat memory model, the stack can be located anywhere
  in the linear address space for the program. A stack can be up to 4GB
  long, the maximum size of a segment.

  Items are placed on the stack using the PUSH instruction and removed
  from the stack using the POP instruction. When an item is pushed onto
  the stack, the processor decrements the ESP register, then write the
  item at the new top of stack. When an item is popped off the stack,
  the processor read the item from the top of stack, then increments 
  the ESP register. In this manner, the stack grows down in memory (
  towards lesser address) when items are pushed on the stack and shrinks
  up (towards greater addresses) when the items are popped from the stack.

  A program or operating system/executive can set up many stacks. For
  example, in multitasking system, each task can be given its own stack.
  The number of stacks in a system is limited by the maximum of segments
  and the available physical memory.

  When a system sets up many stacks, only one stack -- the current stack
  is available at time. The current stack is the one contained in the 
  segment referenced by the SS register.

  The processor references the SS register automatically for all stack
  operations. For example, when the ESP register is used as a memory
  address, it automatically points to an address in the current stack.
  Also, the CALL, RET, PUSH, POP, ENTER, and LEAVE instruction all
  perform operations on the curremt stack.

### Debug on BiscuitOS

  BiscuitOS support online debug Stack on `qemu`, developer should open
  Kernel-macro when configure kernel. Shortly, follow these step to
  enable Stack on system.

  1. Enale specifical Kernel-macro

     Invoke `make menuconfig` on top on source tree, and enable
     specifical item as follow figures.

     ```
       make menuconfig
     ```

     First, select `Kernel hacking`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/BiscuitOS_common_Kbuild.png)

     Then, set `Debug/Running kernel` as `Y` and select `TestCase
     configuration`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/kernel_hacking.png)

     Next, set `Testcase for kernel function` as `Y` and select
     `Task scheduler`

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/TestCase.png)

     Finally, set `Test Task scheduler` and `Stack: SS, ESP, EBP, Near call,
     Far call and Switching Stack` as `Y`.

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/task/Testcase_TASK_stack.png)

  2. Enable debug demo code

     The main code for Stack on `*/tools/testcase/scheduler/stack/stack.c`,
     Developer can add test code on `debug_stack_common`, such as:

     ```
       void debug_stack_common(void)
       {

           if (1) {
               diagnose_stack_pointer();
               stack_link_pointer();
               stack_near_call();
               stack_far_call();
               stack_far_return();
               stack_paramenter_pass();
               save_procedure_state();
               if (0) { /* #GP */
                   /* Establish a new stack */
                   establish_kernel_stack();
               }
           }
           /* ignore warning */
           if (0) {
               diagnose_call();
               stack_near_return();
               parse_paramter_from_register();
               parse_paramter_from_stack(1, 2, 3, 4);
               parse_argument_list(argument);
               parse_popa(0, 0, 0, 0, 0, 0, 0, 0);
           }
       }
     ```

  3. Running test code

     If you configure correctly, you can run Stack demo code on qemu,
     such as:

     ```
       make
       make start
     ```

     ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/task/Stack_running.png)

