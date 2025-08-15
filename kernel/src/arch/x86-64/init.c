#include <goofy-os/acpi.h>
#include <goofy-os/cpu.h>

void cpu_init() {
	// Mask all pic
	pic_disable();
	acpi_init();
}
