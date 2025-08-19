#include <goofy-os/cpu.h>
#include <goofy-os/fbcon.h>
#include <goofy-os/hcf.h>
#include <goofy-os/printk.h>
#include <goofy-os/scancode.h>
#include <goofy-os/sched.h>

void timer_handler(struct registers *ctx) {
	// printk("%c", '0' + current_cpuid());
	x2apic_eoi();
	if (!cpu_cores[current_cpuid()].preempt_count)
		schedule(ctx);
}

void keyboard_handler(struct registers *ctx) {
	uint8_t scancode = inb(0x60);
	if (scancode & 0x80) {
		// pic_eoi(ctx->vector_number - 0x20);
		return;
	}

	uint8_t ascii_translation = kbd_US[scancode];

	// Release
	if (ascii_translation) {
		printk("%c", ascii_translation);
	}
	// pic_eoi(ctx->vector_number - 0x20);
	return;
}

void dump_regs(struct registers *ctx) {
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
	printk("rsp: %p\n", ctx->rsp);
	printk("rbp: %p\n", ctx->rbp);

	printk("rip: %p\n", ctx->rip);
	return;
}

void force_unlock_console() { fbcon.lock.locked = 0; }

void isr_generic_handler(struct registers *ctx) {
	__asm__ __volatile__("cli");
	switch (ctx->vector_number) {
	case 0x3:
		break;
	case 0x20:
		// printk("%c", '0' + current_cpuid());
		timer_handler(ctx);
		break;
	case 0x21:
		// keyboard_handler(ctx);
		break;
	default:
		force_unlock_console();
		printk("Unrecognized interrupt! v=%x c=%d\n",
		       ctx->vector_number, current_cpuid());
		dump_regs(ctx);
		hcf();
	}
	return;
}