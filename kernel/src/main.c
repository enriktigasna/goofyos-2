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
	limine_init();
	console_init(__limine_framebuffer);
	cpu_init();
	mm_init();

	printk("Welcome to GoofyOS\n");
	printk("[*] Enabled interrupts\n");

	while (true) {
	}

	hcf();
}
