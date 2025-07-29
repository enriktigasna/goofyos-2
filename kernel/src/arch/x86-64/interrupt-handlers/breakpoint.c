#include <goofy-os/interrupts.h>
#include <goofy-os/printk.h>
#include <stdint.h>

__attribute__((interrupt)) void
breakpoint_interrupt(struct interrupt_frame *frame) {
	printk("Hello from int3\n");
	printk("Called from %p\n", frame->ip);
	return;
}
