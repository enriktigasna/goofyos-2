#pragma once

#include <goofy-os/list.h>
#include <goofy-os/spinlock.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_ORDER 15
#define N_ORDERS MAX_ORDER + 1
#define MAX_BUDDY_ZONES 0x20

#define MM_MAX_MEMORY_REGIONS 0x20
#define __va(phys) ((uint64_t)phys + hhdm_offset)
#define __hhdm_to_page(addr)                                                   \
	&sparsemap_array[((uint64_t)(addr) - hhdm_offset) >> 12]

#define page_to_pfn(_page) ((struct page *)(_page) - sparsemap_array)
#define page_to_virt(page) (pfn_to_virt(page_to_pfn(page)))
#define pfn_to_page(pfn) (&sparsemap_array[pfn])

// If a chunk is for example of order 3, we can find the first page in this
// chunk, because it will be aligned to (1 << 3) = 8
#define base_pfn(pfn, order) ((pfn) & ~((1 << (order)) - 1))
#define base_page(page, order) (pfn_to_page(base_pfn(page_to_pfn(page), order)))

#define PG_PRESENT (0x1ULL << 0)
#define PG_WRITE (1ULL << 1)
#define PG_USER (1ULL << 2)
#define PG_NX (1ULL << 63)
#define PG_PS (1ULL << 6)

#define PG_FLAGMASK (PG_PRESENT | PG_WRITE | PG_USER | PG_NX | PG_PS)

#define PAGE_SIZE 4096

#define PAGE_FLAG_FREE 1

struct mm_memmap_region {
	uint64_t base;
	uint64_t size;
};

struct page_table {
	uint64_t *cr3;
	struct spinlock lock;
};

struct page_directory {
	uint64_t *pd;
	struct spinlock lock;
};

struct kmem_struct {
	struct spinlock lock;
};

extern int mm_region_count;
extern struct mm_memmap_region mm_phys_regions[MM_MAX_MEMORY_REGIONS];
extern bool buddy_initialized;
extern uint64_t hhdm_offset;
extern struct page_table kernel_virtual_pt;
extern void *(*mapper_alloc_zpage)();

void mm_init();
void buddy_init();
void *pgalloc();
void *zpgalloc();
void pgfree(void *page);
void map_page(struct page_table *pt, uint64_t phys, void *virt, uint64_t flags);
void unmap_page(struct page_table *pt, void *virt);

void kernel_top_pgt_init();
void sparse_init();
bool is_mapped(struct page_table *pt, void *virt);
int virt_to_pfn(void *virt);
void *pfn_to_virt(unsigned long pfn);

struct page {
	union {
		struct {
			unsigned long flags;
			union {
				struct {
					// page owned by a slab
					struct slab *slab;
				};
				struct {
					// page owned by mempool
					struct mempool *mempool;
					unsigned int n_free;
				};
				struct {
					// buddy free page
					struct list_head buddy_list;
					long buddy_order;
				};
			};
		};
		uint64_t padding[8];
	};
};

extern struct page *sparsemap_array;
struct page_table *new_page_table();