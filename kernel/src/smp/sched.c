#include <goofy-os/interrupts.h>
#include <goofy-os/slab.h>

void switch_context() {}

// void isr_context

/**
 * Take current thread and saves it's task, pop a task from the scheduler and
 * Do that
 */
void schedule() {}

void sched_idle() {
	while (1) {
		__asm__ __volatile__("hlt");
	}
}

/**
 *  Initializes the scheduler, adds an idle task to be jumped to incase
 */
void sched_init() {
	struct registers *idle_ctx = kzalloc(sizeof(struct registers));
	idle_ctx->rip = (uint64_t)&sched_idle;
}