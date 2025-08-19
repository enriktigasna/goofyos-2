#include <goofy-os/interrupts.h>
#include <goofy-os/printk.h>
#include <goofy-os/sched.h>
#include <goofy-os/slab.h>
#include <goofy-os/time.h>
#include <goofy-os/vmalloc.h>

void do_ktimer(struct ktimer *ktimer) {
	while (1) {
		hpet_wait_us_yield(ktimer->us_delay);
		ktimer->func(ktimer->arg);
	}
}

struct task *init_ktimer(struct ktimer *ktimer) {
	struct task *ktimer_task = kzalloc(sizeof(struct task));
	ktimer_task->regs = kzalloc(sizeof(struct registers));
	ktimer_task->pt = new_page_table();
	ktimer_task->regs->rip = (uint64_t)do_ktimer;
	ktimer_task->regs->rdi = (uint64_t)ktimer;
	ktimer_task->regs->rsp =
	    (uint64_t)vmalloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE - 0x10;
	ktimer_task->regs->cs = KERNEL_CS;

	return ktimer_task;
}