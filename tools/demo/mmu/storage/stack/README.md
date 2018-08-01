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
