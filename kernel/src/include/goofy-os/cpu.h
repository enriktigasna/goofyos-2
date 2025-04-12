#pragma once
#include <stdint.h>

struct gdt_descriptor {
        uint16_t size;
        uint64_t address;
} __attribute__((packed));

void cpu_init();
void init_gdt();
void set_gdt(struct gdt_descriptor* desc);
