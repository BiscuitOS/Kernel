Virtual Address (VAS)
---------------------------------------------

In computing, a virtual address space (VAS) or address space is the set of
ranges of virtual adresses that an operating system makes available to a 
process. The range of virtual addresses usually starts at a low address and
can extend to the highest address allowed by the computer's instruction set
architecture and supported by the operating system's pointer size
implementation, which can be 4 bytes for 32-bit or 8 bytes for 64-bit OS
version. This provides several benefits, one of which is security through
process isolation assuming each process is given a separate address space.

# Linux Virtual address

The virtual address space of any Linux process is divided into two subspaces:
kernel space and user space. User space occupies the lower portion of the 
address space, starting from address 0 and extending up to the platform-
specific task size limit **TASK_SIZE**. 

```
0                                               TASK_SIZE            4G
+-----------------------------------------------+---------------------+
|                                               |                     |
|                                               |                     |
|                                               |                     |
+-----------------------------------------------+---------------------+
```

The remainder is occupied by kernel space. Most platforms use a task size limit
that is large enough so that at least half of the available address space is
occupied by the user address space.

User space is private to the process, meaning that it is mapped by the 
process's own page table. In contrast, kernel space is shared across all 
processes. These are two ways to think about kernel space: We can either think
of it as being mapped into the top part of each process, or we can think of it
as a single space that occupies the top part of the CPU's virtual address. 
Interestingly, depending on the specifics of CPU on which Linux is running,
kernel space can be implemented in one or the other way.

During execution at the user level, only user space is accessible. Attempting
to read, write, or execute kernel space would cause a protection violation 
fault. This prevents a faulty or malicious user process from corrupting the
kernel. In contrast, during excution in the kernel, both user and kernel space
are accessible.

# Program Address Space

Without memory for storing data, it's impossible for a program to get any work
done. (Or rather, it's impossible to get any useful work done.) Real-world 
programs can't afford to rely on fixed-size buffers or arrays of data 
structures. They have to be able to handle inputs of varying sizes, from small
to large. This in turn leads to the use of dynamically allocated memory -- 
memory allocated at runtime instead of at compile time. This is how the GNU
"no arbitrary limits" principle is put into action.

Because dynamically allocated memory is such a basic building block for 
real-world programs, we cover it early, before looking at everything else there
is to do. Our discussion focuses exclusively on the user-level view of the
processor and its memory; it has nothing to do with CPU architecture.

For a working definition, we've said that a process is a running program. This
means that the operating system has loaded the executable file for the program
into memory, has arranged for it to have access to its command-line arguments
and evironment variables, and has started it running. A process has five 
conceptually different areas of memory allocated to it:

* Code

  Often referred to as the text segment, this is area in which the executable
  instructions reside. Linux arrange things so that multiple running instances
  of the same program share their code if possible; only one copy of the 
  instructions for the same program resides in memory at any time. (This is
  transparent to the running programs.) The portion of the executable file
  containing the text segment is the text section.

* Initialized data

  Statically allocated and global data that are initialized with nonzero values
  live in the data segment. Each process running the same program has its own
  data segment. The portion of the executable file containing the data segment
  is the data section.

* Zero-initalized data (BSS)

  Global and statically allocated data that are initialized to zero by default
  are kept in what is colloquially called the BSS area of the process. Each
  process running the same program has its own BSS area. When running, the BSS
  data are placed in the data segment. In the executable file, they are stored
  in the BSS section.

  The format of a Linux executable is such that only variables that are 
  initialized to a nonzero value occupy space in the executable's disk file. 
  Thus, a large array declared **static char somebuf[2048];**, which is 
  automatically zero-filled, does not take up 2 KB worth of disk space. (Some
  compilers have options that let you place zero-initialized data into the 
  data segment.)

* Heap

  The heap is where dynamic memory (obtained by malloc() and friends) comes
  from. As memory is allocated on the heap, the process's address space grows,
  as you can see by watching a running program with the **ps** command.

  Although it is possible to give memory back to the system and shrink a
  processor's address space, this is almost never done. (We distinguish between
  releasing nolonger-needed dynamic memory and shrinking the address space;
  this is discussed in more detail later in this chapter.)

  It is typical for the heap to "grow upward". This means that successive items
  that are added to the heap are added at addresses that are numerically 
  greater than previous items. It is also typical for the heap to start
  immediately afer the BSS area of the data segment.

* Stack

  The stack segment is where local variables are allocated. Local variables are
  variables decleared inside the opening left brace of a function  body (or
  other left brace) that aren't defined as **staic**.

  On most architectures, function parameters are also placed on the stack, as
  well as "invisible" bookkeeping information generated by the compiler, such
  as room for a function return value and storage for the return address 
  representing the return from a function to its caller. (Some architectures
  do all this with registers.)

  It is the use of a stack for function parameters and return values that makes
  it convenient to write recursive function (function that call themselves).

  Variables stored on the stack "disappear" when the function containing them
  returns; the space on the stack is reused for subsequent funciton calls. On
  most modem architectures, the stack "grows downward", meaning that item 
  deeper in the call chain are at numerically lower address.

When a program is running, the initialized data, BSS, and heap areas are 
usually placed into a single contiguous area: the data segment. The stack 
segment and code segment are separate from the data segment and from each 
other. 

```
4G +----------------+
   |                |
   |                |
   |  Kernel Space  |
   |                |
   |                |
3G +----------------+-- 0xC0000000
   |                | A
   |                | | Random stack offset
   |                | V
   +----------------+--
   |     Stack      | A
   |                | | 
   +----------------+ | RLIMIT_STACK
   |                | |
   |                | V
   |                |--
   |                | A
   |                | | Random mmap offset
   |                | V
   +----------------+--
   | Memory Mapping |
   |    Segment     |
   +----------------+
   |                |
   +----------------+ Program break
   |                | brk
   |                |
   |      Heap      |
   |                |
   |                |
   +----------------+ start_brk
   |                | Random brk offset
   +----------------+
   |  BSS Segment   |
   +----------------+ end_data
   |  Data Segment  |
   +----------------+ start_data/end_code
   |  Text Segment  |
   +----------------+ 0x08048000    
   |                |
0G +----------------+
```

Although it's theoretically possible for the stack and heap to grow into
each other, the operating system prevents that event, and any program that
tries to make it happen is asking for trouble. This is particularly true on 
modem systems, on which process address spaces are large and the gap between
the top of the stack and the end of the heap is a big one. The different 
memory area can have different hardware memory protection assigned to them. For
example, the text segment might be marked "execute only", whereas the data and
stack segment would have execute permission disabled. This partice can prevent
certain kinds of security attacks. The details, of cause, are hardware and 
operating-system specific and likely to change over time. Of note is that both
Standard C and C++ allow const items to be placed in the read-only memory.

# Reference

[Wikipedia: Virtual address space](https://en.wikipedia.org/wiki/Virtual_address_space)

[Virtual Memory in the IA-64 Linux Kernel: 4.2](http://www.informit.com/articles/article.aspx?p=29961&seqNum=2)

[Linux Programming by Example: The Fundamentals: 3.1](http://www.informit.com/articles/article.aspx?p=173438)

[Computer Architecture English note](https://biscuitos.github.io/blog/Architecture_English_note/)
