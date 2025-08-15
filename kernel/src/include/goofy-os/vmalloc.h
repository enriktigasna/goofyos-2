#pragma once
#include <goofy-os/mm.h>
#include <stddef.h>
#include <stdint.h>

#define VMALLOC_START 0xffffd00000000000
#define VMALLOC_END 0xffffe00000000000

#define VM_INUSE 1

#define PAGE_COUNT(sz) (sz + (PAGE_SIZE - 1)) / PAGE_SIZE

void *vmalloc(size_t size);
void *vmalloc_flags(size_t size, uint64_t flags);
void *vzalloc(size_t size);
void vfree(void *range);

void *vmap_contiguous(uint64_t phys_addr, size_t size);
void vunmap_contiguous(void *range);

void vmalloc_init();

struct vmalloc_struct {
	uint64_t flags;
	size_t size;
	void *addr;
	struct vmalloc_struct *next;
};