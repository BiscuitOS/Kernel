inline assembly for X86 in Linux
--------------------------------------------------------

  * GNU assembler syntax in brief

    Let's first look at the basic assembler syntax used in Linux. GCC, the 
    GNU C Compiler for Linux, uses AT&T assembly syntax. Some of the basic
    rules of this syntax are listed below.  

  * Register naming

    Register names are prefixed by %. That is, if eax has to be used, it 
    should be used as %eax

  * Source and destination ordering

    In any instruction, source comes first and destination follows. This 
    differs from intel syntax, where source comes after destination.

    ```
      mov %eax, %ebx;  transfers the contents of eax to ebx
    ```

  * size of operand

    The instruction are suffixed by b, w, or l, depending on whether the 
    operand is a byte, word, or long. This is not mandatory. GCC tries
    provide the approprotate suffix by reading the operands. But specifying
    the suffixes manually improves the code readability and eliminates the
    possibility of the compilers guessing incorrectly.

    ```
      movb %al,   %bl;    --- Byte move
      movw %ax,   %bx;    --- Word move
      movl %eax,  %ebx;   --- Longword move
    ```

  * Immediate operand

    An immediate operand is specified by using $.

    ```
      movl $0xffff, %eax --- will move the value of 0xffff into eax register.
    ```

  * Indirect memory reference

    Any indirect reference to memory are done by using ().

    ```
      movb (%esi), %al -- will transfer the byte in the memory
                          pointed by esi into al register
    ```

  * inline assembly

    GCC provides the special construct `asm` for inline assembly, which has
    the following format:

  * GCC's `asm` construct

    ```
      asm (assembler template
          : output operands              (optional)
          : input operands               (optional)
          : list of clobbered register   (optional)
          );
    ```
    In this example, the assembler template consists of assembly instructions.
    The input operands are the C expressions that serve as input operands to 
    the instructions. The output operands are the C experssion on which the
    output of the assembly instructions will be performed.

    Inline assembly is importand promarily because of its ability to operate 
    and output visible on C variables. Because of this capability, `asm` works
    as an interface between the assembly instructions and the C program that
    contains it.

    One very basic but importand distinction is that simple inline assembly 
    consists only of instructions, whereas extended inline assembly consists
    of operands. To illustrate, consider the follwing example:

    The basics of the inline assembly

    ```
      int a = 10, b;
      asm ("movl %1, %%eax;
            movl %%eax, %0;"
           : "=r" (b)         /* output */
           : "r"  (a)         /* input */
           : "%eax"           /* clobbered register */);
    ```
    In our example we have now made the value of `b` equal to `a` using 
    assembly instructions. Note the following points:

    1) `b` is the output operand, referred to by `%0` and `a` is the input
       operand, referred to by `%1`.

    2) `r` is a constraint on the operands, which specifies that variables 
       `a` and `b` are to be stored in register. Note the output operand 
       constraint should have a constraint modifier `=` along with it to 
       specify that it is an output operand.

    3) To use a register `%eax` inside `asm`, `%eax` should be preceded
       by one more `%`, in other words, `%%eax`, as `asm` uses %0, %1 etc.
       to identify variables. Anything with single `%` will be tranted as 
       input/output operands and not as registers.

    4) The clobbered register `%eax` after the third colon tell GCC that
       the value of `%eax` is to be modified inside `asm`, so GCC won't
       use this register to store any other value.

    5) `mov %1, %%eax` moves the vlaue of `a` into `%eax`, and 
       `mov %%eax, %0` moves the contents of `eax` into `b`.

    6) When the execution of `asm` is complete, `b` will relect the updated
       value, as it is specified as an output operand. In other words,
       the change made to `b` inside `asm` is supposed to be reflected
       outside the `asm`.

  Now let's go into each items in a bit more detail.

  * Assembler template

    The assembler template is a set of assembly instructions (either a
    single instruction or a group of instructions) that gets inserted 
    inside the C program. Either each instruction should be enclosed within
    double quotes, or the entire group of instructions should be within 
    double quotes. Each instruction should also end with a delimiter. The
    valid delimiters are newline(\n) and semicolon(;). `\n` may be followed
    by a tab(\t) as formatter to increase the readability of the instructions
    GCC generates in the assembly file. The instruction refer to the C
    expressions (which are specified as operands) by number `%0`,`%1` etc.

    If you want to make sure that the compiler doesn't not optimize the 
    instructions inside the `asm`, you can use the `volatile` keyword
    after `asm`. If the program has to be ANSI C compiler, `__asm__` and
    `__volatile__` should be used instead of asm and volatile.

  * Operands

    C expressions serve as operands for the assembly instructions inside
    `asm`. Operands are the main feature of inline assembly in cases where
    assembly instructions do a meaningful job by operating on the C 
    expressions of a C program.

    Each operand is specified by a string of operand constraints followed
    by the C expression in brackets, for example: `constraint` (C expression).
    The primary function of the operand constraint is to determine the 
    addressing mode of the operand.

    You may use more than one operand both in input and output sections.
    Each operand is separated by a comma.

    The operands are referenced by numbers inside the assembly template.
    If there are a total of n operands (both input and output inclusive),
    then the first output operand is numbered 0, continuing in increasing 
    order, and the last input operand is number n-1. The total number of 
    operands is limited to ten, or to the maximum number of operands in 
    any instruction pattern in the machine description, whichever is 
    greater.

  * List of clobbered register

    If the instruction in `asm` refer to hardware registers, we can tell
    GCC that we will use and modify them ourselves. GCC will consequently
    not assume that the values it loads into these registers will be valid.
    It isn't generally necessary to list input and output registers as 
    clobbered because GCC knows that `asm` uses them (because they are 
    specified explicitly as constranints). However, if the instructions 
    use any other registers, implicitly or explicitly (and the registers
    are not present either in input or in the output constraint list), then
    the registers have to be specified as a clobbered list. Clobbered 
    registers are listed after the third colon by specifying the register
    names as strings.

    As far as keywords are concerned, if the instructions modify the 
    memory in some unpredictable fashion, and not explicitly, then the 
    `memory` keyword may be added to the list of clobbered registers. This
    tells GCC not to keep the memory values cached in the register across
    the instructions.

  * Operand constraints

    As mentioned before, each operand in `asm` should by describled by a
    string of operand constraints followed by the C expression in brackets.
    Operand constraints essentially determine the addressing mode of the 
    operand in the instruction. Constraints can also specify:

    1) Whether an operand is allowed to be in a register, and which
       kinds of registers it may be include in.

    2) Whether the operand can be a memory reference, and which kinds of
       addresses to use in such a case.

    3) Whether the operand may by an immediate constant

    Constraints may also require two operands to match.

  * Commonly used constraints

    Of the available operand constraints, only a few are used with any 
    frequency. These are listed below along with brief descriptions. For
    a complete list of operand constraints, refer to the GCC and GAS 
    manuals.

  * Register operand constraint(r)

    When operands are specified using this constraint, they get stored in
    General Purpose Registers. Take the following example:

    ```
      asm ("movl %%cr3, %0\n" : "=r" (cr3val));
    ```
    
    Here the variable cr3val is kept in a register,  the value of `%cr3` is
    copied onto that register, and the value of cr3val is updated into the 
    memory fro9m this register. When the `r` constraint is specified, GCC 
    may keep the variable cr3val in any of available GRPs. To specify the 
    register, you must directly specify the register names by using specific
    register constrains.

    ```
      a     %eax
      b     %ebx
      c     %ecx
      d     %edx
      S     %esi
      D     %edi
    ``` 
  
  * Memory operand constraint(m)

    When the operands are in the memory, any operations performed on them
    will occur directly in the memory location, as opposed to register
    constraints, which first store the value in a register to be modified 
    and then write it back to the memory location


































