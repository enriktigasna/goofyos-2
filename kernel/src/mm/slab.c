#include <goofy-os/buddy.h>
#include <goofy-os/hcf.h>
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

struct spinlock slab_lock;

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

// 1. Allocate page of cache order
// 2. Allocate new slab, and associate pages
// 3. If you are allocating from kmem_cache then get from there

// Have empty partial and full, have one short function for alloc_from_slab
// void* kmem
void *kmem_cache_alloc_nolock(struct kmem_cache *cache) { return NULL; }
void *kmem_cache_alloc() {}

void *kmalloc_nolock(size_t size) {
	if (size > kmalloc_sizes[N_ORDERS - 1])
		return NULL;
	struct kmem_cache *cache = &kmalloc_caches[kmalloc_idx(size)];
	return kmem_cache_alloc_nolock(cache);
}

void *kmalloc(size_t size) {
	acquire(&slab_lock);
	kmalloc_nolock(size);
	release(&slab_lock);
}

void slab_init() {
	mempool_init(&slab_mempool, sizeof(struct slab), 1);

	// slab_jar_bootstrap();
	/*
	for (int i = 0; i < SLAB_COUNT; i++) {
		struct kmem_cache *curr = &kmalloc_caches[i];
		curr->size = kmalloc_sizes[i];
		curr->order = kmalloc_orders[i];
		sprintf((char *)&curr->name, "kmalloc-%d", kmalloc_sizes[i]);
	}
	*/
}