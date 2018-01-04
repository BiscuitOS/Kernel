/*
 * Usermanual of Interrupt on X86
 *
 * (C) 2018.1 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/head.h>
#include <asm/system.h>
#include <test/debug.h>

/*
 * Handle Interrupt or exception from trap gate
 *   Trap CALL and IRET Operation on same privilege level
 *   If the code segment for the handler procedure has the same privilege
 *   privilege level as the currently executing program or task, the handler
 *   procedure uses the current stack. If the handler executes at a more
 *   privileged level, the processor switches to the stack for the handler's
 *   privilege level.
 *
 *   If no stack switch occures, the processor does the following when calling
 *   an interrupt or exception handler:
 *
 *   1) Pushes the current contents of the EFLAGS, CS, and EIP registers (
 *      in the order) on the stack.
 *
 *   2) Pushes an error code (if appropriate) on the stack.
 *
 *   3) Loads the segment selector for the new code segment and the new
 *      instruction pointer (from the interrupt gate or trap gate) into
 *      the SS and EIP registers, respectively.
 *
 *   4) Begins exception of the handler procedure.
 *
 *   Trap CALL Operation on different privilege level
 *   If a stack switch does occur, the processor does the following:
 *
 *   1) Temporarily saves (internally) the current contents of the SS,
 *      ESP, EFLAGS, CS and EIP registers.
 *
 *   2) Loads the segment selector and stack pointer for the new stack (that
 *      is, the stack for the privilege level being called) from the TSS
 *      into the SS and ESP registers and swithes to the new stack.
 *
 *   3) Pushes the temporarily saved SS, ESP, EFLAGS, CS, and EIP values
 *      for the interrupted procedure's stack onto the new stack.
 *
 *   4) Pushes an error code on the new stack (if appropriate).
 *
 *   5) Loads the segment selector for the new code segment and the new
 *      instruction pointer (from the trap gate) into to the CS and EIP
 *      registers, respectively.
 *
 *   6) Begins execution of the handler procedure at the new privilege
 *      level.
 *
 *   A return from an interrupt or exceeption handler is initiated with the
 *   IRET instrcution. The IRET instruction is similar to the far RET 
 *   instruction, exception that is also restores the contents of the EFLAGS
 *   register for the interrupted procedure. When executing a return from
 *   an interrupt or exception handler from the same privilege level as the
 *   interrupt procedure, the processor performs these actions:
 *
 *   1) Restores the CS and EIP registers to their values prior to the 
 *      interrupt or exception.
 *
 *   2) Restores the EFLAGS register.
 *
 *   3) Increments the stack pointer appropriately.
 *
 *   4) Resumes execution of the insterrupted procedure.
 *
 *   Trap CALL Operation on different privilege level
 *   When executing a return from an interrupt or exception handler from
 *   a different privilege level than the interrupt procedure, the 
 *   processor performs these actions:
 *
 *   1) Performs a privilege check.
 *
 *   2) Restores the CS and EIP registers theirs values prior to the 
 *      interrupt or exception.
 *
 *   3) Restore the EFLAGS register.
 *
 *   4) Restore the SS and ESP register to their values prior to the
 *      interrupt or exception, resulting in a stack switch back to
 *      the stack of the interrupt procedure.
 *
 *   5) Resumes execution of the interrupted procedure.
 */
static void handler_trap_gate(void)
{
    unsigned long eflags, eip, error_code;
    unsigned short cs;

    /*
     * Calculate size of local stack:
     *   Utilze 'objdump' to determine the size of local stack.
     */

    /* Obtain informtation from calling procedure
     * Stack: layer 
     * 
     * |------------------------------------------------|
     * |  SS (Calling Procedure)                        |
     * |------------------------------------------------|
     * |  ESP (Calling Procedure)                       |
     * |------------------------------------------------|
     * |  EFLAG (Calling Procedure)                     |
     * |------------------------------------------------|
     * |  Code Segment (Calling Procedure)              |
     * |------------------------------------------------|
     * |  EIP (Calling Procedure)                       |
     * |------------------------------------------------|
     * |  ERROR CODE (Calling Procedure)                |
     * |------------------------------------------------|
     * |  Local Stack (Called Procedure)                |
     * |------------------------------------------------|
     * |  ESP (Current Procedure)                       |
     * |------------------------------------------------|  
     */
    __asm__ volatile ("pushl %%ebp\n\r"
            "movl %%esp, %%ebp\n\r"
            "addl $0x1C, %%esp\n\r"
            "popl %%eax\n\r"
            "popl %%ebx\n\r"
            "popl %%ecx\n\r"
            "popl %%edx\n\r"
            "movl %%ebp, %%esp\n\r"
            "popl %%ebp"
            : "=a" (error_code), "=b" (eip),
              "=c" (cs), "=d" (eflags));
    printk("Trap: eflags[%#x] eip[%#x] cs[%#x] error_code[%#x]\n",
            eflags, eip, cs, error_code);

    /* Return from trap gate: ESP points EIP for calling procedure */
    __asm__ volatile ("addl $0x1C, %%esp\n\r"
            "iret"
            ::);
}

