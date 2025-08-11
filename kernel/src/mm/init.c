#include <goofy-os/boot.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <limine.h>
#include <stdint.h>

int mm_region_count = 0;
uint64_t hhdm_offset;
struct mm_memmap_region mm_phys_regions[MM_MAX_MEMORY_REGIONS];

static void setup_memmap_regions() {
	int idx = 0;
	for (int i = 0; i < __limine_memmap_response->entry_count; i++) {
		struct limine_memmap_entry *entry =
		    __limine_memmap_response->entries[idx++];
		if (entry->type == LIMINE_MEMMAP_USABLE) {
			mm_phys_regions[mm_region_count].base = entry->base;
			mm_phys_regions[mm_region_count].size = entry->length;

			mm_region_count++;
		}
	}
}

void mm_init() {
	hhdm_offset = __limine_hhdm_response->offset;
	setup_memmap_regions();

	pgalloc_init();
	sparse_init();
}