Memory Organization
------------------------------------------------------------

The memory that the processor address on its bus is called physical memory.
Physical memory is organized as a sequence of 8-bit bytes. Each byte is 
assigned a unique address, called a physical address. The physical address
space ranges from zero to a maximum of 2^36 - 1 (64GBytes) if the processor
does not support Intel 64 architecture.

## Memory models

When employing the processor's memory management facilities, programs do not
directly address physical memory. Instead, they access memory using one of
three memory models: **flat**, **segmented**, or **read address** mode.

![Three Memory Management Models](https://github.com/EmulateSpace/PictureSet/blob/master/BiscuitOS/kernel/MMU000391.png)

* **Flat memory model** -- Memory appears to a program as a single, continuous
  address space(as Figure). This space is called a **linear address space**.
  Code, data, and stacks are all contained in this address space. Linear 
  address space is byte addressable, with address running contiguously from
  0 to 2^32 - 1. An address for any byte in linear address space is called
  a linear address.

* **Segmented memory model** -- Memory appears to a program as a group of 
  independent address spaces called segment. Code, data, and stacks are
  typically contained in separate segments. To address a byte in a segment,
  a program issues a logical address. This consists of a segment selector and
  an offset (logical addresses are often referred to as far pointers). The
  segment selector identifies the segment to be accessed and the offset 
  identifies a byte in the address space of the segment. Programs running on 
  an IA-32 processor can address up to 16,383 segments of different sizes and
  types, and each segment can be as large as 2^32 bytes.

  Internally, all the segments that are defined for a system are mapped into
  the processor's linaer address space. To access a memory location, the 
  processor thus translates each logical address into a linear address. This
  translation is transparent to the application program.

  The primary reason for using segmented memory is to increase the reliability
  of programs and systems. For example, placing a program's stack in a separate
  segment prevents the stack from growing into the code or data space and
  overwriting instructions or data, respectively.

* **Real-address mode memory model** -- This is the memory model for the Intel
  8086 processor. It is supported to provide compatibility with existing 
  programs written to run on the Intel 8086 processor. The real-address mode
  uses a specific implementation of segmented memory in which the linear
  address space for the program and the operating system/executive consists of
  an array of segments of up to 64 KBytes in size each. The maximum size of 
  the linear address space in real-address mode is 2^20 bytes.

## Logical Address

Even with the minimum use of segments, every byte in the processor's address
space is accessed with a logical address. A **logical address** consists of a
16-bit segment selector and a 32-bit offset. The segment selector
identifies the segment the byte is located in and the offset specifies the 
location of the byte in the segment relative to the base address of the 
segment.

```
Logical Address:

 15              0   31                           0
 +---------------+   +----------------------------+
 | Seg. Selector |   | Offset (Effective Address) |
 +---------------+   +----------------------------+
```

More detail **logical address** see: logical_address/README.md

## Linear Address



