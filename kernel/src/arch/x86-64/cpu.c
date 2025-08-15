#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <goofy-os/printk.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

size_t n_cpus;
struct cpu cpu_cores[MAX_CPUS];

void new_cpu_wait() {
	popcli();
	printk("Cpu %d woke up\n", current_cpuid());
	while (true)
		;
}