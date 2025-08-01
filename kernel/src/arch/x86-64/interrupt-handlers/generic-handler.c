#include <goofy-os/cpu.h>
#include <goofy-os/hcf.h>
#include <goofy-os/printk.h>
#include <goofy-os/scancode.h>

void timer_handler(struct interrupt_context *ctx) {
	// printk(".");
	pic_eoi(ctx->vector_number - 0x20);
	return;
}

void keyboard_handler(struct interrupt_context *ctx) {
	uint8_t scancode = inb(0x60);
	if (scancode & 0x80) {
		pic_eoi(ctx->vector_number - 0x20);
		return;
	}

	uint8_t ascii_translation = kbd_US[scancode];

	// Release
	if (ascii_translation) {
		printk("%c", ascii_translation);
	}
	pic_eoi(ctx->vector_number - 0x20);
	return;
}

void dump_regs(struct interrupt_context *ctx) {
	printk("rax: %p\n", ctx->rax);
	printk("rbx: %p\n", ctx->rbx);
	printk("rcx: %p\n", ctx->rcx);
	printk("rdx: %p\n", ctx->rdx);
	printk("rdi: %p\n", ctx->rdi);
	printk("rsi: %p\n", ctx->rsi);
	printk("r8: %p\n", ctx->r8);
	printk("r9: %p\n", ctx->r9);
	printk("r10: %p\n", ctx->r10);
	printk("r11: %p\n", ctx->r11);
	printk("r12: %p\n", ctx->r12);
	printk("r13: %p\n", ctx->r13);
	printk("r14: %p\n", ctx->r14);
	printk("r15: %p\n", ctx->r15);

	printk("rip: %p\n", ctx->iret_rip);
	return;
}

void isr_generic_handler(struct interrupt_context *ctx) {
	switch (ctx->vector_number) {
	case 0x3:
		break;
	case 0x20:
		timer_handler(ctx);
		break;
	case 0x21:
		keyboard_handler(ctx);
		break;
	default:
		printk("Unrecognized interrupt! v=%x\n", ctx->vector_number);
		dump_regs(ctx);
		hcf();
	}
	return;
}