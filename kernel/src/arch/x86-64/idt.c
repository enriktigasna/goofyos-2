#include <goofy-os/cpu.h>
#include <string.h>
#include <stdint.h>

struct idtr idt_register;
__attribute__((aligned(0x10))) struct idt_entry idt_table[IDT_MAX_DESCRIPTORS];

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags)
{
        struct idt_entry *desc = &idt_table[vector];

        desc->isr_low        = (uint64_t)isr & 0xFFFF;
        desc->kernel_cs      = GDT_OFFSET_KERNEL_CODE;
        desc->ist            = 0;
        desc->attributes     = flags;
        desc->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
        desc->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
        desc->reserved       = 0;
}

void init_idt()
{
        memset(idt_table, 0, sizeof(idt_table));
        idt_register.limit = sizeof(idt_table) - 1;
        idt_register.base = (uint64_t)&idt_table;

        set_idt(&idt_register);
}
