#include <goofy-os/cpu.h>
#include <goofy-os/spinlock.h>
#include <stddef.h>

size_t cli_count;

// TODO: SMP Support (use current->cpu)
void pushcli() {
	struct cpu *curr_cpu = &cpu_cores[current_cpuid()];
	curr_cpu->cli_count++;
	__asm__ __volatile__("cli");
};

void popcli() {
	struct cpu *curr_cpu = &cpu_cores[current_cpuid()];
	curr_cpu->cli_count--;
	if (!cli_count) {
		__asm__ __volatile__("sti");
	}
}

void acquire(struct spinlock *lock) {
	pushcli();
	raw_acquire(&lock->locked);
}

void release(struct spinlock *lock) {
	lock->locked = 0;
	popcli();
}