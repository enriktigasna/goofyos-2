#include <goofy-os/cpu.h>
#include <stdint.h>
#include <string.h>

extern void *isr_vector_0_handler;

struct idtr idt_register;
__attribute__((aligned(0x10))) struct idt_entry idt_table[IDT_MAX_DESCRIPTORS];

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
	struct idt_entry *desc = &idt_table[vector];

	desc->isr_low = (uint64_t)isr & 0xFFFF;
	desc->kernel_cs = GDT_OFFSET_KERNEL_CODE;
	desc->ist = 0;
	desc->attributes = flags;
	desc->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
	desc->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
	desc->reserved = 0;
}

void idt_set_entry(uint8_t idx, uint64_t handler, uint8_t flags) {
	struct idt_entry *entry = &idt_table[idx];

	entry->isr_low = handler & 0xFFFF;
	entry->kernel_cs = GDT_OFFSET_KERNEL_CODE;
	entry->ist = 0;
	entry->attributes = flags;
	entry->isr_mid = (handler >> 16) & 0xFFFF;
	entry->isr_high = (handler >> 32) & 0xFFFFFFFF;
	entry->reserved = 0;
}

void init_idt() {
	idt_register.limit = sizeof(idt_table) - 1;
	idt_register.base = (uint64_t)&idt_table;

	set_idt(&idt_register);
	for (int i = 0; i < 256; i++) {
		idt_set_entry(i, (uint64_t)(&isr_vector_0_handler) + i * 16,
			      0x8E);
	}
}
