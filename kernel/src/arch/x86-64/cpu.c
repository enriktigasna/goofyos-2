#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <goofy-os/printk.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t n_cpus;
struct cpu cpu_cores[MAX_CPUS];

void new_cpu_wait() {
	printk("Cpu %d woke up\n", current_cpuid());
	set_idt(&idt_register);
	set_gdt(&gdt_register);

	x2apic_init_timer();
	tss_percpu_init();
	popcli();
	while (true)
		;
}