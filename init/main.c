/*
 *  linux/init/main.c
 *
 *  (C) 1991  Linus Torvalds
 */

#define __LIBRARY__
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/tty.h>

#include <asm/io.h>
#include <asm/system.h>

#ifdef CONFIG_TESTCASE
#include <test/debug.h>
#endif

/*
 * we need this inline - forking for kernel space will result
 * in NO COPY ON WRITE (!!!), until an execve is executed. This
 * is no problem, but for the stack. This is handled by not letting
 * main() use the stack at all after fork(). Thus, no function
 * calls - which means inline code for fork too, as otherwise we
 * would use the stack upon exit from 'fork()'.
 *
 * Actually only pause and fork are needed inline, so that there
 * won't be any messing with the stack from main(), but we define
 * some others too.
 */
inline int fork(void) __attribute__((always_inline));
inline int pause(void) __attribute__((always_inline));
inline _syscall0(int, fork)
inline _syscall1(int, setup, void *, BIOS)
inline _syscall0(int, pause)
inline _syscall0(int, sync)

char printbuf[1024];

/*
 * This is set up by the setup-routine at boot-time
 */
#define EXT_MEM_K        (*(unsigned short *)0x90002)
#define DRIVE_INFO       (*(struct drive_info *)0x90080)
#define ORIG_ROOT_DEV    (*(unsigned short *)0x901FC)

/*
 * Yeah, yeah, it's ugly, but I cannot find how to do this correctly
 * and this seems to work. I anybody has more info on the real-time
 * clock I'd be interested. Most of this was trial and error, and some
 * bios-listing reading. Urghh.
 */
#define CMOS_READ(addr)  ({   \
	outb_p(0x80 | addr, 0x70);   \
	inb_p(0x71);                 \
})

#define BCD_TO_BIN(val)  ((val)=((val) & 15) + ((val) >> 4) * 10)

struct drive_info {
	char dummy[32];
} drive_info;

const char *command_line = "loglevel=8 console=ttyS0,115200";
static long memory_end;
static long buffer_memory_end;
static long main_memory_start;

extern void init(void);
extern int vsprintf(char *buf, const char *fmt, va_list args);
extern void mem_init(long, long);
extern void blk_dev_init(void);
extern void chr_dev_init(void);
extern void hd_init(void);
extern void floppy_init(void);
extern long kernel_mktime(struct tm *);
extern long startup_time;

/*
 * Initialize system time.
 */
static void time_init(void)
{
    struct tm time;

    do {
        time.tm_sec = CMOS_READ(0);
        time.tm_min = CMOS_READ(2);
        time.tm_hour = CMOS_READ(4);
        time.tm_mday = CMOS_READ(7);
        time.tm_mon = CMOS_READ(8);
        time.tm_year = CMOS_READ(9);
    } while (time.tm_sec != CMOS_READ(0));

    BCD_TO_BIN(time.tm_sec);
    BCD_TO_BIN(time.tm_min);
    BCD_TO_BIN(time.tm_hour);
    BCD_TO_BIN(time.tm_mday);
    BCD_TO_BIN(time.tm_mon);
    BCD_TO_BIN(time.tm_year);
    time.tm_mon--;
    startup_time = kernel_mktime(&time);
}

/*
 * Detect memory from setup-routine.
 */
static void memory_detect(void)
{
    memory_end = (1 << MB_SHIFT) + (EXT_MEM_K << KB_SHIFT);
    memory_end &= 0xFFFFF000;

    /* Current version only support litter than 16Mb */
    if (memory_end > 16 << MB_SHIFT)
        memory_end = 16 << MB_SHIFT;
    if (memory_end > 12 << MB_SHIFT)
        buffer_memory_end = 4 << MB_SHIFT;
    else if (memory_end > 6 << MB_SHIFT)
        buffer_memory_end = 2 << MB_SHIFT;
    else
        buffer_memory_end = 1 << MB_SHIFT;

    main_memory_start = buffer_memory_end;
}

int main(void)
{
    /*
     * Interrupts are still disabled. Do necessary setups, then
     * enable them.
     */
#ifdef CONFIG_DEBUG_KERNEL_EARLY
    debug_on_kernel_early();
#endif
    ROOT_DEV = ORIG_ROOT_DEV;
    drive_info = DRIVE_INFO;
    memory_detect();
    mem_init(main_memory_start, memory_end);
    trap_init();
    blk_dev_init();
    chr_dev_init();
    tty_init();
    time_init();
    sched_init();
    buffer_init(buffer_memory_end);
#ifdef CONFIG_HARDDISK
    hd_init();
#endif
#ifdef CONFIG_FLOPPY
    floppy_init();
#endif
    sti();
#ifdef CONFIG_DEBUG_KERNEL_LATER
    debug_on_kernel_later();
#endif
#if defined (CONFIG_DEBUG_USERLAND_EARLY) || \
    defined (CONFIG_DEBUG_USERLAND_SHELL)
    debug_kernel_on_userland_stage();
#endif
    move_to_user_mode();
    if (!fork()) {   /* we count on this going ok */
        init();
    }
/*
 * NOTE! For any other task 'pause()' would mean we have to get a
 * signal to awaken, but task0 is the sole exception (see 'schedule()')
 * as task 0 gets activated at every idle moment (when no other tasks
 * can run). For task0 'pause()' just means we go check if some other
 * task can run, and if not we return here.
 */
    for (;;)
        pause();
}

int printf(const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    write(1, printbuf, i = vsprintf(printbuf, fmt, args));
    va_end(args);
    return i;
}

static char *argv_rc[] = { "/bin/sh", NULL };
static char *envp_rc[] = { "HOME=/", NULL };

static char *argv[] = { "-/bin/sh", NULL };
static char *envp[] = { "HOME=/usr/root", NULL };

void init(void)
{
    int pid, i;

    setup((void *)&drive_info);
    (void)open("/dev/tty0", O_RDWR, 0);
    (void)dup(0);
    (void)dup(0);
    printf("%d buffers = %d bytes buffer space\n\r", NR_BUFFERS,
            NR_BUFFERS * BLOCK_SIZE);
    printf("Free mem: %d bytes\n\r", memory_end - main_memory_start);
#ifdef CONFIG_DEBUG_USERLAND_SYSCALL
    debug_on_userland_syscall();
#endif
    if (!(pid = fork())) {
        close(0);
        if (open("/etc/rc", O_RDONLY, 0))
            _exit(1);
        execve("/bin/sh", argv_rc, envp_rc);
        _exit(2);
    }
    if (pid > 0)
        while (pid != wait(&i))
            /* nothing */;
    while (1) {
        if ((pid = fork()) < 0) {
            printf("Fork failed in init\r\n");
            continue;
        }
        if (!pid) {
            close(0);
            close(1);
            close(2);
            setsid();
            (void) open("/dev/tty0", O_RDWR, 0);
            (void) dup(0);
            (void) dup(0);
            _exit(execve("/bin/sh", argv, envp));
        }
        while (1)
            if (pid == wait(&i))
                break;
        printf("\n\rchild %d died with code %04x\n\r", pid, i);
        sync();
    }
    _exit(0);      /* NOTE! _exit, not exit() */
}
