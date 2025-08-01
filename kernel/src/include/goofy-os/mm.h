#include <stdbool.h>
#include <stdint.h>

#define MM_MAX_MEMORY_REGIONS 0x20
#define __va(phys) phys + hhdm_offset;

struct mm_memmap_region {
	uint64_t base;
	uint64_t size;
};

extern int mm_region_count;
extern struct mm_memmap_region mm_phys_regions[MM_MAX_MEMORY_REGIONS];
extern bool pgalloc_initialized;
extern uint64_t hhdm_offset;

void mm_init();
void __early_pgalloc_init();
void *__early_getpage();
void *__early_zgetpage();
void __early_freepage(void *page);