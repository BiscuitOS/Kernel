#ifndef _DEBUG_VFS_H
#define _DEBUG_VFS_H

#ifdef CONFIG_DEBUG_VFS
extern int debug_vfs_common(void);

#ifdef CONFIG_DEBUG_VFS_BUFFER
extern int debug_vfs_buffer(void);

#ifdef CONFIG_DEBUG_BUFFER_FREELIST
extern int debug_free_list(void);
#endif

#endif

#endif

#endif
