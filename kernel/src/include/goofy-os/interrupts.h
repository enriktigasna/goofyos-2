#pragma once
#include <goofy-os/cpu.h>
#include <stdint.h>

struct interrupt_frame {
	uint64_t ip;
	uint64_t cs;
	uint64_t flags;
	uint64_t sp;
	uint64_t ss;
};

void breakpoint_interrupt(struct interrupt_frame *frame);
void double_fault_stub(struct interrupt_frame *frame);
void timer_handler(struct interrupt_frame *frame);
void pagefault_handler(struct interrupt_frame *frame);

void dump_regs(struct regs *registers);