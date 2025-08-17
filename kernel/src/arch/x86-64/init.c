#include <goofy-os/acpi.h>
#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/time.h>
#include <goofy-os/vmalloc.h>

uint64_t boot_context_cr3;
void **kernel_stacks;
void smp_init() {
	boot_context_cr3 = __readcr3();
	kernel_stacks = kmalloc(sizeof(void *) * n_cpus);

	struct limine_mp_info **cpus = __limine_mp_response->cpus;
	for (int i = 1; i < n_cpus; i++) {
		kernel_stacks[i] = vmalloc(KERNEL_STACK_SIZE);
		// Wake up CPU
		cpus[i]->goto_address = &cpu_wakeup;
	}
}

void detect_x2apic() {
	uint64_t val;
	rdmsr(0x802, &val);
	printk("IA32_APIC_BASE %p\n", val);
}

void cpu_init() {
	// Mask all pic
	pic_disable();
	detect_x2apic();
	acpi_init();
	hpet_init();
	x2apic_calibrate_timer();
	x2apic_init_timer();
	smp_init();
}
