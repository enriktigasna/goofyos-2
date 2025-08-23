#include <goofy-os/boot.h>
#include <goofy-os/cmdline.h>
#include <goofy-os/cpu.h>
#include <goofy-os/fbcon.h>
#include <goofy-os/hcf.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/time.h>
#include <goofy-os/vmalloc.h>
#include <limine.h>
#include <stddef.h>
#include <stdint.h>

/*
 * 1. Initialize early console
 * 2. Initialize CPU state with IDT, GDT, etc.
 * 3. Initialize memory manager
 */

void ktimer_task(uint64_t id) { printk("Hello from timer thread %d!\n", id); }

void schedule_bsp() {
	cpu_cores[0].cli_count = 0;
	go_to_task(&idle_task);
}

void kmain() {
	limine_init();
	serial_init();
	mm_init();
	console_init(__limine_framebuffer);
	parse_cmdline();
	init_gdt();
	init_idt();
	cpu_init();
	x2apic_init_timer();

	printk("Welcome to GoofyOS\n");
	if (cmdline_contains("initrd")) {
		// Do initrd mount
	}

	schedule_bsp();
}
