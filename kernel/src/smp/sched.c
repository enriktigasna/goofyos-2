#include "string.h"
#include <goofy-os/interrupts.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/sched.h>
#include <goofy-os/slab.h>
#include <goofy-os/time.h>

struct task idle_task;
struct scheduler scheduler;

void preempt_enable() {
	if (cpu_cores[current_cpuid()].preempt_count)
		cpu_cores[current_cpuid()].preempt_count--;
}

void preempt_disable() { cpu_cores[current_cpuid()].preempt_count++; }

/**
 * Not to be called from user context, as it won't reschedule current task
 */
void go_to_task(struct task *task) {
	cpu_cores[current_cpuid()].current_task = task;
	return_to_ctx(task->regs, (uint64_t)task->pt->cr3 - hhdm_offset);
}

void sched_task(struct task *task) {
	if (task == &idle_task) {
		return;
	}

	task->next = scheduler.head;
	scheduler.head = task;

	if (task->next)
		task->next->prev = task;
	else
		scheduler.tail = task;

	task->prev = NULL;
}

struct task *pop_task() {
	struct task *ret = scheduler.tail;
	if (!ret) {
		return &idle_task;
	}

	scheduler.tail = ret->prev;
	scheduler.tail->next = NULL;

	ret->prev = NULL;
	ret->next = NULL;
}

void save_task(struct registers *regs) {
	if (!cpu_cores[current_cpuid()].current_task) {
		return;
	}
	memcpy(cpu_cores[current_cpuid()].current_task->regs, regs,
	       sizeof(struct registers));
}

void schedule(struct registers *regs) {
	acquire(&scheduler.lock);
	save_task(regs);
	sched_task(cpu_cores[current_cpuid()].current_task);
	struct task *target = pop_task();
	release(&scheduler.lock);
	go_to_task(target);
}

void sched_idle() {
	while (1) {
		__asm__ __volatile__("sti");
		__asm__ __volatile__("hlt");
	}
}

/**
 *  Initializes the scheduler, adds an idle task to be jumped to when no tasks
 */
void sched_init() {
	idle_task.regs = kzalloc(sizeof(struct registers));
	idle_task.regs->rip = (uint64_t)&sched_idle;
	idle_task.regs->cs = KERNEL_CS;
	idle_task.pt = new_page_table();
}