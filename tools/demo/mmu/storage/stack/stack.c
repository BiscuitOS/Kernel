/*
 * Stack
 *
 * (C) 2018.08.05 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>

#include <demo/debug.h>

/*
 * Calling procedures using CALL and RET
 *   The CALL instruction allows control transfers to procedures within 
 *   the current code segment (near call) and in a different code segment (
 *   far call). Near calls usually provide access to local procedures 
 *   within the currently running program or task. Far calls are usually
 *   used to access operating system procedure or procedures in a different
 *   task. 
 *
 *   The RET instruction also allows near and far returns to match the near
 *   and far versions of the CALL instruction. In addition, the RET
 *   instruction allows a program to increment the stack pointer on a 
 *   return to release parameters from the stack. The number of bytes
 *   release from the stack is determined by an optional argument (n) to
 *   the RET instruction.
 *  
 * Near CALL and RET operation
 *   When executing a near call, the processor does the following
 *   1. Pushes the current value of the EIP register on the Stack.
 *   2. Loads the offset of the called procedure in the EIP register.
 *   3. Begins execution of the called procedure.
 *
 *
 *   Stack during Near Call
 *
 *                   |                            |
 *                   +----------------------------+
 *   Stack Frame     |           Param1           |
 *   Before Call     +----------------------------+
 *        A          |           Param2           |
 *        |          +----------------------------+
 *        o----------|-          Param3           |<-- ESP Before Call
 *                   +----------------------------+
 *        o----------|-        Calling EIP        |<-- ESP After Call
 *        |          +----------------------------+
 *        V          |                            |
 *   Stack Frame     +----------------------------+
 *   After Call      |                            |
 *
 *
 *   When executing a near return, the processor performs these actions:
 *   1. Pops the top-of-stack value (the return instruction pointer) 
 *      into the EIP register.
 *   2. If the RET instruction has an optional n argument, increments 
 *      the stack pointer by the pointer by the number of bytes specified
 *      with the n operand to release parameters from the stack.
 *   3. Resumes execution of the calling procedure.
 *
 *
 *   Stack during Near Return
 *
 *                   |                            |
 *                   +----------------------------+
 *                   |                            |<-- ESP After Return
 *                   +----------------------------+
 *                   |           Param1           |
 *                   +----------------------------+
 *                   |           Param2           |
 *                   +----------------------------+
 *                   |           Param3           |
 *                   +----------------------------+
 *                   |        Calling EIP         |<-- ESP Before Return
 *                   +----------------------------+
 *                   |                            |
 *                   +----------------------------+
 *                   |                            |
 *
 *
 *  
 *   When executing a far call, the processor does the following
 *   1. Pushes the current value of the CS register on the stack
 *   2. Pushes the current value of the EIP register on the stack.
 *   3. Loads the segment selector of the segment that contains
 *      the called procedure in the CS register.
 *   4. Loads the offset of the called procedure in the EIP reigster.
 *   5. Begins execution of the called procedure.
 *
 *
 *   Stack during Near Call
 *
 *                   |                            |
 *                   +----------------------------+
 *   Stack Frame     |           Param1           |
 *   Before Call     +----------------------------+
 *        A          |           Param2           |
 *        |          +----------------------------+
 *        o----------|-          Param3           |<-- ESP Before Call
 *                   +----------------------------+
 *        o----------|-        Calling CS         |
 *        |          +----------------------------+
 *        V          |         Calling EIP        |<-- ESP After Call
 *   Stack Frame     +----------------------------+
 *   After Call      |                            |
 *   
 *
 *   When executing a far return, the processor performs these actions:
 *   1. Pops the top-of-stack value (the return instruction pointer)
 *      into the EIP register.
 *   2. Pops the top-of-stack value (the segment selector for the code
 *      segment being returned to) into the CS register.
 *   3. If the RET instruction has an optional n argument, increments
 *      the stack pointer by the number of bytes specified with the 
 *      n operand to release parameters from the stack.
 *   4. Resumes execution of the calling procedure.
 *
 *
 *   Stack during far Return
 *
 *                   |                            |
 *                   +----------------------------+
 *                   |                            |<-- ESP After Return
 *                   +----------------------------+
 *                   |           Param1           |
 *                   +----------------------------+
 *                   |           Param2           |
 *                   +----------------------------+
 *                   |           Param3           |
 *                   +----------------------------+
 *                   |        Calling CS          |
 *                   +----------------------------+
 *                   |        Calling EIP         |<-- ESP Before return
 *                   +----------------------------+
 *                   |                            |
 *
 *
 */


