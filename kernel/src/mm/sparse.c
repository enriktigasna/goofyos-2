#include <goofy-os/cpu.h>
#include <goofy-os/mm.h>
#include <stddef.h>

#define STRUCTS_PER_PAGE (0x1000 / sizeof(struct page))

struct page *sparsemap_array = (struct page *)0xffffc00000000000;

void __map_region_struct(struct mm_memmap_region *region, uint64_t *pt) {
	uint64_t curr = region->base;
	while (curr < region->base + region->size) {
		struct page *ptr = &sparsemap_array[curr >> 12];
		uint64_t aligned = (uint64_t)ptr & ~0xfff;

		uint64_t page = (uint64_t)zpgalloc() - hhdm_offset;
		map_page(pt, page, (void *)aligned, PG_NX | PG_WRITE);
		curr += 0x1000 * STRUCTS_PER_PAGE;
	}
}

void sparse_init() {
	// Map in all the pages that should contain struct page
	uint64_t *pt = (uint64_t *)__va(__readcr3());

	for (int i = 0; i < mm_region_count; i++) {
		__map_region_struct(&mm_phys_regions[i], pt);
	}
}