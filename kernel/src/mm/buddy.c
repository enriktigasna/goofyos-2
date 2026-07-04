#include <goofy-os/buddy.h>
#include <goofy-os/bump_alloc.h>
#include <goofy-os/hcf.h>
#include <goofy-os/list.h>
#include <goofy-os/mem_macros.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/spinlock.h>
#include <stddef.h>
#include <string.h>

/*
 * Early allocator with inline freelists
 * Used to bootstrap memory management
 */

void *pfn_to_virt(int pfn) { return (void *)((pfn << 12) + hhdm_offset); }

struct kmem_struct kmem;

struct buddy_freelist {
	struct list_head *head;
	unsigned int count;
};

struct buddy_zone {
	unsigned int pfn_base;
	unsigned int size;
};

struct buddy_freelist buddy_lists[N_ORDERS];
struct buddy_zone buddy_zones[MAX_BUDDY_ZONES];

bool buddy_initialized = false;
struct pgalloc_chunk *pgalloc_head;

bool valid_buddy_pfn(int pfn) {
	for (int i = 0; i < MAX_BUDDY_ZONES; i++) {
		if (buddy_zones[i].pfn_base <= pfn &&
		    (buddy_zones[i].pfn_base + buddy_zones[i].size) > pfn)
			return true;
	}
	return false;
}

// Remove a page from it's freelist
void remove_from_freelist(struct page *page) {
	if (buddy_lists[page->buddy_order].head == &page->buddy_list)
		buddy_lists[page->buddy_order].head = page->buddy_list.next;
	list_remove_node(&page->buddy_list);
	buddy_lists[page->buddy_order].count--;
	page->flags &= ~PAGE_FLAG_FREE;
}

void free_pages_nolock(struct page *page, int order) {
	int pfn = page - sparsemap_array;
	int buddy_pfn = GET_BUDDY(pfn, order);
	struct page *buddy_page = &sparsemap_array[buddy_pfn];

	// Is it valid?
	if (!valid_buddy_pfn(buddy_pfn))
		goto direct_free;
	// Is it free?
	if (!(buddy_page->flags & PAGE_FLAG_FREE))
		goto direct_free;
	// Is it same order?
	if (buddy_page->buddy_order != order)
		goto direct_free;
	// Is it not max order?
	if (buddy_page->buddy_order == MAX_ORDER)
		goto direct_free;

	// Merge the buddy
	remove_from_freelist(buddy_page);
	free_pages_nolock(buddy_page, order + 1);
	return;

	// We can't merge it, free it normally
direct_free:
	page->buddy_order = order;
	page->flags |= PAGE_FLAG_FREE;
	list_add_front(buddy_lists[order].head, &page->buddy_list);
	buddy_lists[order].head = &page->buddy_list;
	// printk("order is %d head is %p\n", order, &page->buddy_list);
	buddy_lists[order].count++;
}

void free_pages(struct page *page, int order) {
	acquire(&kmem.lock);
	free_pages_nolock(page, order);
	release(&kmem.lock);
}

void free_page(struct page *page) { free_pages(page, 0); }

struct page *alloc_pages_nolock(int order, int flags) {
	int pfn;
	struct page *ret;
	// First check for same order freelist

	if (buddy_lists[order].count > 0) {
		ret = container_of(buddy_lists[order].head, struct page,
				   buddy_list);
		buddy_lists[order].head = buddy_lists[order].head->next;
		buddy_lists[order].count--;
		return ret;
	}
	if (order == MAX_ORDER)
		return NULL;
	// Allocate 1 order higher, then free the buddy
	ret = alloc_pages_nolock(order + 1, 0);
	if (ret == NULL)
		return NULL;

	pfn = ret - sparsemap_array;
	int pfn_buddy = GET_BUDDY(pfn, order);
	free_pages(&sparsemap_array[pfn_buddy], order);
	ret->flags &= ~PAGE_FLAG_FREE;

	return ret;
}

struct page *alloc_pages(int order, int flags) {
	struct page *ret;
	acquire(&kmem.lock);
	ret = alloc_pages_nolock(order, flags);
	release(&kmem.lock);

	if (flags & BUDDY_ZERO)
		memset(pfn_to_virt(ret - sparsemap_array), 0,
		       PAGE_SIZE << order);

	return ret;
}

struct page *alloc_page() { return alloc_pages(0, 0); }

void *pgalloc() {
	struct page *page = alloc_pages(0, 0);
	return pfn_to_virt(page - sparsemap_array);
}

void *pgzalloc() {
	struct page *page = alloc_pages(0, BUDDY_ZERO);
	void *virt = pfn_to_virt(page - sparsemap_array);
	return virt;
}

struct page *pgalloc_phys() { return __hhdm_to_page(pgalloc()); }

void pgfree(void *page) {
	struct page *_page = &sparsemap_array[virt_to_pfn(page)];
	free_page(_page);
}

void buddy_init() {
	// First N available pages will be allocated for the page
	// Skip the things allocated by bump allocator

	for (int i = 0; i < mm_region_count; i++) {
		buddy_zones[i].pfn_base = mm_phys_regions[i].base >> 12;
		buddy_zones[i].size = mm_phys_regions[i].size >> 12;
	}

	// Iterate through buddy_zones, skip first n
	// TODO: Optimize, check alignmnent and if enough is left, free big
	// orders at once
	int page_idx = 0;
	for (int zone_idx = 0; zone_idx < MAX_BUDDY_ZONES; zone_idx++) {
		for (int i = 0; i < buddy_zones[zone_idx].size; i++) {
			if (page_idx < bump_allocated) {
				page_idx++;
				continue;
			}

			int pfn = buddy_zones[zone_idx].pfn_base + i;
			free_page(&sparsemap_array[pfn]);

			page_idx++;
		}
	}

	buddy_initialized = true;
	mapper_alloc_zpage = pgzalloc;
	for (int i = 0; i < N_ORDERS; i++) {
		struct list_head *cur = buddy_lists[i].head;
		for (int j = 0; j < buddy_lists[i].count; j++) {
			cur = cur->next;
		}
	}
}