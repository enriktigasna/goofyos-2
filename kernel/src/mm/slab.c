#include <goofy-os/hcf.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/spinlock.h>
#include <string.h>

size_t kmalloc_sizes[SLAB_COUNT] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048};
struct kmem_cache kmalloc_caches[SLAB_COUNT];
struct spinlock slab_lock;

struct kmem_cache slab_cache;
struct slab bootstrap_slab_cache[SLAB_PREALLOC_PAGES];

// Initialize the slab_cache directly,
// you will need it to init other caches

// Turn a page into a freelist of size size
// *page becomes the head
void *fill_page_freelist(size_t size, void *page) {
	void *next = NULL;
	for (int i = 0; i < PAGE_SIZE / slab_cache.size; i++) {
		*(void **)(page + i * size) = next;
		next = (void *)(page + i * size);
	}

	return next;
}

void slab_cache_bootstrap() {
	slab_cache.size = kmalloc_size(sizeof(struct slab));

	for (int i = 0; i < SLAB_PREALLOC_PAGES; i++) {
		struct slab *tmp = &bootstrap_slab_cache[i];
		if (slab_cache.slab_empty) {
			slab_cache.slab_empty->prev = tmp;
		}

		tmp->next = slab_cache.slab_empty;
		tmp->slab_cache = &slab_cache;

		// TODO: Set struct slab in struct page
		void *page = pgalloc();
		if (!page) {
			hcf();
		}

		tmp->freelist = fill_page_freelist(slab_cache.size, page);
		tmp->free_objects = PAGE_SIZE / slab_cache.size;

		slab_cache.slab_empty = tmp;
	}
}

void *_kmem_cache_alloc(struct kmem_cache *cache) {
	// Try to allocate from partial first
	struct slab *partial;
	if (partial = cache->slab_partial) {
		void *object = partial->freelist;
		partial->freelist = *(void **)object;
		partial->free_objects--;

		if (!partial->free_objects) {
			cache->slab_partial = partial->next;
			if (partial->next)
				partial->next->prev = NULL;
			partial->next = cache->slab_full;
			if (partial->next)
				partial->next->prev = partial;
			cache->slab_full = partial;
		}

		return object;
	}

	// Fallback to empty, put empty into partial
	struct slab *empty;
	if (empty = cache->slab_empty) {
		void *object = empty->freelist;
		empty->freelist = *(void **)object;
		empty->free_objects--;

		cache->slab_empty = empty->next;
		empty->next = cache->slab_partial;
		if (empty->next)
			empty->next->prev = empty;
		empty->prev = NULL;

		cache->slab_partial = empty;

		return object;
	}

	// Allocate new page
	void *page = pgalloc();
	if (!page) {
		return NULL;
	}

	void *head = fill_page_freelist(cache->size, page);

	struct page *struct_page = __hhdm_to_page(page);
	struct slab *new_slab;

	// Prevent slab_cache infinite recursion
	if (cache == &slab_cache) {
		new_slab = (struct slab *)head;
		memset(new_slab, 0, sizeof(struct slab));

		head = *(void **)head;
		new_slab->free_objects = (PAGE_SIZE / cache->size) - 2;
	} else {
		new_slab = _kmem_cache_alloc(&slab_cache);
		memset(new_slab, 0, sizeof(struct slab));

		if (!new_slab) {
			pgfree(page);
			return NULL;
		}
		new_slab->free_objects = (PAGE_SIZE / cache->size) - 1;
	}

	struct_page->slab = new_slab;

	new_slab->next = cache->slab_partial;
	if (new_slab->next)
		new_slab->next->prev = new_slab;
	cache->slab_partial = new_slab;

	void *object = head;
	new_slab->freelist = *(void **)head;

	return object;
}

void *kmem_cache_alloc(struct kmem_cache *cache) {
	acquire(&slab_lock);
	void *obj = _kmem_cache_alloc(cache);
	release(&slab_lock);
	return obj;
}

void *kmalloc(size_t size) {
	if (size > 2048)
		return NULL;

	struct kmem_cache *cache = &kmalloc_caches[kmalloc_idx(size)];
	printk("cache %p\n", cache);
	return kmem_cache_alloc(cache);
}

// 1. First check if slab cache has partials or empties. If it does, return
// kmem_cache_alloc(slab_cache)
struct slab *alloc_slab_struct() {}

void slab_init() {
	slab_cache_bootstrap();
	for (int i = 0; i < SLAB_COUNT; i++) {
		kmalloc_caches[i].size = kmalloc_sizes[i];
	}
}