#ifndef _DEBUG_H
#define _DEBUG_H

/*
 * Used for debug calls..
 */
typedef int (*debugcall_t)(void);

#define __used      __attribute__((__used__))

/*
 * debugcalls are now grouped by functionality into separate
 * subsections. Ordering inside the subsections is determined
 * by link order.
 * For backwards compatibilty, debugcall() puts the call in the
 * device ini subsection.
 *
 * The 'id' arg to __define_debugcall() is needed so that multiple
 * debugcalls can point at the same hander without causing duplicate-
 * symbol build errors.
 */
#define __define_debugcall(fn,id) \
    static debugcall_t __debugcall_##fn##id __used \
        __attribute__((__section__(".debugcall"#id".debug"))) = fn;

/*
 * Invake debugcall function
 */
#define debugcall(fn) \
    __asm__ ("call *%0" :: "r" (fn))

/*
 * Early debugcalls run before initializing SMP
 *
 * Only for built-in code, not modules
 */
#define early_debugcall(fn)             __define_debugcall(fn,early)
/*
 * A "pure" debugcall has no dependencies on anything else, and purely
 * debug variables that couldn't be statically debuged.
 *
 * This only exists for build-in code, not for modules.
 * Keep main.c: debugcall_level_anmes[] in sync.
 */
#define pure_debugcall(fn)              __define_debugcall(fn,0)

#define core_debugcall(fn)              __define_debugcall(fn,1)
#define core_debugcall_sync(fn)         __define_debugcall(fn,1s)
#define postcore_debugcall(fn)          __define_debugcall(fn,2)
#define postcore_debugcall_sync(fn)     __define_debugcall(fn,2s)
#define arch_debugcall(fn)              __define_debugcall(fn,3)
#define arch_debugcall_sync(fn)         __define_debugcall(fn,3s)
#define subsys_debugcall(fn)            __define_debugcall(fn,4)
#define subsys_debugcall_sync(fn)       __define_debugcall(fn,4s)
#define fs_debugcall(fn)                __define_debugcall(fn,5)
#define fs_debugcall_sync(fn)           __define_debugcall(fn,5s)
#define rootfs_debugcall(fn)            __define_debugcall(fn,rootfs)
#define device_debugcall(fn)            __define_debugcall(fn,6)
#define device_debugcall_sync(fn)       __define_debugcall(fn,6s)
#define late_debugcall(fn)              __define_debugcall(fn,7)
#define late_debugcall_sync(fn)         __define_debugcall(fn,7s)

/*
 * Debug levels
 */
enum {
    DEBUG_EARLY,
    DEBUG_PURE,
    DEBUG_CORE,
    DEBUG_CORE_SYNC,
    DEBUG_POSTCORE,
    DEBUG_POSTCORE_SYNC,
    DEBUG_ARCH,
    DEBUG_ARCH_SYNC,
    DEBUG_SUBSYS,
    DEBUG_SUBSYS_SYNC,
    DEBUG_FS,
    DEBUG_FS_SYNC,
    DEBUG_ROOTFS,
    DEBUG_DEVICE,
    DEBUG_DEVICE_SYNC,
    DEBUG_LATE,
    DEBUG_LATE_SYNC
};

extern void do_debugcall(int levels);

#endif
