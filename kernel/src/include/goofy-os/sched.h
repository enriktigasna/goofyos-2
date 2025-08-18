#include <goofy-os/interrupts.h>
#include <goofy-os/mm.h>

void switch_context();
void schedule();

struct task {
	struct page_table *pt;
	struct registers *regs;
};

struct scheduler {
	void *tail;
};