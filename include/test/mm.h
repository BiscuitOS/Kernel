#ifndef _TESTCASE_MM_H
#define _TESTCASE_MM_H

#ifdef CONFIG_DEBUG_MMU
struct logic_addr
{
    unsigned int offset;
    unsigned short sel;
};

extern int debug_mmu_common(void);
extern int debug_mmu_common_userland(void);

#ifdef CONFIG_DEBUG_MMU_LINEAR
extern int debug_mmu_linear_common(void);

#ifdef CONFIG_DEBUG_LINEAR_ADDR
extern int debug_linear_address_common(void);
#endif

#endif // CONFIG_DEBUG_MMU_LINEAR

#ifdef CONFIG_DEBUG_MMU_PHYSIC
extern int debug_mmu_physic_common(void);
#endif // CONFIG_DEBUG_MMU_PHYSIC

#ifdef CONFIG_DEBUG_MMU_VIRTUAL
extern int debug_mmu_virtual_common(void);
extern int debug_mmu_virtual_common_userland(void);
extern void debug_virtual_address_common_userland(void);
#endif // CONFIG_DEBUG_VIRTUAL

#ifdef CONFIG_DEBUG_MMU_PGTABLE
extern int debug_mmu_pgtable_common(void);

#ifdef CONFIG_DEBUG_PGDIR
// PGDIR Structure
struct pgdir_node
{
    unsigned int dir;
};

extern void debug_pgdir_common(void);
#endif

#ifdef CONFIG_DEBUG_PGTABLE
// PG-TABLE Structure
struct pgtable_node
{
    unsigned int tb;
};

extern void debug_page_table_common(void);
#endif

#endif // CONFIG_DEBUG_PGTABLE

#ifdef CONFIG_DEBUG_MMU_SEGMENTATION
extern int debug_mmu_segmentation_common(void);


#endif // CONFIG_DEBUG_MMU_SEGMENTATION

#ifdef CONFIG_DEBUG_MMU_LOGIC

extern int debug_mmu_logical_common(void);

#ifdef CONFIG_DEBUG_LOGIC
extern void debug_logic_address_common(void);
#endif

#endif // CONFIG_DEBUG_MMU_LOGIC

#ifdef CONFIG_DEBUG_MMU_PAGING
extern int debug_mmu_paging_common(void);

#ifdef CONFIG_DEBUG_PAGING_REGISTER
extern int debug_paging_register_common(void);
#endif

#endif // CONFIG_DEBUG_MMU_PAGING

#ifdef CONFIG_DEBUG_MMU_ACCESS
extern int debug_mmu_access_right_common(void);

#ifdef CONFIG_DEBUG_ACCESS_RIGHT
extern int debug_access_right_common(void);
#endif

#endif // CONFIG_DEBUG_MMU_ACCESS

#ifdef CONFIG_DEBUG_MMU_PAGE_FAULT
extern int debug_mmu_page_fault_common(void);

#ifdef CONFIG_DEBUG_PAGE_FAULT
extern int debug_page_fault_common(void);
#endif

#endif // CONFIG_DEBUG_MMU_PAGE_FAULT

#ifdef CONFIG_DEBUG_MMU_TLB
extern int debug_mmu_TLB_common(void);

#ifdef CONFIG_DEBUG_TLB
extern int debug_TLB_common(void);
#endif

#endif // CONFIG_DEBUG_MMU_TLB

#ifdef CONFIG_DEBUG_MMU_KMALLOC
extern int debug_mmu_kmalloc_common(void);

extern int debug_kmalloc_allocator_common(void);

#endif // CONFIG_DEBUG_MMU_KMALLOC

#endif // CONFIG_DEBUG_MMU
#endif
