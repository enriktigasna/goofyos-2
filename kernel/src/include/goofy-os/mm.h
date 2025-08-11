#pragma once

#include <goofy-os/spinlock.h>
#include <stdbool.h>
#include <stdint.h>

#define MM_MAX_MEMORY_REGIONS 0x20
#define __va(phys) ((uint64_t)phys + hhdm_offset)

#define PG_PRESENT 0x1ULL
#define PG_WRITE 0x2ULL
#define PG_USER 0x4ULL
#define PG_NX 0x8000000000000000ULL
#define PG_FLAGMASK (PG_WRITE | PG_NX | PG_USER)

struct mm_memmap_region {
	uint64_t base;
	uint64_t size;
};

struct kmem_struct {
	struct spinlock lock;
};

extern int mm_region_count;
extern struct mm_memmap_region mm_phys_regions[MM_MAX_MEMORY_REGIONS];
extern bool pgalloc_initialized;
extern uint64_t hhdm_offset;

void mm_init();
void pgalloc_init();
void *pgalloc();
void *zpgalloc();
void pgfree(void *page);
void map_page(uint64_t *pt, uint64_t phys, void *virt, uint64_t flags);

void sparse_init();
bool is_mapped(void *virt);

struct page {
	union {
		struct {
			uint64_t flags;
		};
		uint64_t padding[8];
	};
};

extern struct page *sparsemap_array;