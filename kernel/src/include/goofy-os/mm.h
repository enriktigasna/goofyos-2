#include <stdbool.h>
#include <stdint.h>

#define MM_MAX_MEMORY_SECTIONS 0x20

struct mm_memmap_section {
	uint64_t base;
	uint64_t size;
};

extern struct mm_memmap_section phys_sections[MM_MAX_MEMORY_SECTIONS];
extern bool pagealloc_initialized;

void mm_init();
void pagealloc_init();