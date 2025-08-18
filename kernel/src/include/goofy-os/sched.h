#pragma once
#include <goofy-os/interrupts.h>
#include <goofy-os/mm.h>
#include <goofy-os/spinlock.h>

extern struct task idle_task;

void sched_init();

void switch_context();
void schedule();

struct task {
	struct page_table *pt;
	struct registers *regs;

	struct task *next;
	struct task *prev;
};

struct scheduler {
	struct task *head;
	struct task *tail;
	struct spinlock lock;

	uint64_t preempt_count;
};

void go_to_task(struct task *task);
void return_to_ctx(struct registers *regs, uint64_t cr3);

void preempt_enable();
void preempt_disable();