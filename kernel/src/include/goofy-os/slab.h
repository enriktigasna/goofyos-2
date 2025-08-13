#pragma once
#include <stddef.h>
#include <stdint.h>

#define SLAB_COUNT 9
#define SLAB_MAX_EMPTY 2
#define SLAB_PREALLOC_PAGES 2
#define kmalloc_size(sz) (kmalloc_sizes[kmalloc_idx(sz)])
#define PAGE_SIZE 4096

#define SLAB_STATIC (1 << 0)
#define SLAB_FULL (1 << 1)
#define SLAB_PARTIAL (1 << 2)
#define SLAB_EMPTY (1 << 3)

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

	int n_full;
	int n_partial;
	int n_empty;

	char name[32];
};

struct slab {
	struct kmem_cache *slab_jar;

	struct slab *next;
	struct slab *prev;

	void *freelist;
	int free_objects;
	int flags;
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
void kfree(void *object);