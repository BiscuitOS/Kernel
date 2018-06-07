System call Mechanism
--------------------------------------------------

  ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/systemcall/TestDays.png)

  In computing, a system call is the programmatic way in which a computer 
  program requests a service from the kernel of the operating system it is 
  executed on. This may include hardware-related services (for example, 
  accessing a hard disk drive), creation and execution of new processes, 
  and communication with integral kernel services such as process scheduling. 
  System calls provide an essential interface between a process and the 
  operating system.

  In most systems, system calls can only be made from userspace processes, 
  while in some systems, OS/360 and successors for example, privileged 
  system code also issues system calls.

### How to debug system call on BiscuitOS

  Now, BiscuitOS has support all system call from userland to kernel space.
  You can directly debug and use this system call on BiscuitOS. So, please
  follow this step to tour system call mechanism.

  * Change debug stage on userland

    BiscuitOS supports different debug stage that contain kernel, userland
    and so on. According the mechanism of system call, the program should
    call 0x80 interrupt to invoke a system call, such as `open`, `read` 
    and `write` that locate on userland. The kernel will trigger a interrupt
    when userland invoke system call. When kernel obtain information about
    system call, it will call handler of system call.

    If you want to debug the routine for special system call. First, you 
    should configure correct debug environment. so, we should run command
    `make menuconfig` on topmost of kernel source tree. As follow figure:

    ```
      cd *//BiscuitOS/kernel
      make menuconfig
    ```

    Then, developer will obtain figure as follow, please choose `Kernel
    hacking`.

    ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/BiscuitOS_common_Kbuild.png)

    The next figure, set `Debug/Running kernel` as `Y` and select `TestCase
    configuration`

    ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/kernel_hacking.png)

    Now, set `Testcase for kernel function` as `Y` and select `System CAll
    Mechanism`

    ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/TestCase.png)

    Then, set `Debug System CAll Mechanism on X86 Architecture` and go on
    selecting `open   Open file/Directory/socket/pipe etc`.

    ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/systemcall/systemcall.png)

    Finally, set `open(): open file/Directory/socket/pipe etc` as `Y`
    and choose `System CALL: Open (linux0.11.1)`.

    ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/systemcall/open.png)


  * Running special system call

    After configure a special system call, you can run and debug special
    system call as follow:

    ```
      cd *//BiscuitOS/kernel
      make
      make start
    ```

    ![Alt text](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel_hacking/testcase/systemcall/open_run.png)

### More infomation about system call

   Please refer: [opengroup](http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html "opengroup.org")
