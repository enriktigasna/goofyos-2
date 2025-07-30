#include <goofy-os/cpu.h>
#include <goofy-os/printk.h>

void isr_generic_handler(struct interrupt_context *ctx) {
	printk("Handling interrupt [%x]\n", ctx->vector_number);
	return;
}