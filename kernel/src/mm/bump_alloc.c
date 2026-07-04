#include <goofy-os/bump_alloc.h>
#include <goofy-os/hcf.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <stddef.h>
#include <string.h>

unsigned int bump_allocated = 0;
void *bump_page() {
	if (buddy_initialized) {
		printk("Used early allocator after pagealloc initialized!\n");
		hcf();
	}

	unsigned int left = bump_allocated;

	for (int i = 0; i < mm_region_count; i++) {
		unsigned int page_count = mm_phys_regions[i].size >> 12;
		if (page_count > left) {
			unsigned int phys = mm_phys_regions[i].base;
			bump_allocated++;
			return (void *)__va(phys + (left * PAGE_SIZE));
		}
		left -= page_count;
	}
	return NULL;
}

void *bump_zpage() {
	void *pg = bump_page();
	memset(pg, 0, 0x1000);
	return pg;
}