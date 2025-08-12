#pragma once
#include <stddef.h>
#include <stdint.h>

#define SLAB_COUNT 9
#define SLAB_PREALLOC_PAGES 2
#define kmalloc_size(sz) (8 << kmalloc_idx(sz))
#define PAGE_SIZE 4096

/*
 * Slab allocator behavior:
 * Three lists: slab_full, slab_partial, slab_empty, will first look in partial,
 * then empty, then allocate a new slab and retry for empty
 *
 * 1. Look in partial cache
 * 2. Look in empty cache
 * 3. Allocate new empty slab for your cache, and retry for slab_empty
 */

struct kmem_cache {
	size_t size;

	struct slab *slab_full;
	struct slab *slab_partial;
	struct slab *slab_empty;
};

struct slab {
	struct kmem_cache *slab_cache;

	struct slab *next;
	struct slab *prev;

	void *freelist;
	int free_objects;
};

// clang-format off
static inline __attribute__((always_inline))
size_t kmalloc_idx(size_t size) {
	if (size <=    8) return 0;
	if (size <=   16) return 1;
	if (size <=   32) return 2;
	if (size <=   64) return 3;
	if (size <=  128) return 4;
	if (size <=  256) return 5;
	if (size <=  512) return 6;
	if (size <= 1024) return 7;
	if (size <= 2048) return 8;
	return size; // You fucked up
}
// clang-format on
void slab_init();
void *kmalloc(size_t size);