/*
 * CALL -- Call Procedure
 *   
 *   Saves procedure linking information on the stack and branches to the
 *   called procedure specified using the target operand. The target
 *   operand specifies the address of the first instruction in the called
 *   procedure. The operand can be an immediate value, a general-purpose
 *   register, or a memory location.
 * 
 *   This instruction can be used to execute four type of calls:
 *  
 *   * Near Call - A call to a procedure in the current code segment (the
 *     segment currently pointed to by the CS register), sometime referred
 *     to as an intra-segment call.
 *
 *   * Far Call - A call to procedure located in a different segment than
 *     the current code segment, sometimes referred to as an inter-segment
 *     call.
 *
 *   * Inter-privilege-level far call - A far call to a procedure in a 
 *     segment at a different prvilege level than that of the currently
 *     executing program or procedure.
 *
 *   * Task Switch - A call to a procedure located in a different task.
 *
 *   The latter two call types (inter-privilege-level call and task switch)
 *   can only be executed in protected mode.
 */

/*
 * Near Call
 *
 *   When executing a near call, the processor pushes the value of the EIP
 *   register (which contains the offset of the instruction following the
 *   the CALL instruction) on stack (for use later as a return-instruction
 *   pointer). The processor then branches to the address in the current
 *   code segment specified by the target operand. The target relative
 *   offset (a signed displacement relative to the current value of the
 *   instruction pointer in the EIP register, this value points to the 
 *   instruction following the CALL instruction). The CS register is not
 *   changed on near calls.
 *
 *   For a near call absolute, an absolute offset is specified indirectly
 *   in a general-purpose register or a memory location (r/m16, r/m32). 
 *   The operand-size attribute determines the size of the target operand (
 *   16, 32 or 64 bits). Absolute offsets are loaded directly into the 
 *   EIP(RIP) register. If the operand size attribute is 16, the upper
 *   two bytes of the EIP register are cleared, resulting in a maximum
 *   instruction pointer size of 16 bits. When accessing an absolute offset
 *   indirectly using the stack pointer [ESP] as the base register, the
 *   base value used is the value of the ESP before the instruction executes.
 *   
 *   A example, the calling procedure is 'debug_near_call()' and called
 *   procedure is "near_call_procedure()".
 */

