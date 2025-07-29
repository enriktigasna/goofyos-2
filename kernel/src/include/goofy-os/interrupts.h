#pragma once
#include <stdint.h>

struct interrupt_frame {
	uint64_t ip;
	uint64_t cs;
	uint64_t flags;
	uint64_t sp;
	uint64_t ss;
};

void breakpoint_interrupt(struct interrupt_frame *frame);