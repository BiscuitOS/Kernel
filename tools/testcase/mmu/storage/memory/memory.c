/*
 * Memory on MMU
 *
 * (C) 2018.06.01 BiscuitOS <buddy.zhang@aliyun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/page.h>

#include <test/debug.h>

#ifdef CONFIG_DEBUG_DETECT_MEMORY

#ifdef CONFIG_MEMORY_FROM_BIOS
/*
 * Memory information on BIOS
 *   The address of setup argument is base on empty_zero_page. The 
 *   empty_zero_page holds more information for system setup. And
 *   we can obtain special memory information from it.
 * 
 *   * EXT_MEM_K
 *     Indicate the size for expand main memory. On Linux 1.0, the kernel
 *     always hold lower 1M memory to storage system code and data. And
 *     EXT_MEM_K is memory size that over 1M Byte. We can compute the 
 *     main memory size as follow:
 *       MainMemory = (1 << 20) + (EXT_MEM_K << 10)
 */
extern char empty_zero_page[PAGE_SIZE];
#define PARAM        empty_zero_page
#define EXT_MEM_K    (*(unsigned short *)(PARAM+2))

static int detect_memory_from_BIOS(void)
{
    printk("EXT-Memory(Byte): %#x\n", (EXT_MEM_K<<10));
    printk("MainMemory:       %#x\n", (1<<20)+(EXT_MEM_K<<10));
    return 0;
}
#endif

#ifdef CONFIG_MEMORY_FROM_LD
/*
 * Memory information from ld
 *   The symbol from ld will indicate the kernel section address.
 *   We can use these symbol to indicate the running address for different
 *   section. More ld symbol information see arch/x86/kernel/vmlinux.lds.S
 *
 *   * __executable_start
 *     The symbol indicate the running start address for kernel code
 *     section. It value usually is same with start address of program.
 * 
 *   * etext/_etext/__etext
 *     The symbol indicate the running end address of kernel code. But on
 *     some architecture, the data section is behind of code section, so
 *     this symbol also indicate the running start address of kernel data.
 *
 *   * edata/_edata/__edata
 *     The symbol indicate the running end address for kernel data, But on
 *     common architecture, the bss section is behind of data section, so
 *     this symbol also indicate the running start address of kernel bss.
 *
 *   * end/_end/__end
 *     The symbol indicate the running end address for kernel. 
 *     The kernel usually contain '.text', '.data' and '.bss' section. 
 *     
 */

static int detect_memory_from_ld(void)
{
    extern char __executable_start[];
    extern char etext[], _etext[], __etext[];
    extern char edata[], _edata[], __edata[];
    extern char end[], __end[];

    printk("Kernel:   %#08x - %#08x\n", 
          (unsigned int)__executable_start, (unsigned int)end);
    printk("Code:     %#08x - %#08x\n", 
          (unsigned int)__executable_start, (unsigned int)etext);
    printk("Data:     %#08x - %#08x\n", 
          (unsigned int)_etext, (unsigned int)edata);
    printk("BSS:      %#08x - %#08x\n", 
          (unsigned int)_edata, (unsigned int)__end);
    printk("__etext:  %#08x\n", (unsigned int)__etext);
    printk("__edata:  %#08x\n", (unsigned int)__edata);

    return 0;
}
#endif

#ifdef CONFIG_MEMORY_FROM_TASK
/*
 * Memory information from task_struct
 *   Task_struct hold current task running inforation that also contain
 *   current task memory information.
 *
 *   * start_code
 *     The current task start address of code section.
 *
 *   * end_code
 *     The current task end address of code section. On common architecture,
 *     the data section is behind of code section, so this value also
 *     indicates the start address of data section.
 *
 *   * end_data
 *     The current task end address of data section.
 *
 *   * start_brk
 *     The current task start address of heap.
 *
 *   * brk
 *     The size of heap for current task.
 *
 *   * start_stack
 *     The start address of stack for current task.
 *
 *   * start_mmap
 *     The start address of mmap for current task.
 */
static int detect_memory_from_task_struct(void)
{
    struct task_struct *task = current;

    printk("Code:     %#08x - %#08x\n", 
          (unsigned int)task->start_code, (unsigned int)task->end_code);
    printk("Data:     %#08x - %#08x\n", 
          (unsigned int)task->end_code, (unsigned int)task->end_data);
    printk("Brk:      %#08x - %#08x\n", 
          (unsigned int)task->start_brk, (unsigned int)task->brk);
    printk("Stack:    %#08x\n", (unsigned int)task->start_stack);
    printk("Mmap:     %#08x\n", (unsigned int)task->start_mmap);

    return 0;
}
#endif

#ifdef CONFIG_MEMORY_FROM_GLOBAL_ARGUMENT
/*
 * Memory information from global argument.
 *   Speical argument indicate the system memory information. We can 
 *   utilize these argment to allocate memory or mmap virtual address.
 *
 *   * high_memory
 */
extern unsigned long high_memory;

static int detect_memory_from_global_argument(void)
{
    printk("HighMemory: %08x\n", (unsigned int)high_memory);
    return 0;
}
#endif

static int memory_detect(void)
{
#ifdef CONFIG_MEMORY_FROM_BIOS
    detect_memory_from_BIOS();
#endif
#ifdef CONFIG_MEMORY_FROM_LD
    detect_memory_from_ld();
#endif
#ifdef CONFIG_MEMORY_FROM_TASK
    detect_memory_from_task_struct();
#endif
#ifdef CONFIG_MEMORY_FROM_GLOBAL_ARGUMENT
    detect_memory_from_global_argument();
#endif
    return 0;
}
late_debugcall(memory_detect);
#endif
