#ifndef _DEBUG_BINARY_H
#define _DEBUG_BINARY_H

#ifdef CONFIG_DEBUG_BINARY
extern int debug_binary_common(void);
extern int debug_binary_common_userland(void);

#ifdef CONFIG_DEBUG_BINARY_AOUT
extern int debug_binary_aout_common_userland(void);

#ifdef CONFIG_DEBUG_AOUT_EXEC
extern int debug_binary_aout_exec(void);
extern int d_do_execve(unsigned long *eip, long tmp, char *filename,
              char **argv, char **envp);
#endif

#ifdef CONFIG_DEBUG_AOUT_FORMAT
extern int debug_binary_aout_format(void);
#endif

#endif // CONFIG_DEBUG_BINARY_AOUT

#ifdef CONFIG_DEBUG_BINARY_ELF
extern int debug_binary_elf_common_userland(void);

#ifdef CONFIG_DEBUG_ELF_FORMAT
extern int debug_binary_elf_format(void);
#endif

#endif // CONFIG_DEBUG_BINARY_ELF

#endif // CONFIG_DEBUG_BINARY

#endif