/*
 * Invoke called procedure on near call
 *
 * Invoke GDB to dump all register information. As follow:
 * breakpointer on 'near_call_procedure'
 * *******************************************************
 * (gdb) b near_call_procedure
 * (gdb) c
 * (gdb) info reg
 * eax            0x41              65
 * ecx            0x16              22
 * edx            0x21ed0           138960
 * ebx            0x10              16
 * esp            0x21eb4           0x21eb4 <user_stack+3988>
 * ebp            0x21f0c           0x21f0c <user_stack+4076>
 * esi            0x0               0
 * edi            0xffc             4092
 * eip            0x16930           0x16930 <near_call_procedure>
 * eflags         0x2               [ ]
 * cs             0x8               8
 * ss             0x10              16
 * ds             0x10              16
 * es             0x10              16
 * fs             0x10              16
 * gs             0x10              16
 *
 * (gdb) x/32x 0x21ea0
 * 0x21ea0 <user_stack+3968>: 0x000b849c 0x00000018 0x0000001c 0x00000006
 * 0x21eb0 <user_stack+3984>: 0x00000005 0x0001697f 0x00000041 0x00000010
 * 0x21ec0 <user_stack+4000>: 0x00000016 0x00021ed0 0x08182403 0x24280605
 * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
 * 0x21ee0 <user_stack+4032>: 0x00400000 0xffffffff 0x00000000 0x00000000
 * 0x21ef0 <user_stack+4048>: 0x00000000 0x00000000 0x00000000 0x00000000
 * 0x21f00 <user_stack+4064>: 0x00000000 0x00000000 0x00000000 0x00000152
 * 0x21f10 <user_stack+4080>: 0x00005412 0x00000000 0x00000000 0x00000000
 * *******************************************************
 *
 * Then invoke 'OBJDUMP' to dump code information. As Follow:
 * *******************************************************
 * $root> objdump -xsSdh > objump.info
 *
 * 00016930 <near_call_procedure>:
 * 
 * static int near_call_procedure(char a, short b, int c, int *p)
 * {
 *     16930:    83 ec 08           sub    $0x8,%esp
 *     16933:    8b 54 24 0c        mov    0xc(%esp),%edx
 *     16937:    8b 44 24 10        mov    0x10(%esp),%eax
 *     1693b:    88 54 24 04        mov    %dl,0x4(%esp)
 *     1693f:    66 89 04 24        mov    %ax,(%esp)
 *     return 0;
 *     16943:    b8 00 00 00 00     mov    $0x0,%eax
 * }
 *     16948:    83 c4 08           add    $0x8,%esp
 *     1694b:    c3                 ret
 *
 */
static __unused int near_call_procedure(char a, short b, int c, int *p)
{
    /*
     * The called procedure contains 4 paraments, befer the called
     * procedure establish local stack, the ESP stack point to the
     * next instrcution address of "CALL" instruction on calling 
     * procedure.
     *
     * Before called procedure establish local stack, and set up a
     * breakpointer on 'near_call_procedure', as follow:
     *
     * (gdb) b near_call_procedure
     * (gdb) c
     * (gdb) info reg
     * esp            0x21eb4           0x21eb4 <user_stack+3992>
     * ebp            0x21f0c           0x21f0c <user_stack+4076>
     * eip            0x1697a           0x1697a <near_call_procedure>
     * (gdb) x/32x 0x21eb0
     * 0x21eb0 <user_stack+3984>: 0x00000005 0x0001697f 0x00000041 0x00000010
     * 0x21ec0 <user_stack+4000>: 0x00000016 0x00021ed0 0x08182403 0x24280605
     * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
     * 0x21ee0 <user_stack+4032>: 0x00400000 0xffffffff 0x00000000 0x00000000
     * 0x21ef0 <user_stack+4048>: 0x00000000 0x00000000 0x00000000 0x00000000
     * 0x21f00 <user_stack+4064>: 0x00000000 0x00000000 0x00000000 0x00000152
     * 0x21f10 <user_stack+4080>: 0x00005412 0x00000000 0x00000000 0x00000000
     *
     *     16979:    50                 push   %eax
     *     1697a:    e8 b1 ff ff ff     call   16930 <near_call_procedure>
     *     1697f:    83 c4 10           add    $0x10,%esp
     * 
     * On above information, the address of 'CALL' instruction is "0x1697a",
     * and the next instruction address of 'CALL' instruction is "0x1697f",
     * and the content of ESP register is '0x21eb4' which contents is:
     *
     * 0x21eb0 <user_stack+3984>: 0x00000005 0x0001697f 0x00000041 0x00000010
     *
     * Then Establish local stack, the called procedure decrements special
     * value for ESP register, for example:
     *
     * static int near_call_procedure(char a, short b, int c, int *p)
     * {
     *     16930:    83 ec 08           sub    $0x8,%esp
     *     16933:    8b 54 24 0c        mov    0xc(%esp),%edx
     *     16937:    8b 44 24 10        mov    0x10(%esp),%eax
     *     1693b:    88 54 24 04        mov    %dl,0x4(%esp)
     *     1693f:    66 89 04 24        mov    %ax,(%esp)     
     *
     * Now setup a break pointer on "0x1693f", as follow:
     *
     * (gdb) b *0x1693f
     * (gdb) c
     * (gdb) info reg
     * esp            0x21eac           0x21eac <user_stack+3980>
     * ebp            0x21f0c           0x21f0c <user_stack+4076>
     * eip            0x1693f           0x1693f <near_call_procedure+15>
     * (gdb) x/32x 0x21ea0
     * 0x21ea0 <user_stack+3968>: 0x000b849c 0x00000018 0x0000001c 0x00000006
     * 0x21eb0 <user_stack+3984>: 0x00000041 0x0001697f 0x00000041 0x00000010
     * 0x21ec0 <user_stack+4000>: 0x00000016 0x00021ed0 0x08182403 0x24280605
     * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
     * 0x21ee0 <user_stack+4032>: 0x00400000 0xffffffff 0x00000000 0x00000000
     * 0x21ef0 <user_stack+4048>: 0x00000000 0x00000000 0x00000000 0x00000000
     * 0x21f00 <user_stack+4064>: 0x00000000 0x00000000 0x00000000 0x00000152
     * 0x21f10 <user_stack+4080>: 0x00005412 0x00000000 0x00000000 0x00000000
     *
     * The local stack has been established which conjunction with calling
     * procedure stack. And the size of local stack is 8 Byte.
     * The local stack of called procedure as figure:
     *
     *
     * +------------------------+ (high address)
     * |                        |  A
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Procedure Stack
     * |                        |  |
     * +------------------------+  V
     * |                        |<---- Before establish local stack
     * +------------------------+  A   old ESP register.
     * |                        |  |
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Local Stack (calling procedure)
     * |                        |  |
     * +------------------------+  |    
     * |                        |  |
     * +------------------------+  V
     * |    last/4th parament   |<---- After establish local stack
     * +------------------------+      new ESP register.
     * |      3rd parament      |
     * +------------------------+
     * |      2nd parament      |
     * +------------------------+
     * |      1st parament      |<---- Before execute CALL instruction
     * +------------------------+      ESP register.
     * |    Next instruction    |<---- After execute CALL instruction
     * +------------------------+  A   ESP register/ Before called 
     * |                        |  |   procedure establish local stack.
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Local stack (called procedure)
     * |                        |  |
     * +------------------------+  |
     * |                        |  |  
     * +------------------------+  V
     * |                        |<---- After called procedure establish
     * +------------------------+      local stack ESP register.
     *
     * The called procedure indicate the return-instruction address and
     * parament after calling procedure execute "CALL" instruction, so
     * it will access parament correctly.
     */

    return 0;
}

