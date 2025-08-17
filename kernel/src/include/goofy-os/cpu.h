#pragma once
#include <goofy-os/interrupts.h>
#include <stddef.h>
#include <stdint.h>

#define GDT_OFFSET_KERNEL_CODE 0x08
#define GDT_OFFSET_KERNEL_DATA 0x10
#define GDT_OFFSET_USER_CODE 0x18
#define GDT_OFFSET_USER_DATA 0x20

#define MAX_CPUS 0x10
#define MAX_IOAPICS 8
#define KERNEL_STACK_SIZE 0x10000

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

inline void rdmsr(uint32_t msr, uint64_t *val) {
	uint32_t *lo = (uint32_t *)val;
	uint32_t *hi = (uint32_t *)((uint64_t)val + 4);
	__asm__ volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

inline void wrmsr(uint32_t msr, uint64_t val) {
	uint32_t lo = val & 0xffffffff;
	uint32_t hi = val >> 32;
	__asm__ volatile("wrmsr" ::"a"(lo), "d"(hi), "c"(msr));
}

struct cpu {
	size_t lapic_id;
	size_t cli_count;
	uint64_t hz;
};

struct ioapic {
	void *base;
};

extern size_t n_cpus;
extern struct cpu cpu_cores[MAX_CPUS];
extern struct ioapic ioapics[MAX_IOAPICS];

inline void io_wait(void) { outb(0x80, 0); }
void pic_disable();
uint64_t __readcr3();
void vm_invalidate(void *entry);

void pushcli();
void popcli();

void ioapic_write(struct ioapic *ioapic, uint8_t offset, uint32_t val);
uint32_t ioapic_read(struct ioapic *ioapic, uint8_t offset, uint32_t val);

extern void **kernel_stacks;
void cpu_wakeup();

uint64_t current_cpuid();
void new_cpu_wait();
void x2apic_init_timer();

void pit_program_mode2(uint16_t div);
uint16_t pit_read_counter(void);