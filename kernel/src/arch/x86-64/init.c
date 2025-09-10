#include <goofy-os/acpi.h>
#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <goofy-os/mm.h>
#include <goofy-os/printk.h>
#include <goofy-os/sched.h>
#include <goofy-os/slab.h>
#include <goofy-os/time.h>
#include <goofy-os/vmalloc.h>

uint64_t boot_context_cr3;
void smp_init() {
	boot_context_cr3 = __readcr3();
	struct limine_mp_info **cpus = __limine_mp_response->cpus;
	for (int i = 1; i < n_cpus; i++) {
		// Wake up CPU
		cpus[i]->goto_address = &cpu_wakeup;
	}
}

void detect_x2apic() {
	uint64_t val;
	rdmsr(0x802, &val);
}

void set_tss_desc(struct tss_desc *desc, struct tss *task_seg) {
	uint64_t base = (uint64_t)task_seg;
	uint32_t limit = sizeof(*task_seg) - 1;

	desc->limit_low = limit & 0xFFFF;
	desc->base_low = base & 0xFFFF;
	desc->base_mid1 = (base >> 16) & 0xFF;
	desc->type = 0x9;
	desc->zero = 0;
	desc->dpl = 0;
	desc->p = 1;
	desc->limit_high = (limit >> 16) & 0xF;
	desc->avl = 0;
	desc->zero2 = 0;
	desc->g = 0;
	desc->base_mid2 = (base >> 24) & 0xFF;
	desc->base_high = (base >> 32);
	desc->reserved = 0;
}

void tss_init() {
	uint64_t gdt_idx = 5;
	for (int i = 0; i < n_cpus; i++) {
		struct tss *task_seg = kzalloc(sizeof(struct tss));

		// Stack grows downward, 16 aligned, this is why its like this
		void *exception_stack =
		    vmalloc(INTERRUPT_STACK_SIZE) + INTERRUPT_STACK_SIZE - 0x10;
		void *interrupt_stack =
		    vmalloc(INTERRUPT_STACK_SIZE) + INTERRUPT_STACK_SIZE - 0x10;

		task_seg->ist1_high = (uint64_t)exception_stack >> 32;
		task_seg->ist1_low = (uint64_t)exception_stack & 0xFFFFFFFF;

		task_seg->ist2_high = (uint64_t)interrupt_stack >> 32;
		task_seg->ist2_low = (uint64_t)interrupt_stack & 0xFFFFFFFF;

		cpu_cores[i].tss = task_seg;
		set_tss_desc((struct tss_desc *)&gdt_table[gdt_idx], task_seg);
		gdt_idx += 2;
	}
}

void tss_percpu_init() {
	unsigned short selector = (5 + current_cpuid() * 2) * 8;
	__asm__ __volatile__("ltr %0" ::"m"(selector) : "memory");
}

void cpu_init() {
	// Mask all pic
	pic_disable();
	detect_x2apic();
	acpi_init();
	hpet_init();
	x2apic_calibrate_timer();
	x2apic_init_timer();
	tss_init();
	tss_percpu_init();
	sched_init();
	smp_init();
}
