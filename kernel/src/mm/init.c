#include <goofy-os/boot.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <limine.h>
#include <stdint.h>

struct mm_memmap_section phys_sections[MM_MAX_MEMORY_SECTIONS];

static void __setup_memmap_sections() {
	int idx = 0;
	for (int i = 0; i < __limine_memmap_response->entry_count; i++) {
		struct limine_memmap_entry *entry =
		    __limine_memmap_response->entries[idx++];
		if (entry->type == LIMINE_MEMMAP_USABLE ||
		    entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
			printk("Adding %p of length %p\n", entry->base,
			       entry->length);
			phys_sections[i].base = entry->base;
			phys_sections[i].size = entry->length;
		}
	}
}

void mm_init() {
	__setup_memmap_sections();
	//
}