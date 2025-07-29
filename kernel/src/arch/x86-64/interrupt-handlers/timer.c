#include <goofy-os/interrupts.h>
#include <goofy-os/printk.h>
#include <stdint.h>

__attribute__((interrupt)) void timer_handler(struct interrupt_frame *frame) {
	printk(".");
	return;
}