/*
 * Before near call
 *
 * Invoke GDB to dump all register information. As follow:
 * breakpointer on 'debug_near_call'
 * *******************************************************
 * (gdb) b debug_near_call
 * (gdb) c
 * (gdb) info reg
 * eax            0x5b6677fe        1533442046
 * ecx            0x34              52
 * edx            0x168             360
 * ebx            0x3               3
 * esp            0x21edc           0x21edc <user_stack+4028>
 * ebp            0x21f0c           0x21f0c <user_stack+4076>
 * esi            0x0               0
 * edi            0xffc             4092
 * eip            0x1694c           0x1694c <debug_near_call>
 * eflags         0x2               [ ]
 * cs             0x8               8
 * ss             0x10              16
 * ds             0x10              16
 * es             0x10              16
 * fs             0x10              16
 * gs             0x10              16
 *
 * (gdb) x/32x 0x21eb0
 * 0x21eb0 <user_stack+3984>: 0x00000005 0x00000007 0x00000012 0xe3100000
 * 0x21ec0 <user_stack+4000>: 0x000b80a0 0x00018286 0x08180803 0x08060605
 * 0x21ed0 <user_stack+4016>: 0xffff0000 0x00000030 0x00000020 0x00016a2a
 * 0x21ee0 <user_stack+4032>: 0x00400000 0xffffffff 0x00000000 0x00000000
 * 0x21ef0 <user_stack+4048>: 0x00000000 0x00000000 0x00000000 0x00000000
 * 0x21f00 <user_stack+4064>: 0x00000000 0x00000000 0x00000000 0x00000152
 * 0x21f10 <user_stack+4080>: 0x00005412 0x00000000 0x00000000 0x00000000
 * *******************************************************
 *
 * Then invoke 'OBJDUMP' to dump code information. As Follow:
 * *******************************************************
 * $root> objdump -xsSdh > objump.info
 *
 * 0001694c <debug_near_call>:
 *
 * static int debug_near_call(void)
 * {
 *     1694c:    53                 push   %ebx
 *     1694d:    83 ec 10           sub    $0x10,%esp
 *     char a = 'A';
 *     16950:    c6 44 24 0f 41     movb   $0x41,0xf(%esp)
 *     short b = 0x10;
 *     16955:    66 c7 44 24 0c 10 00      movw   $0x10,0xc(%esp)
 *     int c = 0x16;
 *     1695c:    c7 44 24 08 16 00 00      movl   $0x16,0x8(%esp)
 *     16963:    00
 *
 *     __asm__ ("pushl %%edx\n\t"
 *     16964:    8b 4c 24 08        mov    0x8(%esp),%ecx
 *     16968:    0f b6 44 24 0f     movzbl 0xf(%esp),%eax
 *     1696d:    0f b7 5c 24 0c     movzwl 0xc(%esp),%ebx
 *     16972:    8d 54 24 08        lea    0x8(%esp),%edx
 *     16976:    52                 push   %edx
 *     16977:    51                 push   %ecx
 *     16978:    53                 push   %ebx
 *     16979:    50                 push   %eax
 *     1697a:    e8 b1 ff ff ff     call   16930 <near_call_procedure>
 *     1697f:    83 c4 10           add    $0x10,%esp
 *              "pushl %%ebx\n\t"
 *              "pushl %%eax\n\t"
 *              "call near_call_procedure\n\r"
 *              "addl $16, %%esp" :: "a" (a), "b" (b),
 *              "c" (c), "d" (&c) );
 *     return 0;
 *     16982:    b8 00 00 00 00     mov    $0x0,%eax
 * }
 *     16987:    83 c4 10           add    $0x10,%esp
 *     1698a:    5b                 pop    %ebx
 *     1698b:    c3                 ret
 *
 */
