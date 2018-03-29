#ifndef _DEBUG_SEGMENT_H
#define _DEBUG_SEGMENT_H

#ifdef CONFIG_DEBUG_SEGMENT
extern int debug_segment_common(void);
extern int debug_segment_common_userland(void);

#ifdef CONFIG_DEBUG_SEGMENT_FS
extern int debug_binary_aout_exec(void);
extern int debug_segment_fs_common_userland(void);
#endif // CONFIG_DEBUG_SEGMENT_FS

#ifdef CONFIG_DEBUG_SEGMENT_OBTAIN_USERLAND
extern int obtain_data_from_userland(const char *file, char **argv);
#endif

#ifdef CONFIG_DEBUG_SEGMENT_PUSH_KERNEL
extern int copy_kernel_to_userland(char *userland);
#endif

#endif // CONFIG_DEBUG_SEGMENT

#endif
