#pragma once

#include <stdint.h>

struct spinlock {
	uint64_t locked;
};

void acquire(struct spinlock *lock);
void release(struct spinlock *lock);

void raw_acquire(uint64_t *locked);