#include <goofy-os/cpu.h>
#include <goofy-os/printk.h>
#include <goofy-os/time.h>

#define X2APIC_MSR_BASE 0x800
#define X2APIC_DISABLE 0x10000
#define X2APIC_SVR_ENABLE 0x100

#define X2APIC_TASKPRIOR 0x8
#define X2APIC_EOI 0xB
#define X2APIC_TIMER_DIV 0x3E
#define X2APIC_LDR 0xD
#define X2APIC_SVR 0xF
#define X2APIC_LVT_TMR 0x32
#define X2APIC_LVT_PERF 0x34
#define X2APIC_LVT_LINT0 0x35
#define X2APIC_LVT_LINT1 0x36
#define X2APIC_TIMER_INITCNT 0x38
#define X2APIC_TIMER_CURCNT 0x39

#define APICTMR_PERIODIC 0x20000

#define IOAPICREDTBL(n) (0x10 + 2 * n)

uint64_t tsc_per_second;

void ioapic_write(struct ioapic *ioapic, uint8_t offset, uint32_t val) {
	*(volatile uint32_t *)ioapic->base = offset;
	*(volatile uint32_t *)(ioapic->base + 0x10) = val;
}

uint32_t ioapic_read(struct ioapic *ioapic, uint8_t offset, uint32_t val) {
	*(volatile uint32_t *)ioapic->base = offset;
	return *(volatile uint32_t *)(ioapic->base + 0x10);
}

void pit_program_mode2(uint16_t div) {
	outb(0x43, 0x34); // ch0, lobyte/hibyte, mode 2, binary
	outb(0x40, div & 0xFF);
	outb(0x40, div >> 8);
}

uint16_t pit_read_counter(void) {
	outb(0x43, 0b0000000); // latch ch0
	uint8_t lo = inb(0x40);
	uint8_t hi = inb(0x40);
	return (hi << 8) | lo;
}

// Get approximation, not to be used for exact things

inline void x2apic_write(uint32_t reg, uint64_t val) {
	wrmsr(X2APIC_MSR_BASE + reg, val);
}

inline uint64_t x2apic_read(uint32_t reg) {
	uint64_t val;
	rdmsr(X2APIC_MSR_BASE + reg, &val);
	return val;
}

inline void x2apic_eoi() { x2apic_write(X2APIC_EOI, 0); }

uint64_t ticks_in_10ms;
void x2apic_calibrate_timer() {
	x2apic_write(X2APIC_TIMER_DIV, 0x3);
	x2apic_write(X2APIC_TIMER_INITCNT, 0xFFFFFFFF);
	hpet_wait_us(10000);
	x2apic_write(X2APIC_LVT_TMR, X2APIC_DISABLE);
	ticks_in_10ms = 0xFFFFFFFF - x2apic_read(X2APIC_TIMER_CURCNT);
}

void x2apic_init_timer() {
	printk("Initializing lapic timer\n");

	x2apic_write(X2APIC_SVR, 0xFF | X2APIC_SVR_ENABLE);
	x2apic_write(X2APIC_LVT_TMR, 0x20 | APICTMR_PERIODIC);
	x2apic_write(X2APIC_TIMER_DIV, 0x3);
	x2apic_write(X2APIC_TIMER_INITCNT, ticks_in_10ms);

	if (current_cpuid() == 1) {
		printk("Core %d\n", current_cpuid());
		printk("SVR: %p\n", x2apic_read(X2APIC_SVR));
		printk("Timer initcnt: %p\n",
		       x2apic_read(X2APIC_TIMER_INITCNT));
		printk("Timer div: %p\n", x2apic_read(X2APIC_TIMER_DIV));
		printk("Timer LVT: %p\n", x2apic_read(X2APIC_LVT_TMR));
	}
}