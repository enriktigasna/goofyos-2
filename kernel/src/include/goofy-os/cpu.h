#pragma once
#include <goofy-os/interrupts.h>
#include <stdint.h>

#define GDT_OFFSET_KERNEL_CODE 0x08
#define GDT_OFFSET_KERNEL_DATA 0x10
#define GDT_OFFSET_USER_CODE 0x18
#define GDT_OFFSET_USER_DATA 0x20

struct gdtr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

struct idtr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));

struct idt_entry {
	uint16_t isr_low;
	uint16_t kernel_cs;
	uint8_t ist;
	uint8_t attributes;
	uint16_t isr_mid;
	uint32_t isr_high;
	uint32_t reserved;
} __attribute__((packed));

void cpu_init();
void init_gdt();
void init_idt();

void set_gdt(struct gdtr *gdt_register);
void set_idt(struct idtr *idt_register);

#define IDT_MAX_DESCRIPTORS 256

typedef void (*interrupt_handler_t)(struct interrupt_frame *frame);
extern interrupt_handler_t interrupt_table[IDT_MAX_DESCRIPTORS];

inline void outb(uint16_t port, uint8_t val) {
	__asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

inline void io_wait(void) { outb(0x80, 0); }
void pic_remap(uint8_t offset1, uint8_t offset2);