#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <goofy-os/fbcon.h>
#include <goofy-os/hcf.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
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
	cpu_init();
	mm_init();

	printk("Welcome to GoofyOS\n");

	uint64_t cr3 = __readcr3();
	printk("cr3 %p", cr3);

	map_page((void *)__va(cr3), 0x1337000, (void *)0xffffc00000000000,
		 PG_WRITE);

	while (true) {
	}

	hcf();
}
