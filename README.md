BiscuitOS
----------------------------------------------

![TOP_PIC](https://github.com/EmulateSpace/PictureSet/blob/master/github/mainmenu.jpg)

BiscuitOS is a Unix-like computer operating system that is composed entirely
of free software, most of which is under the GNU General Public License 
and packaged by a group of individuals participating in the BiscuitOS Project.
As one of the earliest operating systems based on the Linux kernel 0.11, 
it was decided that BiscuitOS was to be developed openly and freely 
distributed in the spirit of the GNU Project. 

While all BiscuitOS releases are derived from the GNU Operating System 
and use the GNU userland and the GNU C library (glibc), other kernels 
aside from the Linux kernel are also available, such as those based on 
BSD kernels and the GNU Hurd microkernel.

## To Start

  Before you start this tour, you need install core toolchain on
  your host PC (such as Ubuntu16.04), so please install these tools
  on start.

  ```
    sudo apt-get install qemu gcc make gdb git
    sudo apt-get install indent

    On 64bit machine:
    sudo apt-get install lib32z1 lib32z1-dev
    sudo apt-get install libncurses5-dev
  ```
  
  First of all, You need get source code of kernel for BiscuitOS from github, 
  follow these steps to get newest and stable branch. The BiscuitOS
  project will help you easy to build a kernel iamge for BiscuitOS.

  ```
    git clone https://github.com/BiscuitOS/Kernel.git
  ```

  The next step, we need to build Kernel with common Kbuild syntax.
  The Kbuild will help you easy to built  kernel. So utilise this command 
  on your terminal.

  ```
    cd */Kernel
    make defconfig
    make
  ```

  If you finish above step, we will get the kernel image, but it doesn't
  work, as you know, a full system need kernel and rootfs. so you should
  get a running system. Don't worry, you can get it from BiscuitOS project.

  ```
    git clone https://github.com/BiscuitOS/BiscuitOS.git
  ```

  The project will help to compiler a BiscuitOS distro named 
  `BiscuitOS-0.11.img`, and copy it on `*/Kernel/`, then running new system:

  ```
    cp -rf */BiscuitOS/output/BiscuitOS-0.11.img */Kernel/
    make start
  ```
  
  Because Kernel project is written on Kbuild syntax, so U can use Kbuild 
  syntax to add/delete or configure your owner kernel. 

  ```
    make menuconfig
    or 
    make xconfig
  ```
### Other

  > Software is like sex. It's better when it's free
  >
  >                      --- Linus Torvalds