/*
 * Establish Interrupt- or Exception-Handler with trap gate
 *   Call Operation for Interrupt or Exception Handling Procedure
 *   A call to an interrupt or exception handler procedure is similar to a
 *   procedure call to another protection. Here, the vector reference one
 *   of two kinds of gates in the IDT: an interrupt gate or a trap gate.
 *   Interrupt and trap gates are similar to call gates in that they provide
 *   the following information:
 * 
 *   1) Access rights information
 *
 *   2) The segment selector for the code segment that contains the handler
 *      procedure.
 *
 *   3) An offset into the code segment to the first instruction of the 
 *      handler procedure.
 *
 *   The difference between an interrupt gate and a trap gate is as follows.
 *   If an interrupt or exception handler is called through an interrupt 
 *   gate, the processor clear the interrupt enable (IF) flag in the EFLAGS
 *   register to prevent subsequent interrupts from interfering with the
 *   execution of the handler. When a handler is called through a trap gate,
 *   the state of the IF flag is not changed.
 */
static void establish_interrupt_with_trap_gate(void)
{
    /* setup trap on IDT */
    set_trap_gate(0x88, &handler_trap_gate);

    /* trigger specific trap */
    __asm__ ("int $0x88");
}

/*
 * Handle Interrupt or exception from interrupt gate
 *   Interrupt CALL and IRET Operation on same privilege level
 *   If the code segment for the handler procedure has the same privilege
 *   privilege level as the currently executing program or task, the handler
 *   procedure uses the current stack. If the handler executes at a more
 *   privileged level, the processor switches to the stack for the handler's
 *   privilege level.
 *
 *   If no stack switch occures, the processor does the following when calling
 *   an interrupt or exception handler:
 *
 *   1) Pushes the current contents of the EFLAGS, CS, and EIP registers (
 *      in the order) on the stack.
 *
 *   2) Pushes an error code (if appropriate) on the stack.
 *
 *   3) Loads the segment selector for the new code segment and the new
 *      instruction pointer (from the interrupt gate or trap gate) into
 *      the SS and EIP registers, respectively.
 *
 *   4) Clear the IF flag in the EFLAGS register.
 *
 *   5) Begins exception of the handler procedure.
 *
 *   Trap CALL Operation on different privilege level
 *   If a stack switch does occur, the processor does the following:
 *
 *   1) Temporarily saves (internally) the current contents of the SS,
 *      ESP, EFLAGS, CS and EIP registers.
 *
 *   2) Loads the segment selector and stack pointer for the new stack (that
 *      is, the stack for the privilege level being called) from the TSS
 *      into the SS and ESP registers and swithes to the new stack.
 *
 *   3) Pushes the temporarily saved SS, ESP, EFLAGS, CS, and EIP values
 *      for the interrupted procedure's stack onto the new stack.
 *
 *   4) Pushes an error code on the new stack (if appropriate).
 *
 *   5) Loads the segment selector for the new code segment and the new
 *      instruction pointer (from the trap gate) into to the CS and EIP
 *      registers, respectively.
 *
 *   6) Clears the IF flag in the EFLAGS register.
 *
 *   7) Begins execution of the handler procedure at the new privilege
 *      level.
 *
 *   A return from an interrupt or exceeption handler is initiated with the
 *   IRET instrcution. The IRET instruction is similar to the far RET 
 *   instruction, exception that is also restores the contents of the EFLAGS
 *   register for the interrupted procedure. When executing a return from
 *   an interrupt or exception handler from the same privilege level as the
 *   interrupt procedure, the processor performs these actions:
 *
 *   1) Restores the CS and EIP registers to their values prior to the 
 *      interrupt or exception.
 *
 *   2) Restores the EFLAGS register.
 *
 *   3) Increments the stack pointer appropriately.
 *
 *   4) Resumes execution of the insterrupted procedure.
 *
 *   Interrupt CALL Operation on different privilege level
 *   When executing a return from an interrupt or exception handler from
 *   a different privilege level than the interrupt procedure, the 
 *   processor performs these actions:
 *
 *   1) Performs a privilege check.
 *
 *   2) Restores the CS and EIP registers theirs values prior to the 
 *      interrupt or exception.
 *
 *   3) Restore the EFLAGS register.
 *
 *   4) Restore the SS and ESP register to their values prior to the
 *      interrupt or exception, resulting in a stack switch back to
 *      the stack of the interrupt procedure.
 *
 *   5) Resumes execution of the interrupted procedure.
 */
