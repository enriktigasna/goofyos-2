#include <goofy-os/cpu.h>

void cpu_init() {
	init_gdt();
	init_idt();
	pic_remap(0x20, 0x28);
}
