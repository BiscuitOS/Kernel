#ifndef _DEBUG_BINARY_H
#define _DEBUG_BINARY_H

#ifdef CONFIG_DEBUG_BINARY
extern int debug_binary_common(void);
extern int debug_binary_common_userland(void);

#ifdef CONFIG_DEBUG_BINARY_AOUT
extern int debug_binary_aout_common_userland(void);

#ifdef CONFIG_DEBUG_AOUT_EXEC
int debug_binary_aout_exec(void);
#endif

#ifdef CONFIG_DEBUG_AOUT_FORMAT
int debug_binary_aout_format(void);
#endif

#endif // CONFIG_DEBUG_BINARY_AOUT

#endif // CONFIG_DEBUG_BINARY

#endif
