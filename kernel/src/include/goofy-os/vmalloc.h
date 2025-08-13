// Needed functions:
#include <stdint.h>

void *vmalloc(unsigned long size);
void *vzalloc(unsigned long size);
void vfree(const void *addr);

void *vmap(struct page **pages, unsigned int count, unsigned long flags, uint64_t prot);
void *vunmap(const void* addr);