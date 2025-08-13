#include <goofy-os/hcf.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/spinlock.h>
#include <string.h>

const size_t kmalloc_sizes[SLAB_COUNT] = {8,   16,  32,	  64,  128,
					  256, 512, 1024, 2048};
struct kmem_cache kmalloc_caches[SLAB_COUNT];
struct spinlock slab_lock;

struct kmem_cache slab_jar;
struct slab bootstrap_slab_jar[SLAB_PREALLOC_PAGES];

// Initialize the slab_jar directly,
// you will need it to init other caches

// Turn a page into a freelist of size size
// *page becomes the head
void *fill_page_freelist(size_t size, void *page) {
	void *next = NULL;
	for (int i = 0; i < PAGE_SIZE / size; i++) {
		*(void **)(page + i * size) = next;
		next = (void *)(page + i * size);
	}

	return next;
}

void slab_jar_bootstrap() {
	slab_jar.size = kmalloc_size(sizeof(struct slab));
	sprintf((char *)&slab_jar.name, "slab_jar");

	for (int i = 0; i < SLAB_PREALLOC_PAGES; i++) {
		struct slab *tmp = &bootstrap_slab_jar[i];
		if (slab_jar.slab_empty) {
			slab_jar.slab_empty->prev = tmp;
		}

		tmp->flags |= SLAB_STATIC;
		tmp->next = slab_jar.slab_empty;
		tmp->slab_jar = &slab_jar;

		// TODO: Set struct slab in struct page
		void *page = pgalloc();
		if (!page) {
			hcf();
		}

		struct page *pgstruct = __hhdm_to_page(page);
		pgstruct->slab = tmp;

		tmp->freelist = fill_page_freelist(slab_jar.size, page);
		tmp->free_objects = PAGE_SIZE / slab_jar.size;

		slab_jar.slab_empty = tmp;
		slab_jar.n_empty++;
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

			cache->n_full++;
			cache->n_partial--;
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

		cache->n_empty--;
		cache->n_partial++;
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

	// Prevent slab_jar infinite recursion
	if (cache == &slab_jar) {
		new_slab = (struct slab *)head;
		head = *(void **)head;
		memset(new_slab, 0, sizeof(struct slab));

		new_slab->free_objects = (PAGE_SIZE / cache->size) - 2;
	} else {
		new_slab = _kmem_cache_alloc(&slab_jar);
		memset(new_slab, 0, sizeof(struct slab));

		if (!new_slab) {
			pgfree(page);
			return NULL;
		}
		new_slab->free_objects = (PAGE_SIZE / cache->size) - 1;
	}

	struct_page->slab = new_slab;
	new_slab->slab_jar = cache;

	new_slab->next = cache->slab_partial;
	if (new_slab->next)
		new_slab->next->prev = new_slab;
	cache->slab_partial = new_slab;
	cache->n_partial++;

	void *object = head;
	new_slab->freelist = *(void **)object;

	return object;
}

void *kmem_cache_alloc(struct kmem_cache *cache) {
	acquire(&slab_lock);
	void *obj = _kmem_cache_alloc(cache);
	release(&slab_lock);
	return obj;
}

static inline __attribute__((always_inline)) bool static_slab(struct slab *sl) {
	return sl->flags & SLAB_STATIC;
}

/*
 * All slabs freed are in the hhdm
 */
void _kfree(void *object) {
	struct page *pg = __hhdm_to_page(object);
	struct slab *slab = pg->slab;
	struct kmem_cache *cache = slab->slab_jar;

	// Add to slab freelist
	size_t max_objects = PAGE_SIZE / cache->size;
	*(void **)object = slab->freelist;
	slab->freelist = object;

	// If this is the last object in the slab we freed
	if (++slab->free_objects == max_objects) {
		// Pop it out from the d-linked list and put into free
		if (slab->prev) {
			slab->prev->next = slab->next;
		}

		if (slab->next) {
			slab->next->prev = slab->prev;
		}

		// Case where we destroy slab
		if (cache->slab_empty > SLAB_MAX_EMPTY && !static_slab(slab)) {
			_kfree(slab);
			pgfree((void *)((uint64_t)object & ~0xfff));
			return;
		}

		// Case where slab goes into empty
		slab->next = cache->slab_empty;
		slab->prev = NULL;

		if (cache->slab_empty) {
			cache->slab_empty->prev = slab;
		}

		cache->slab_empty = slab;
		return;
	}

	if (slab->free_objects > max_objects) {
		printk("DOUBLE FREE (more free_objects than max_objects)\n");
		hcf();
	}
}

void kfree(void *object) {
	acquire(&slab_lock);
	_kfree(object);
	release(&slab_lock);
}

void *kmalloc(size_t size) {
	if (size > 2048)
		return NULL;

	struct kmem_cache *cache = &kmalloc_caches[kmalloc_idx(size)];
	return kmem_cache_alloc(cache);
}

// 1. First check if slab cache has partials or empties. If it does, return
// kmem_cache_alloc(slab_jar)
struct slab *alloc_slab_struct() {}

void slab_init() {
	slab_jar_bootstrap();
	for (int i = 0; i < SLAB_COUNT; i++) {
		struct kmem_cache *curr = &kmalloc_caches[i];
		curr->size = kmalloc_sizes[i];
		sprintf((char *)&curr->name, "kmalloc-%d", kmalloc_sizes[i]);
	}
}