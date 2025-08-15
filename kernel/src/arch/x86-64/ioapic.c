#include <goofy-os/cpu.h>
#define IOAPICREDTBL(n) (0x10 + 2 * n)

void ioapic_write(struct ioapic *ioapic, uint8_t offset, uint32_t val) {
	*(volatile uint32_t *)ioapic->base = offset;
	*(volatile uint32_t *)(ioapic->base + 0x10) = val;
}

uint32_t ioapic_read(struct ioapic *ioapic, uint8_t offset, uint32_t val) {
	*(volatile uint32_t *)ioapic->base = offset;
	return *(volatile uint32_t *)(ioapic->base + 0x10);
}