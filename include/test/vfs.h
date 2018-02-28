#ifndef _DEBUG_VFS_H
#define _DEBUG_VFS_H

#ifdef CONFIG_DEBUG_VFS
extern int debug_vfs_common(void);
extern int debug_vfs_common_userland(void);

#ifdef CONFIG_DEBUG_VFS_BUFFER
extern int debug_vfs_buffer(void);

#ifdef CONFIG_DEBUG_BUFFER_FREELIST
extern int debug_free_list(void);
#endif

#endif // End of CONFIG_DEBUG_VFS_BUFFER

#ifdef CONFIG_DEBUG_VFS_MINIXFS
extern int debug_vfs_minixfs_common_userland(void);

#ifdef CONFIG_DEBUG_MINIXFS_BLOCK
extern int debug_vfs_minixfs_userland(void);
#endif

#endif // End of CONFIG_DEBUG_VFS_MINIXFS

#endif

#endif
