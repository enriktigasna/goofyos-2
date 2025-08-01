#include <goofy-os/mm.h>
#include <stdint.h>

#define PG_PRESENT 0x1ULL
#define PG_WRITE 0x2ULL
#define PG_USER 0x4ULL
#define PG_NX 0x8000000000000000ULL
#define PG_FLAGMASK PG_WRITE | PG_NX | PG_USER

/* shout out Dbstream osdev page */
/*
 *  https://wiki.osdev.org/User:Dbstream/Paging_on_x86
 *
 * +63---------+57-----+47-----+38-----+29-----+20-----+11----------------+
 * | canonical | PML5e | PML4e | PML3e | PML2e | PML1e | offset into page |
 * +---------58+-----48+-----39+-----30+-----21+-----12+-----------------0+
 *
 * Indeces into page table
 */

uint64_t get_index(int level, void *addr) {
	return (uint64_t)addr >> (12 + 9 * level) & 0x1ff;
}

// TODO:
// Physical memory should be represented as uint64_t not void*
void __early_map_page(uint64_t *pt, void *phys, void *virt, uint64_t flags) {
	if (flags & ~PG_FLAGMASK) {
		hcf(); // You fucked up
	}

	int level = 4;

	volatile uint64_t *table = &pt[get_index(level, virt)];

	while (level) {
		uint64_t phys = *table;
		if (*table & PG_PRESENT) {
			phys &= 0x7ffffffffffff000;
		} else {
			phys = (uint64_t)__early_zgetpage() - hhdm_offset;
			*table = phys | flags;
		}

		level--;
		table =
		    &((uint64_t *)(phys + hhdm_offset))[get_index(level, virt)];
	}

	*table = (uint64_t)phys | flags;
}