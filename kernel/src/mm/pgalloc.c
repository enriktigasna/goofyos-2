#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/spinlock.h>
#include <stddef.h>
#include <string.h>

/*
 * Early allocator with inline freelists
 * Used to bootstrap memory management
 */

struct kmem_struct kmem;

struct pgalloc_chunk {
	struct pgalloc_chunk *next;
	uint64_t page_count;
};

bool pgalloc_initialized = false;
struct pgalloc_chunk *pgalloc_head;
void pgalloc_init() {
	for (int i = 0; i < mm_region_count; i++) {
		uint64_t region_base = __va(mm_phys_regions[i].base);
		uint64_t page_count = mm_phys_regions[i].size >> 12;

		struct pgalloc_chunk *curr =
		    (struct pgalloc_chunk *)region_base;
		curr->next = pgalloc_head;
		curr->page_count = page_count;

		pgalloc_head = curr;
	}

	pgalloc_initialized = true;
}

void *pgalloc() {
	acquire(&kmem.lock);
	if (!pgalloc_head) {
		release(&kmem.lock);
		return NULL;
	}

	struct pgalloc_chunk *curr = pgalloc_head;
	if (curr->page_count == 1) {
		pgalloc_head = curr->next;

		release(&kmem.lock);
		return curr;
	}

	struct pgalloc_chunk *next = curr->next;
	uint64_t page_count = curr->page_count;

	pgalloc_head = (struct pgalloc_chunk *)((uint64_t)curr + 0x1000);
	pgalloc_head->next = next;
	pgalloc_head->page_count = page_count - 1;

	release(&kmem.lock);
	return curr;
}

struct page *pgalloc_phys() { return __hhdm_to_page(pgalloc()); }

void *zpgalloc() {
	void *page = pgalloc();

	if (page) {
		memset(page, 0, 0x1000);
	}

	return page;
}

void pgfree(void *page) {
	acquire(&kmem.lock);
	struct pgalloc_chunk *curr = page;
	curr->next = pgalloc_head;
	pgalloc_head = curr;
	release(&kmem.lock);
}