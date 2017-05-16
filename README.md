BiscuitOS
--------------------------------------------

> Hello everybody:
>
> I'm doing an OS for X86.
> This has been brewing since March, I'd like any feedback on things
> people like/dislike in Linux. as my OS resembles it somewhat.
>
> A tiny operate system has work on i386. I'll add more features like 
> virtual file-system, virtual memory and general device driver.
>
> BTW, it's only a toy (just a hobby), I only want to know machanism of linux,
> and won't be big and professional like Linux.
>
>                          Buddy (buddy.zhang@aliyun.com)  ^_^
> 

----------------------------------------------

### Basic information

  OS information:

  | Item                    | contents                      | 
  | ----------------------- | ----------------------------- |
  | OS Name                 | BiscuitOS                     |
  | Release Version         | V0.0.1                        |
  | Target Platform         | I386(X86 family)              |
  | Emulator                | Qemu                          |
  | Memory limit            | 16 M                          |
  | Disk                    | Floppy                        |
  | Rootfs                  | none                          |
  | Host Machine            | Ubuntu16.04                   |
  | Paging                  | Support                       |
  | Segmenting              | Support                       |
  | Bit                     | 32-bit                        |

-------------------------------------------------

### Usage

  This OS running on X86 machine, if you don't have X86 machine, you 
  can use X86 emulator to run OS. This OS have ran on `qemu`, `qemu` is 
  an useful X86 emulator to emulate environment of X86. Please follow 
  specific steps to make it work.

  1. Install essential tools

	  ```
	  sudo apt-get install qemu
	  sudo apt-get install figlet gcc make gdb git
	  sudo apt-get install cflow graphviz gawk
	  ```

	 If you use 64 bit system, please install 32 bit library.
	
	  ```
	  sudo apt-get install lib32z1 lib32z1-dev
	  ```

  2. Download source code

     Download souce code from github, please connect network first.
	
	  ```
	  git clone https://github.com/BuddyZhang1/BiscuitOS.git
	  ```

  3. Compile BiscuitOS
  
	  ```
      make clean  
      make 
      make start
      ```

  4. Debug BiscuitOS

     If you want to debug BiscuitOS, please open debug macro on top Makefile.
	
	  ```
	  vi */BiscuitOS/Makefile
	  --- DEBUG := 
	  +++ DEBUG := 1
	  ```
	
	 Then, recompile source code.
	
	  ```
	  make clean
	  make
	  ```
	
	 Now, we can start to debug OS.
	 On Host:
      
	  ```
      make debug
      ```
     On Target:

	    You can utilize `gdb` to single debug and so on. follow these steps:
	    
		```
	    gdb tools/build/system
	    (gdb)target remote :1234
	    (gdb)b main
	    (gdb)c
	    (gdb)n
	    or 
	    (gdb)s
	    ```
	  
	  You can also utilize `objdump` to get more useful information, such as:
	    
		```
	    objdump -x -s -S -dh tools/build/system
	    ```

	  If you want to debug `bootsect.s` and `setup.s`, you should follow
	  these steps:
        
		```
        gdb tools/build/.debug_bootsect
	    or 
        gdb tools/build/.debug_setup
        (gdb)target remote :1234
        (gdb)b *0x7C00
        (gdb)c
        (gdb)d
	    (gdb)(delete break?)y
	    (gdb)ni
	    or 
	    (gdb)si
        ```
	  
	  You can also uitlize `objdump` to get more useful information,like:
	    
		```
	    objdump -x -s -S -dh tools/build/.debug_bootsect
	    ```

--------------------------------------------------

### Documention

  I have collected more useful documentions, you can access my github to 
  get it. The github link:
  
  ```
    https://github.com/BuddyZhang1/Book-Segment
  ```

---------------------------------------------------

### Contact

  If you have any new source code or problem, you can send email to me.
  This is my frequently used email:
  
  ```
    buddy.zhang@aliyun.com
  ```
----------------------------------------------------

### Other

  > Software is like sex. It's better when it's free
  >
  >                      --- Linus Torvalds