static void handler_interrupt_gate(void)
{
    unsigned long eflags, eip, error_code;
    unsigned short cs;

    /*
     * Calculate size of local stack:
     *   Utilze 'objdump' to determine the size of local stack.
     */

    /* Obtain informtation from calling procedure
     * Stack: layer 
     * 
     * |------------------------------------------------|
     * |  SS (Calling Procedure)                        |
     * |------------------------------------------------|
     * |  ESP (Calling Procedure)                       |
     * |------------------------------------------------|
     * |  EFLAG (Calling Procedure)                     |
     * |------------------------------------------------|
     * |  Code Segment (Calling Procedure)              |
     * |------------------------------------------------|
     * |  EIP (Calling Procedure)                       |
     * |------------------------------------------------|
     * |  ERROR CODE (Calling Procedure)                |
     * |------------------------------------------------|
     * |  Local Stack (Called Procedure)                |
     * |------------------------------------------------|
     * |  ESP (Current Procedure)                       |
     * |------------------------------------------------|  
     */
    __asm__ volatile ("pushl %%ebp\n\r"
            "movl %%esp, %%ebp\n\r"
            "addl $0x1C, %%esp\n\r"
            "popl %%eax\n\r"
            "popl %%ebx\n\r"
            "popl %%ecx\n\r"
            "popl %%edx\n\r"
            "movl %%ebp, %%esp\n\r"
            "popl %%ebp"
            : "=a" (error_code), "=b" (eip),
              "=c" (cs), "=d" (eflags));
    printk("Interrupt: eflags[%#x] eip[%#x] cs[%#x] error_code[%#x]\n",
            eflags, eip, cs, error_code);

    /* Clear TF flag on eflags */

    /* Return from trap gate: ESP points EIP for calling procedure */
    __asm__ volatile ("addl $0x1C, %%esp\n\r"
            "iret"
            ::);
}

/*
 * Establish Interrupt- or Exception-Handler with interrupt gate
 *   Call Operation for Interrupt or Exception Handling Procedure
 *   A call to an interrupt or exception handler procedure is similar to a
 *   procedure call to another protection. Here, the vector reference one
 *   of two kinds of gates in the IDT: an interrupt gate or a trap gate.
 *   Interrupt and trap gates are similar to call gates in that they provide
 *   the following information:
 * 
 *   1) Access rights information
 *
 *   2) The segment selector for the code segment that contains the handler
 *      procedure.
 *
 *   3) An offset into the code segment to the first instruction of the 
 *      handler procedure.
 *
 *   The difference between an interrupt gate and a trap gate is as follows.
 *   If an interrupt or exception handler is called through an interrupt 
 *   gate, the processor clear the interrupt enable (IF) flag in the EFLAGS
 *   register to prevent subsequent interrupts from interfering with the
 *   execution of the handler. When a handler is called through a trap gate,
 *   the state of the IF flag is not changed.
 */
static void establish_interrupt_with_interrupt_gate(void)
{
    /* Setup interrupt on IDT */
    set_intr_gate(0x99, &handler_interrupt_gate);

    /* trigger specific interrupt */
    __asm__ ("int $0x99");
}

/* Common debug entry for interrupt */
void interrupt_useage_common(void)
{
#ifdef CONFIG_DEBUG_KERNEL_LATER
    establish_interrupt_with_trap_gate();
    establish_interrupt_with_interrupt_gate();
#endif
    /* ignore warning */
    if (0) {
        establish_interrupt_with_trap_gate();
        establish_interrupt_with_interrupt_gate();
    }
}
