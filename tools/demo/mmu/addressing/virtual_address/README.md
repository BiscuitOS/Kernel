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

# Reference

[Wikipedia: Virtual address space](https://en.wikipedia.org/wiki/Virtual_address_space)

[Virtual Memory in the IA-64 Linux Kernel: 4.2](http://www.informit.com/articles/article.aspx?p=29961&seqNum=2)

[Computer Architecture English note](https://biscuitos.github.io/blog/Architecture_English_note/)
