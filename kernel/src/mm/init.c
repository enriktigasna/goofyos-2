#include <goofy-os/boot.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/vmalloc.h>
#include <limine.h>
#include <stdint.h>

int mm_region_count = 0;
uint64_t hhdm_offset;

// Should only be used early on!
struct mm_memmap_region mm_phys_regions[MM_MAX_MEMORY_REGIONS];

static void setup_memmap_regions()
{
	struct limine_memmap_entry *entry;
	struct limine_memmap_entries **entries;
	entries = __limine_memmap_response->entries;
	for (int i = 0; i < __limine_memmap_response->entry_count; i++) {
		entry = entries[i];

		if (entry->type == LIMINE_MEMMAP_USABLE) {
			mm_phys_regions[mm_region_count].base = entry->base;
			mm_phys_regions[mm_region_count].size = entry->length;

			mm_region_count++;
		}
	}
}

/**
 * What is needed to set up a buddy allocator?
 *
 * - 0,1,2,3 <- Orders
 * Need one freelist per order
 */

void mm_init()
{
	setup_memmap_regions();
	// Need to set up a temporary pagealloc, then pages, and then real
	kernel_top_pgt_init();
	sparse_init();
	buddy_init();
	slab_init();

	/**

	vmalloc_init();
	*/
}