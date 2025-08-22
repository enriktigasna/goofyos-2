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

struct registers {
	uint64_t rbp;
	uint64_t rax;
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;

	/*
	 * In interrupt context is vector number
	 * In scheduler is just padding for retpoline
	 */
	uint64_t vector_number;
	uint64_t error_code;

	uint64_t rip;
	uint64_t cs;
	uint64_t flags;
	uint64_t rsp;
	uint64_t ss;
} __attribute__((packed));

void isr_generic_stub(struct registers *frame);
void isr_generic_handler(struct registers *ctx);