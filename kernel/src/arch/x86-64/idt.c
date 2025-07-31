#include <goofy-os/cpu.h>
#include <stdint.h>
#include <string.h>

extern void *isr_vector_0_handler;

struct idtr idt_register;
__attribute__((aligned(0x10))) struct idt_entry idt_table[IDT_MAX_DESCRIPTORS];

void __idt_set_entry(uint8_t idx, uint64_t handler, uint8_t flags) {
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
		__idt_set_entry(i, (uint64_t)(&isr_vector_0_handler) + i * 16,
				0x8E);
	}
}
