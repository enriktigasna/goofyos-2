#pragma once
#include <stdint.h>

static inline uint64_t rdtsc(void) {
	uint32_t lo, hi;
	__asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	return ((uint64_t)hi << 32) | lo;
}

struct hpet {
	void *base;
	uint64_t period;
};

extern struct hpet global_hpet;
uint64_t hpet_counter();
void hpet_init();
void hpet_wait_us(uint64_t us);