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

extern int mm_region_count;
extern struct mm_memmap_region mm_phys_regions[MM_MAX_MEMORY_REGIONS];
extern bool pgalloc_initialized;
extern uint64_t hhdm_offset;

void mm_init();
void __early_pgalloc_init();
void *__early_getpage();
void *__early_zgetpage();
void __early_freepage(void *page);
void __early_map_page(uint64_t *pt, void *phys, void *virt, uint64_t flags);
