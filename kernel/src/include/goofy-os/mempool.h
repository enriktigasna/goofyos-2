#pragma once
#include <goofy-os/spinlock.h>

struct list_head;

struct mempool {
	struct list_head *freelist;
	unsigned int count; // Amount objects
	unsigned int size;  // Size of each object
	unsigned int order; // Order of pages
	struct spinlock lock;
};

void mempool_init(struct mempool *mempool, unsigned int size,
		  unsigned int order);
void *mempool_alloc(struct mempool *mempool);
void *mempool_zalloc(struct mempool *mempool);
void mempool_free(struct mempool *mempool, void *chunk);