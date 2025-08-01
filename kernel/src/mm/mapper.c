#include <goofy-os/cpu.h>
#include <goofy-os/hcf.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <stdint.h>

/* shout out Dbstream osdev page */
/*
 *  https://wiki.osdev.org/User:Dbstream/Paging_on_x86
 *
 */

uint64_t get_index(int level, void *addr) {
	return (uint64_t)addr >> (12 + 9 * level) & 0x1ff;
}

// TODO:
// Physical memory should be represented as uint64_t not void*
void __early_map_page(uint64_t *pt, void *phys, void *virt, uint64_t flags) {
	if (flags & ~PG_FLAGMASK) {
		printk("Flagmask is %p you gave it %p\n", PG_FLAGMASK, flags);
		printk("%p\n", flags & ~PG_FLAGMASK);
		hcf(); // You fucked up
	}

	int level = 3;

	volatile uint64_t *table = pt + get_index(level, virt);

	while (level) {
		printk("Table %p\n", table);
		uint64_t value = *table;
		if (*table & PG_PRESENT) {
			value &= 0x7ffffffffffff000;
		} else {
			value = (uint64_t)__early_zgetpage() - hhdm_offset;
			*table = value | flags | PG_PRESENT;
		}

		level--;
		table = (uint64_t *)__va(value) + get_index(level, virt);
	}

	*table = (uint64_t)phys | flags | PG_PRESENT;

	vm_invalidate(virt);
}