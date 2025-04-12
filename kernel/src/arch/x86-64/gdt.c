#include <stdint.h>
#include <goofy-os/cpu.h>

uint64_t gdt_table[5];
struct gdt_descriptor gdt_desc;

void init_gdt() {
        int idx = 0;
        gdt_table[idx++] = 0x0000000000000000;  // Null descriptor
        gdt_table[idx++] = 0x00AF9A000000FFFF;  // Kernel code segment
        gdt_table[idx++] = 0x00CF92000000FFFF;  // Kernel data segment
        gdt_table[idx++] = 0x00AFFA000000FFFF;  // User code segment
        gdt_table[idx++] = 0x00CFF2000000FFFF;  // User data segment

        gdt_desc.size = sizeof(gdt_table) - 1;
        gdt_desc.address = (uint64_t) &gdt_table;

        
        set_gdt(&gdt_desc);
}
