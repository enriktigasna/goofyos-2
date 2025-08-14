// Needed functions:
#include <goofy-os/mm.h>
#include <stdint.h>

#define VMALLOC_START 0xffffd00000000000
#define VMALLOC_END 0xffffe00000000000

#define VM_INUSE 1

#define PAGE_COUNT(sz) (sz + (PAGE_SIZE - 1)) / PAGE_SIZE

void *vmalloc(unsigned long size);
void *vmalloc_flags(unsigned long size, uint64_t flags);
void *vzalloc(unsigned long size);
void vfree(void *range);

void vmalloc_init();

struct vmalloc_struct {
	uint64_t flags;
	size_t size;
	void *addr;
	struct vmalloc_struct *next;
};