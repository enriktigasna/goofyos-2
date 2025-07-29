#include <goofy-os/hcf.h>
#include <goofy-os/interrupts.h>
#include <goofy-os/printk.h>
#include <stdint.h>

__attribute__((interrupt)) void
pagefault_handler(struct interrupt_frame *frame) {
	printk("PAGE FAULT\n");
	hcf();
	return;
}