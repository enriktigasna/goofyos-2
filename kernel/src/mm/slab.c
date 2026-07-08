#include <goofy-os/buddy.h>
#include <goofy-os/hcf.h>
#include <goofy-os/mem_macros.h>
#include <goofy-os/mempool.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/spinlock.h>
#include <string.h>

// #define PRINTK_SLAB

// I genuinely gotta rewrite this

#define SLAB_SIZE(cache) (PAGE_SIZE << (cache)->order)
#define N_OBJECTS(cache) (SLAB_SIZE(cache) / (cache)->size)

const size_t kmalloc_sizes[SLAB_COUNT] = {8,   16,  32,	  64,	128,  192,
					  256, 512, 1024, 2048, 4096, 8192};
const int kmalloc_orders[SLAB_COUNT] = {0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3};
struct kmem_cache kmalloc_caches[SLAB_COUNT];

struct mempool slab_mempool;

// Initialize the kmalloc_cache directly,
// you will need it to init other caches

// Turn a page into a freelist of size size
// *page becomes the head
void *fill_page_freelist(struct kmem_cache *cache, void *page) {
	void *next = NULL;
	for (int i = 0; i < (N_OBJECTS(cache)); i++) {
		*(void **)(page + i * cache->size) = next;
		next = (void *)(page + i * cache->size);
	}
	return next;
}

void kmem_insert_slab(struct slab *slab) {
	struct kmem_cache *cache = slab->cache;
	// LAST SLAB TODO: Make it free the empty return slabs, if n_empty over
	// SLAB_MAX_EMPTY
	printk("%d %d\n", slab->free_objects, N_OBJECTS(cache));
	if (N_OBJECTS(cache) == slab->free_objects) {
		cache->n_empty++;
		list_push_front(&cache->slab_empty, &slab->slab_list);
		return;
	}

	if (slab->free_objects == 0) {
		cache->n_full++;
		list_push_front(&cache->slab_full, &slab->slab_list);
		return;
	}

	printk("inserting to partial\n");
	cache->n_partial++;
	list_push_front(&cache->slab_partial, &slab->slab_list);
}

void *kmem_get_chunk_from_slab(struct slab *slab) {
	slab->free_objects--;
	return slist_pop_front(&slab->freelist);
}

void *kmem_cache_alloc_partial(struct kmem_cache *cache) {
	// Grab a slab from partial
	printk("Trying to alloc from partial\n");
	struct slab *slab = container_of(list_pop_front(&cache->slab_partial),
					 struct slab, slab_list);
	cache->n_partial--;

	// Put it back in the right list
	void *chunk = kmem_get_chunk_from_slab(slab);
	kmem_insert_slab(slab);
	return chunk;
}

void *kmem_cache_alloc_empty(struct kmem_cache *cache) {
	printk("Trying to alloc from empty\n");
	// Grab a slab from empty
	struct slab *slab = container_of(list_pop_front(&cache->slab_empty),
					 struct slab, slab_list);
	cache->n_empty--;

	// Put it back in the right list (most likely partial)
	void *chunk = kmem_get_chunk_from_slab(slab);
	kmem_insert_slab(slab);
	return chunk;
}

int kmem_cache_new_empty(struct kmem_cache *cache) {
	struct page *new_page = alloc_pages(cache->order, 0);
	struct slab *new_slab = mempool_zalloc(&slab_mempool);
	void *virt_page;
	if (new_page == NULL || new_slab == NULL)
		goto err;

	new_slab->cache = cache;

	// Associate all pages with the slab
	for (int i = 0; i < (1 << cache->order); i++) {
		new_page[i].slab = new_slab;
	}

	// Build the freelist in the page
	struct single_list_head *cur;
	virt_page = page_to_virt(new_page);
	for (int i = 0; i < N_OBJECTS(cache); i++) {
		cur = (void *)virt_page + (cache->size * i);
		slist_push_front(&new_slab->freelist, cur);
	}
	new_slab->free_objects = N_OBJECTS(cache);

	// Push it to empty
	kmem_insert_slab(new_slab);
	return 0;

err:
	free_pages(new_page, cache->order);
	mempool_free(&slab_mempool, new_slab);
}

void *kmem_cache_alloc_nolock(struct kmem_cache *cache) {
	if (cache->n_partial)
		return kmem_cache_alloc_partial(cache);
	if (cache->n_empty)
		return kmem_cache_alloc_empty(cache);
	if (kmem_cache_new_empty(cache))
		return NULL;

	return kmem_cache_alloc_empty(cache);
}

void *kmem_cache_alloc(struct kmem_cache *cache) {
	acquire(&cache->cache_lock);
	void *ret = kmem_cache_alloc_nolock(cache);
	release(&cache->cache_lock);
	return ret;
}

void *kmem_cache_free_nolock(struct kmem_cache *cache, void *chunk) {
	struct page *page = __hhdm_to_page(chunk);
	struct slab *slab = page->slab;
	struct page *base_page = base_page(page, cache->order);

	// Slab is in full
	if (slab->free_objects == 0) {
		list_remove_node(&cache->slab_full, &slab->slab_list);
		cache->n_full--;
	}

	// Slab is in partial
	if (slab->free_objects > 0) {
		list_remove_node(&cache->slab_partial, &slab->slab_list);
		cache->n_partial--;
	}

	slist_push_front(&slab->freelist, chunk);
	slab->free_objects++;

	kmem_insert_slab(slab);
}

void *kmem_cache_free(struct kmem_cache *cache, void *chunk) {
	acquire(&cache->cache_lock);
	kmem_cache_free_nolock(cache, chunk);
	release(&cache->cache_lock);
}

void *kmalloc(size_t size) {
	if (size > kmalloc_sizes[SLAB_COUNT - 1])
		return NULL;

	struct kmem_cache *cache = &kmalloc_caches[kmalloc_idx(size)];
	return kmem_cache_alloc(cache);
}

void kfree(void *chunk) {
	struct page *page = __hhdm_to_page(chunk);
	struct slab *slab = page->slab;
	struct kmem_cache *cache = slab->cache;
	struct page *base_page = base_page(page, cache->order);
	kmem_cache_free(cache, chunk);
}

void slab_init() {
	mempool_init(&slab_mempool, sizeof(struct slab), 1);
	for (int i = 0; i < SLAB_COUNT; i++) {
		struct kmem_cache *curr = &kmalloc_caches[i];
		curr->size = kmalloc_sizes[i];
		curr->order = kmalloc_orders[i];
		sprintf(curr->name, "kmalloc-%d", kmalloc_sizes[i]);
	}
	void *samples[0x10];
	for (int i = 0; i < 0x10; i++) {
		samples[i] = kmalloc(8192);
		printk("alloc %p\n", samples[i]);
	}
	for (int i = 0; i < 0x10; i++) {
		kfree(samples[i]);
		printk("free %p\n", samples[i]);
	}
	for (int i = 0; i < 0x10; i++) {
		samples[i] = kmalloc(8192);
		printk("alloc %p\n", samples[i]);
	}
}