static __unused int debug_near_call(void)
{
    /*
     * Before near call
     *
     * On breakpoint 'debug_near_call', the 'debug_near_call' contain 3 
     * local value, and 'debug_near_call' will establish a local stack
     * to store these local value, assembly as follow:
     *
     *     1694c:    53                 push   %ebx
     *     1694d:    83 ec 10           sub    $0x10,%esp
     *     char a = 'A';
     *     16950:    c6 44 24 0f 41     movb   $0x41,0xf(%esp)
     *     short b = 0x10;
     *     16955:    66 c7 44 24 0c 10 00      movw   $0x10,0xc(%esp)
     *     int c = 0x16;
     *     1695c:    c7 44 24 08 16 00 00      movl   $0x16,0x8(%esp)
     *     16963:    00
     * 
     * The common rule to establish a local stack is conjunction with
     * procedure stack, and decrement the ESP register to a suit location.
     * And the range of local stack from new ESP value to old ESP value.
     *
     * Local Stack
     *
     * +------------------------+ (high address)
     * |                        |  A
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Procedure Stack
     * |                        |  |
     * +------------------------+  V
     * |                        |<---- Before establish local stack
     * +------------------------+  A   old ESP register.
     * |                        |  |
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Local Stack
     * |                        |  |
     * +------------------------+  |    
     * |                        |  |
     * +------------------------+  V
     * |                        |<---- After establish local stack
     * +------------------------+      new ESP register.
     * |                        |
     * +------------------------+ (low address)
     *
     * Before establishing Local stack, the ESP register points to stack 
     * which contents as follow:
     *
     * (gdb) info reg
     * esp            0x21edc           0x21edc <user_stack+4028>
     * ebp            0x21f0c           0x21f0c <user_stack+4076>
     * eip            0x1694c           0x1694c <debug_near_call>
     *
     * (gdb) x/32x 0x21eb0
     * 0x21eb0 <user_stack+3984>: 0x00000005 0x00000007 0x00000012 0xe3100000
     * 0x21ec0 <user_stack+4000>: 0x000b80a0 0x00018286 0x08180803 0x08060605
     * 0x21ed0 <user_stack+4016>: 0xffff0000 0x00000030 0x00000020 0x00016a2a
     * 0x21ee0 <user_stack+4032>: 0x00400000 0xffffffff 0x00000000 0x00000000
     * 0x21ef0 <user_stack+4048>: 0x00000000 0x00000000 0x00000000 0x00000000
     *
     * According to above information, the ESP register is '0x21edc' which 
     * contents is '0x00016a2a'.
     *
     * On near call, the local stack is conjuection with procedure stack.
     * And then, add a breakpoint on '0x16963', and local stack has 
     * established. Dump stack and register as follow:
     *
     * (gdb) b *0x1695c
     * (gdb) c
     * (gdb) n
     * (gdb) info reg
     * esp            0x21ec8           0x21ec8 <user_stack+4008>
     * ebp            0x21f0c           0x21f0c <user_stack+4076>
     * eip            0x16964           0x16964 <debug_near_call+24>
     * 
     * (gdb) x/32x 0x21eb0
     * 0x21eb0 <user_stack+3984>: 0x00000005 0x00000007 0x00000012 0xe3100000
     * 0x21ec0 <user_stack+4000>: 0x000b80a0 0x00018286 0x08180803 0x08060605
     * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
     * 0x21ee0 <user_stack+4032>: 0x00400000 0xffffffff 0x00000000 0x00000000
     * 0x21ef0 <user_stack+4048>: 0x00000000 0x00000000 0x00000000 0x00000000
     * 0x21f00 <user_stack+4064>: 0x00000000 0x00000000 0x00000000 0x00000152
     * 0x21f10 <user_stack+4080>: 0x00005412 0x00000000 0x00000000 0x00000000
     *
     * On establishing local stack, the EIP register is "old ESP - $10". 
     * The value of old ESP register is "0x21edc", because "push %ebx" 
     * decrement 4 for old ESP register which is "0x21edc8", and the value 
     * of local stack is '0x21ec8'. According assembly:
     *
     *     1694c:    53                 push   %ebx
     *     1694d:    83 ec 10           sub    $0x10,%esp
     *     char a = 'A';
     *     16950:    c6 44 24 0f 41     movb   $0x41,0xf(%esp)
     *     short b = 0x10;
     *     16955:    66 c7 44 24 0c 10 00      movw   $0x10,0xc(%esp)
     *     int c = 0x16;
     *     1695c:    c7 44 24 08 16 00 00      movl   $0x16,0x8(%esp)
     *     16963:    00
     *
     * The address of first local address is '0x21ed7'(0xf + 0x21ec8)
     * which contents is 0x41, as follow:
     *
     * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
     *
     * The address of 2nd local address is '0x21ed4'(0xc + 0x21ec8) which
     * contents is 0x10, as follow:
     *
     * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
     *
     * The address of 3rd local address is '0x21ed0'(0x8 + 0x21ec8) which
     * contents is 0x16, as follow: 
     *
     * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
     *
     * After establishing local stack, the value of ESP is '0x21ec8'. In
     * order to invoke a function, the calling procedure need push the 
     * parament/parament of called procedure into stack backwards, and
     * the last argument of called procedure first into stack. for example,
     * the calling procedure invoke 'near_call_procedure()' which need 4
     * paraments, so the calling procedure need push 4 argument into stack
     * backwards, as figure:
     *
     * +------------------------+ (high address)
     * |                        |  A
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Procedure Stack
     * |                        |  |
     * +------------------------+  V
     * |                        |<---- Before establish local stack
     * +------------------------+  A   old ESP register.
     * |                        |  |
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Local Stack
     * |                        |  |
     * +------------------------+  |    
     * |                        |  |
     * +------------------------+  V
     * |    last/4th parament   |<---- After establish local stack
     * +------------------------+      new ESP register.
     * |      3rd parament      |
     * +------------------------+
     * |      2nd parament      |
     * +------------------------+
     * |      1st parament      |<---- Before execute CALL instruction
     * +------------------------+      ESP register.
     * |                        |
     * +------------------------+
     * |                        |
     * +------------------------+
     * |                        |
     * +------------------------+
     * |                        |
     * +------------------------+
     *
     * The assembly code for push argument of called procedure as follow:
     *
     *     16964:    8b 4c 24 08        mov    0x8(%esp),%ecx
     *     16968:    0f b6 44 24 0f     movzbl 0xf(%esp),%eax
     *     1696d:    0f b7 5c 24 0c     movzwl 0xc(%esp),%ebx
     *     16972:    8d 54 24 08        lea    0x8(%esp),%edx
     *     16976:    52                 push   %edx
     *     16977:    51                 push   %ecx
     *     16978:    53                 push   %ebx
     *     16979:    50                 push   %eax
     *     1697a:    e8 b1 ff ff ff     call   16930 <near_call_procedure>
     *     1697f:    83 c4 10           add    $0x10,%esp
     *
     * Now, setup a breakpoint on '0x1697a' where before invoke called 
     * procedure. follow step:
     *
     * (gdb) b *0x1697a
     * (gdb) c
     * (gdb) info reg
     * esp            0x21eb8           0x21eb8 <user_stack+3992>
     * ebp            0x21f0c           0x21f0c <user_stack+4076>
     * eip            0x1697a           0x1697a <debug_near_call+46>
     * (gdb) x/32x 0x21eb0
     * 0x21eb0 <user_stack+3984>: 0x00000005 0x00000007 0x00000041 0x00000010
     * 0x21ec0 <user_stack+4000>: 0x00000016 0x00021ed0 0x08182403 0x24280605
     * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
     * 0x21ee0 <user_stack+4032>: 0x00400000 0xffffffff 0x00000000 0x00000000
     * 0x21ef0 <user_stack+4048>: 0x00000000 0x00000000 0x00000000 0x00000000
     * 0x21f00 <user_stack+4064>: 0x00000000 0x00000000 0x00000000 0x00000152
     * 0x21f10 <user_stack+4080>: 0x00005412 0x00000000 0x00000000 0x00000000
     *
     * The address of ESP register for local stack is '0x21ec8', and the last
     * parament of called procedure is a pointer which points to a local 
     * value. The address of local value is '0x21ed0', and then push it into
     * stack. So the address on stack is '0x21ec4' (ESP - 4), and the contents 
     * of stack is:
     *
     * 0x21ec0 <user_stack+4000>: 0x00000016 0x00021ed0 0x08182403 0x24280605
     *
     * And then, the calling procedure push penult into stack, the penult is
     * a int value which contents is 0x16. And the address of
     * the penult is '0x21ec0 (ESP - 4)', and the contents of stack is:
     *
     * 0x21ec0 <user_stack+4000>: 0x00000016 0x00021ed0 0x08182403 0x24280605
     *
     * Last, the calling procedure push 2rd and 1st argument into stack,
     * the address of 2rd and 1st is '0x21ebf' and '0x21ebc'. The contents
     * of stack is:
     *
     * 0x21eb0 <user_stack+3984>: 0x00000005 0x00000007 0x00000041 0x00000010
     *
     * After calling procedure push argument of called procedure into stack,
     * the ESP register is '0x21eb8'. Now, the calling procedure prepares
     * to invoke called procedure. The 'CALL' instruction will invoke called
     * procedure with a address on near call, for example:
     *
     *    call near_call_procedure
     *
     * Now execute 'CALL' instruction and the system will push the next 
     * instruction address of 'CALL' instruction into stack automatically,
     * and then jmups to 'near_call_procedure()'.
     * We setup a breakpoint after "CALL" instruction. as follow:
     *
     * (gdb) b near_call_procedure
     * (gdb) c
     * (gdb) info reg
     * esp            0x21eb4           0x21eb4 <user_stack+3992>
     * ebp            0x21f0c           0x21f0c <user_stack+4076>
     * eip            0x1697a           0x1697a <debug_near_call+46>
     * (gdb) x/32x 0x21eb0
     * 0x21eb0 <user_stack+3984>: 0x00000005 0x0001697f 0x00000041 0x00000010
     * 0x21ec0 <user_stack+4000>: 0x00000016 0x00021ed0 0x08182403 0x24280605
     * 0x21ed0 <user_stack+4016>: 0x00000016 0x41000010 0x00000003 0x00016a2a
     * 0x21ee0 <user_stack+4032>: 0x00400000 0xffffffff 0x00000000 0x00000000
     * 0x21ef0 <user_stack+4048>: 0x00000000 0x00000000 0x00000000 0x00000000
     * 0x21f00 <user_stack+4064>: 0x00000000 0x00000000 0x00000000 0x00000152
     * 0x21f10 <user_stack+4080>: 0x00005412 0x00000000 0x00000000 0x00000000
     * 
     * On above information, the address of 'CALL' instruction is "0x1697a",
     * and the next instruction address of 'CALL' instruction is "0x1697f",
     * and the content of ESP register is '0x21eb4' which contents is:
     *
     * 0x21eb0 <user_stack+3984>: 0x00000005 0x0001697f 0x00000041 0x00000010
     *
     * The next instruction address has been written into stack, the calling
     * procedure will jump into called procedure. For example, the called
     * procedure is "near_call_procedure()", more information see 
     * "near_call_procedure()".
     *
     * +------------------------+ (high address)
     * |                        |  A
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Procedure Stack
     * |                        |  |
     * +------------------------+  V
     * |                        |<---- Before establish local stack
     * +------------------------+  A   old ESP register.
     * |                        |  |
     * +------------------------+  |
     * |                        |  |
     * +------------------------+  |   Local Stack
     * |                        |  |
     * +------------------------+  |    
     * |                        |  |
     * +------------------------+  V
     * |    last/4th parament   |<---- After establish local stack
     * +------------------------+      new ESP register.
     * |      3rd parament      |
     * +------------------------+
     * |      2nd parament      |
     * +------------------------+
     * |      1st parament      |<---- Before execute CALL instruction
     * +------------------------+      ESP register.
     * |    Next instruction    |<---- After execute CALL instruction
     * +------------------------+      ESP register.
     * |                        |
     * +------------------------+
     * |                        |
     * +------------------------+
     * |                        |
     * +------------------------+
     */
    char a = 'A';
    short b = 0x10;
    int c = 0x16;

    __asm__ ("pushl %%edx\n\t"
             "pushl %%ecx\n\t"
             "pushl %%ebx\n\t"
             "pushl %%eax\n\t"
             "call near_call_procedure\n\r"
             "addl $16, %%esp" :: "a" (a), "b" (b),
             "c" (c), "d" (&c) );
    return 0;
}

/*
 * Far Calls in Protected Mode
 *
 *   When the processor is operating in protected mode, the CALL instruction
 *   can be used to perform the following types of the far calls:
 *
 *   * Far call to the same privilege level
 *   * Far call to a different privilege level (inter-privilege level call)
 *   * Task switch (far call to another task)
 *
 *   In protected mode, the processor always uses the segment selector part
 *   of the far address to access the corresponding descriptor in the GDT
 *   or LDT. The descriptor type (code segment, call gate, task gate, or
 *   TSS) and access rights determine the type of call operation to be
 *   performed.
 */

#ifdef CONFIG_DEBUG_CALL_INSTR
static int debug_CALL_RETURN_instr(void)
{
#ifdef CONFIG_DEBUG_NEAR_CALL
    debug_near_call();
#endif
    return 0;
}
device_debugcall(debug_CALL_RETURN_instr);
#endif
