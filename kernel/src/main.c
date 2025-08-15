#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <goofy-os/fbcon.h>
#include <goofy-os/hcf.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/vmalloc.h>
#include <limine.h>
#include <stddef.h>
#include <stdint.h>

/*
 * 1. Initialize early console
 * 2. Initialize CPU state with IDT, GDT, etc.
 * 3. Initialize memory manager
 */

void kmain() {
	pushcli();
	limine_init();
	console_init(__limine_framebuffer);

	init_gdt();
	init_idt();
	mm_init();
	cpu_init();

	printk("Welcome to GoofyOS\n");

	while (true)
		;
	hcf();
}
