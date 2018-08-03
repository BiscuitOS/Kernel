Stack
---------------------------------------------------

![Stack](https://raw.githubusercontent.com/EmulateSpace/PictureSet/master/BiscuitOS/kernel/MMU000000.png)
  
The stack (see Figure) is a contiguous array of memory locations.
It is contained in a segment and identified by the segment selector
in the `SS` register. When using the flat memory model, the stack
can be located anywhere in the linear address space for the program.
A stack can be up to 4GBytes long, the maximum size of a segment.

Items are placed on the stack using the `PUSH` instruction and removed
from the stack using the `POP` instruction. When an item is pushed
onto the stack, the processor decrements the `ESP` register, then
writen the item at the new top of stack. When an item is poped off
the stack, the processor reads the item from the top of stack, then
increments the `ESP` register. In this manner, the stack grows down
in memory (towards lesser addresses) when items are pushed on the
stack and shrinks up (towards greater address) when the items are
popped from the stack.

A program or operating system/executive can set up many stacks.
For example, in multitasking systems, each task can be given its
own stack. The number of stacks in a system is limited by the
maximum number of segments and the available physical memory.

When a system sets up many stacks, only one stack - the `current stack` 
is available at a time. The current stack is the one contained in
the segment referenced by the `SS` register.

The processor references the `SS` register automatically for all
stack operations. For example, when the `ESP` register is used as
a memory address, it automatically points to an address in the current
stack. Also, the `CALL`, `RET`, `PUSH`, `POP`, `ENTER` and `LEAVE`
instructions all perform operations on the current stack.

## Setting Up a Stack

  To set a stack and establish it as the current stack, the program
  or operating system/executive must do the following:

  1. Establish a stack segment.

  2. Load the segment selector for the stack segment into the `SS`
     register using a `MOV`, `POP`, or `LSS` instruction.

  3. Load the stack pointer for the stack into the `ESP` register
     using a `MOV`, `POP`, or `LSS` instruction. The `LSS` instruction
     can be used to load the `SS` and `ESP` register in one operation. 

## Stack Alignment

  The stack pointer for a stack segment should be aligned on 16-bit(word)
  or 32-bit (double-word) boundaries, depending on the width of the stack
  segment. The `D` flag in the segment descriptor for the current code
  segment sets the stack-segment width. The `PUSH` and `POP` instructions
  use the `D` flag to determine how much to decrement or increment the
  stack pointer on a push or pop operation, respectively. When the stack
  width is 16 bits, the stack pointer is incremented or decrement in 16-bit
  increments. When the width is 32 bits, the stack pointer is incremented
  or decremented in 32-bit increments. Pushing a 16-bit value onto a 32-bit
  wide stack can result in stack misaligned (that is, the stack pointer is
  not aligned on a double-word boundary). One exception to this rule is 
  when the contents of a segment of segment reigster (a 16-bit segment
  selector) are pushed onto a 32-bit wide stack. Here, the processor
  automatically aligns the stack pointer to the next 32-bit boundary.

  ```
     Stack

     +------------------------+
     |                        |
     +------------------------+
     |                        |
     +------------------------+
     |                        |
     +------------------------+
     |                        |
     +------------------------+
     |                        |
     +------------------------+<---- ESP

     Stack 

     +------------------------+
     |                        |
     +------------------------+
     |                        |
     +------------------------+
     |                        |
     +------------------------+
     |                        |
     +------------------------+<---- ESP
     |            |     CS    |
     +------------------------+
  ```

  The processor does no check stack pointer alignment. It is the 
  responsibility of the program, tasks, and system procedure running on
  the processor to maintain proper alignment of stack pointers. Misalignment
  a stack pointer can cause serious performance degradation and in some 
  instances program failures.

## Address-Size Attribute for Stack Access

  Instructions that use the stack implicitly (such as the `PUSH` and `POP`
  instructions) have two address-size attributes each of either 16 or 32
  bits. This is because they always have the implicit address of the top
  of the stack, and they may also have an explicit memory address (for
  example, `PUSH` Array1[EBX]]). The attribute of the explicit address is
  determined by the `D` flag of the current code segment and presence or
  absence of the `67H` addreeess-size prefix.

  The address-size attribute of the top of the stack determines whether
  `SP` or `ESP` is used for the stack access. Stack operations with an
  address-size attribute of 16 use the 16-bit SP stack pointer register
  and can use a maximum stack address of 0xFFFFH. Stack operations with
  an address-size attribute of 32 bit use the 32-bit `ESP` register and
  can use a maximum address of 0xFFFFFFFFH. The default address-size 
  attribute for data segments used as stacks is 16. When the flag is set,
  the address-size attribute is 32.

## Procedure linking information

  The processor provide two pointer for linking of procedures: the stack-
  frame base pointer and the return instruction pointer. When used in 
  conjunction with a standard software procedure-call technique, there
  pointers permit reliable and coherent linking of procedures.

#### Stack-Frame Base Pointer

  The stack is typically divide into frames. Each stack frame can then 
  contain local variables, parameters to be passed to another procedure,
  and procedure linking information. The stack-frame base pointer (
  contained in the `EBP` register) identifies a fixed reference point
  within the stack frame for the called procedure. To use the stack-frame
  base pointer, the called procedure typically copies the contents of
  the `ESP` register into the `EBP` register prior to pushing any local
  variables on the stack. The stack-frame base pointer then permits easy
  access to data structures passed on the stack, to the return instruction
  pointer, and to local variables added to the stack by the called 
  procedure.

  Like the `ESP` register, the `EBP` register automatically points to an
  address in the current stack segment (that is, the segment specified by
  the current contents of the `SS` register).

#### Return Instruction Pointer

  Prior to branching to the first instruction of the called procedure,
  the `CALL` instruction pushes the address in the `EIP` register onto
  the current stack. This address is 























