#include <goofy-os/cpu.h>

#define PIC1 0x20
#define PIC2 0xA0
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)
#define PIC_EOI 0x20

#define ICW1_INIT 0x010
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

#define CASCADE_IRQ 2

void pic_eoi(uint8_t irq) {
	if (irq >= 8) {
		outb(PIC2_COMMAND, PIC_EOI);
	}

	outb(PIC1_COMMAND, PIC_EOI);
}

void pic_remap(uint8_t offset1, uint8_t offset2) {
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();

	outb(PIC1_DATA, offset1);
	io_wait();
	outb(PIC1_DATA, offset2);
	io_wait();

	outb(PIC1_DATA, 1 << CASCADE_IRQ);
	io_wait();
	outb(PIC1_DATA, 2);
	io_wait();

	outb(PIC1_DATA, ICW4_8086);
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

	outb(PIC1_DATA, 0);
	outb(PIC2_DATA, 0);
}