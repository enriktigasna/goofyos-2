#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <stddef.h>
#include <stdint.h>

size_t n_cpus;
struct cpu cpu_cores[MAX_CPUS];
