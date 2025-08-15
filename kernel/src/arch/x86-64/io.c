#include <stdint.h>

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

inline void io_wait(void) { outb(0x80, 0); }
