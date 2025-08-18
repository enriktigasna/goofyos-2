#include <goofy-os/cpu.h>
#include <goofy-os/hcf.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <stdint.h>
#include <string.h>

uint64_t get_index(int level, void *addr) {
	return (uint64_t)addr >> (12 + 9 * level) & 0x1ff;
}

struct page_table *new_page_table() {
	uint64_t *pg = zpgalloc();
	memcpy(pg, kernel_virtual_pt->cr3, 0x1000);

	struct page_table *pt = kmalloc(sizeof(struct page_table));
	pt->cr3 = pg;

	return pt;
}

void map_page(struct page_table *pt, uint64_t phys, void *virt,
	      uint64_t flags) {
	acquire(&pt->lock);
	uint64_t *cr3 = pt->cr3;

	if (flags & ~PG_FLAGMASK) {
		printk("Flagmask is %p you gave it %p\n", PG_FLAGMASK, flags);
		printk("%p\n", flags & ~PG_FLAGMASK);
		hcf(); // You fucked up
	}

	int level = 3;

	volatile uint64_t *table = cr3 + get_index(level, virt);

	while (level) {
		uint64_t value = *table;
		if (*table & PG_PRESENT) {
			value &= 0x7ffffffffffff000;
		} else {
			value = (uint64_t)zpgalloc() - hhdm_offset;
			*table = value | PG_PRESENT | PG_WRITE;
		}

		level--;
		table = (uint64_t *)__va(value) + get_index(level, virt);
	}

	*table = (uint64_t)phys | flags | PG_PRESENT;

	vm_invalidate(virt);
	release(&pt->lock);
}

void unmap_page(struct page_table *pt, void *virt) {
	acquire(&pt->lock);
	uint64_t *cr3 = pt->cr3;

	int level = 3;

	volatile uint64_t *table = cr3 + get_index(level, virt);

	while (level) {
		uint64_t value = *table;
		if (*table & PG_PRESENT) {
			value &= 0x7ffffffffffff000;
		} else {
			return;
		}

		level--;
		table = (uint64_t *)__va(value) + get_index(level, virt);
	}

	*table = 0;

	vm_invalidate(virt);
	release(&pt->lock);
}

bool is_mapped(struct page_table *pt, void *virt) {
	acquire(&pt->lock);

	uint64_t *cr3 = pt->cr3;
	int level = 3;
	volatile uint64_t *table = cr3 + get_index(level, virt);
	while (level) {
		uint64_t value = *table;
		if (*table & PG_PS) {
			release(&pt->lock);
			return true;
		}

		if (*table & PG_PRESENT) {
			value &= 0x7ffffffffffff000;
		} else {
			release(&pt->lock);
			return false;
		}

		level--;
		table = (uint64_t *)__va(value) + get_index(level, virt);
		printk("Level %d %p\n", level, table);
	}

	if (*table & PG_PRESENT) {
		release(&pt->lock);
		return true;
	} else {
		release(&pt->lock);
		return false;
	}
}

uint64_t __virt_offset(int level, void *virt) {
	uint64_t page_size = 0x1000;
	for (int i = 0; i < level; i++) {
		page_size *= 0x1000;
	}

	return (uint64_t)virt & (page_size - 1);
}

uint64_t virt_to_phys(void *virt) {
	uint64_t *pt = (uint64_t *)__va(__readcr3());

	int level = 3;

	volatile uint64_t *table = pt + get_index(level, virt);

	while (level) {
		uint64_t value = *table;
		if (*table & PG_PS) {
			return (*table & 0x7ffffffffffff000) +
			       __virt_offset(level, virt);
		}

		if (*table & PG_PRESENT) {
			value &= 0x7ffffffffffff000;
		} else {
			return 0;
		}

		level--;
		table = (uint64_t *)__va(value) + get_index(level, virt);
	}

	return (*table & 0x7ffffffffffff000) + __virt_offset(level, virt);
}

struct page_table *kernel_virtual_pt;

// All contexts can after this copy from kernel_virtual_pt and will be up to
// date with higher half mappings
void kernel_top_pgt_init() {
	kernel_virtual_pt = zpgalloc();
	uint64_t *cr3 = (void *)(__readcr3() + hhdm_offset);
	for (int i = 0x100; i < 0x200; i++) {
		if (cr3[i]) {
			continue;
		}

		cr3[i] = ((uint64_t)zpgalloc() - hhdm_offset) | PG_PRESENT |
			 PG_WRITE;
	}

	kernel_virtual_pt->cr3 = pgalloc();
	memcpy(kernel_virtual_pt->cr3, cr3, 0x1000);

	printk("Virtual pt %p\n", kernel_virtual_pt);
}