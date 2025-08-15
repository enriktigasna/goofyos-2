#pragma once

#include <goofy-os/spinlock.h>
#include <stdbool.h>
#include <stdint.h>

#define MM_MAX_MEMORY_REGIONS 0x20
#define __va(phys) ((uint64_t)phys + hhdm_offset)
#define __hhdm_to_page(addr)                                                   \
	&sparsemap_array[((uint64_t)(addr) - hhdm_offset) >> 12]

#define PG_PRESENT (0x1ULL << 0)
#define PG_WRITE (1ULL << 1)
#define PG_USER (1ULL << 2)
#define PG_NX (1ULL << 63)
#define PG_PS (1ULL << 6)

#define PG_FLAGMASK (PG_PRESENT | PG_WRITE | PG_USER | PG_NX | PG_PS)

struct mm_memmap_region {
	uint64_t base;
	uint64_t size;
};

struct page_table {
	uint64_t *cr3;
	struct spinlock lock;
};

struct kmem_struct {
	struct spinlock lock;
};

extern int mm_region_count;
extern struct mm_memmap_region mm_phys_regions[MM_MAX_MEMORY_REGIONS];
extern bool pgalloc_initialized;
extern uint64_t hhdm_offset;
extern struct page_table *kernel_virtual_pt;

void mm_init();
void pgalloc_init();
void *pgalloc();
void *zpgalloc();
void pgfree(void *page);
void map_page(struct page_table *pt, uint64_t phys, void *virt, uint64_t flags);
void unmap_page(struct page_table *pt, void *virt);

void kernel_top_pgt_init();
void sparse_init();
bool is_mapped(struct page_table *pt, void *virt);
uint64_t virt_to_phys(void *virt);

struct page {
	union {
		struct {
			uint64_t flags;
			struct slab *slab;
		};
		uint64_t padding[8];
	};
};

extern struct page *sparsemap_array;