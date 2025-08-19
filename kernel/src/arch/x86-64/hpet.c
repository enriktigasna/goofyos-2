#include <goofy-os/printk.h>
#include <goofy-os/time.h>
#include <stdint.h>

struct hpet global_hpet;

#define HPET_MAIN_COUNTER 0xF0
#define HPET_GEENRAL_CAPABILITIES 0x0
#define HPET_GENERAL_CONFIGURATION 0x10

#define FEMTOSECONDS_PER_US 1000000000

uint64_t hpet_read(uint32_t reg) {
	return *(uint64_t *)(global_hpet.base + reg);
}

void hpet_write(uint32_t reg, uint64_t val) {
	*(uint64_t *)(global_hpet.base + reg) = val;
}

inline uint64_t hpet_counter() { return hpet_read(HPET_MAIN_COUNTER); }

void hpet_wait_us(uint64_t us) {
	uint64_t cur = hpet_counter();
	uint64_t target = cur + (us * FEMTOSECONDS_PER_US) / global_hpet.period;

	while (hpet_counter() < target) {
		__asm__ __volatile__("pause" ::: "memory");
	}
}

void hpet_wait_us_yield(uint64_t us) {
	uint64_t cur = hpet_counter();
	uint64_t target = cur + (us * FEMTOSECONDS_PER_US) / global_hpet.period;

	while (hpet_counter() < target) {
		__asm__ __volatile__("int $32");
	}
}

void hpet_init() {
	uint64_t period = hpet_read(HPET_GEENRAL_CAPABILITIES) >> 32;
	printk("Period is %p\n", period);
	global_hpet.period = period;

	hpet_write(HPET_GENERAL_CONFIGURATION, 0);
	hpet_write(HPET_MAIN_COUNTER, 0);
	hpet_write(HPET_GENERAL_CONFIGURATION, 1);
}