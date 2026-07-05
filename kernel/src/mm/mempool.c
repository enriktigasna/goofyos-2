#include <goofy-os/buddy.h>
#include <goofy-os/hcf.h>
#include <goofy-os/list.h>
#include <goofy-os/mempool.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>

void mempool_init(struct mempool *mempool, unsigned int size,
		  unsigned int order) {
	if (size < sizeof(struct list_head)) {
		printk("mempool_init size < sizeof(struct list_head) %d<%d\n",
		       size, sizeof(struct mempool));
		hcf();
	}

	if (size > PAGE_SIZE) {
		printk("mempool_init unsupported size %d\n", size);
		hcf();
	}

	mempool->freelist = NULL;
	mempool->count = 0;
	mempool->size = size;
	mempool->order = order;
}

void mempool_free_nolock(struct mempool *mempool, void *chunk) {
	struct page *page = __hhdm_to_page(chunk);
	struct page *base = base_page(page, mempool->order);
	int max_count = (PAGE_SIZE << mempool->order) / mempool->size;
	list_add_front(&mempool->freelist, chunk);
	base->n_free++;

	if (base->n_free == max_count) {
		free_pages(base, mempool->order);
	}
}

void mempool_free(struct mempool *mempool, void *chunk) {
	acquire(&mempool->lock);
	mempool_free_nolock(mempool, chunk);
	release(&mempool->lock);
}

void *mempool_alloc_nolock(struct mempool *mempool) {
	if (mempool->count > 0) {
		void *ret = list_pop_front(&mempool->freelist);
		struct page *page = __hhdm_to_page(ret);
		struct page *base = base_page(page, mempool->order);
		base->n_free--;
		return ret;
	}

	int count = (PAGE_SIZE << mempool->order) / mempool->size;
	struct page *page = alloc_pages(mempool->order, BUDDY_ZERO);
	if (page == NULL)
		return NULL;

	// All of them will have a pointer to mempool.
	// First one will have n_free, they can find first one by order
	for (int i = 0; i < (1 << mempool->order); i++) {
		page[i].mempool = mempool;
	}
	// First one will be returned, the others will be made freelists
	struct list_head *head = NULL;
	void *new_page = page_to_virt(page);

	for (int i = 0; i < count; i++) {
		list_add_front(&head, new_page + (mempool->size * i));
	}

	mempool->count = count;
	mempool->freelist = head;
	page->n_free = count;

	return mempool_alloc_nolock(mempool);
}

void *mempool_alloc(struct mempool *mempool) {
	acquire(&mempool->lock);
	void *ret = mempool_alloc_nolock(mempool);
	release(&mempool->lock);

	return ret;
}