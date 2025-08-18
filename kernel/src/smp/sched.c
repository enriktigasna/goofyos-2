void switch_context() {}

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
void sched_init() {}