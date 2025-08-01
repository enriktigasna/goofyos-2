#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <stddef.h>
#include <string.h>

/*
 * Early allocator with inline freelists
 * Used to bootstrap memory management
 */

struct early_chunk {
	struct early_chunk *next;
	uint64_t page_count;
};

bool early_pgalloc_initialized = false;
struct early_chunk *early_pgalloc_head;
void __early_pgalloc_init() {
	for (int i = 0; i < mm_region_count; i++) {
		uint64_t region_base = __va(mm_phys_regions[i].base);
		uint64_t page_count = mm_phys_regions[i].size >> 12;

		struct early_chunk *curr = (struct early_chunk *)region_base;
		curr->next = early_pgalloc_head;
		curr->page_count = page_count;

		early_pgalloc_head = curr;
	}

	early_pgalloc_initialized = true;
}

void *__early_getpage() {
	if (!early_pgalloc_head) {
		return NULL;
	}

	struct early_chunk *curr = early_pgalloc_head;
	if (curr->page_count == 1) {
		early_pgalloc_head = curr->next;
		return curr;
	}

	struct early_chunk *next = curr->next;
	uint64_t page_count = curr->page_count;

	early_pgalloc_head = (struct early_chunk *)((uint64_t)curr + 0x1000);
	early_pgalloc_head->next = next;
	early_pgalloc_head->page_count = page_count - 1;
	return curr;
}

void *__early_zgetpage() {
	void *page = __early_getpage();
	memset(page, '0', 0x1000);
	return page;
}

void __early_freepage(void *page) {
	struct early_chunk *curr = page;
	curr->next = early_pgalloc_head;
	early_pgalloc_head = curr;
}