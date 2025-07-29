#include <goofy-os/hcf.h>
#include <goofy-os/interrupts.h>
#include <goofy-os/printk.h>
#include <stdint.h>

void dump_regs(struct regs *registers) {
	printk("DOUBLE FAULT!\n");
	printk("rax %p\n", registers->rax);
	printk("rbx %p\n", registers->rbx);
	printk("rcx %p\n", registers->rcx);
	printk("rdx %p\n", registers->rdx);
	printk("rdi %p\n", registers->rdi);
	printk("rsi %p\n", registers->rsi);
	printk("r8 %p\n", registers->r8);
	printk("r9 %p\n", registers->r9);
	printk("r10 %p\n", registers->r10);
	printk("r11 %p\n", registers->r10);
	printk("r12 %p\n", registers->r12);
	printk("r13 %p\n", registers->r13);
	printk("r14 %p\n", registers->r14);
	printk("r15 %p\n", registers->r15);
	printk("rbp %p\n", registers->rbp);
	printk("rip %p\n", registers->rip);
	printk("rsp %p\n", registers->rsp);
	hcf();
	return;
}
