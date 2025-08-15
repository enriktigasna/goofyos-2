#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <stddef.h>
#include <stdint.h>

size_t n_cpus;
struct cpu cpu_cores[MAX_CPUS];

// size_t cpu_count;
// void x2apic_init() { uint64_t num_cpus = __limine_smp_response; }