#include <goofy-os/cpu.h>
#include <stdint.h>

#define SERIAL_BAUD 9600

void serial_init() {
	outb(0x3f8 + 3, 0x00);
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x80);

	uint16_t divisor = (uint16_t)(115200 / SERIAL_BAUD);
	outb(0x3f8 + 0, divisor & 0xff);
	outb(0x3f8 + 1, (divisor >> 8) & 0xff);

	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x03);
	outb(0x3f8 + 2, 0xc7);
	outb(0x3f8 + 4, 0x0b);
}

void serial_write(char c) {
	while ((inb(0x3f8 + 5) & 0x20) == 0)
		;
	outb(0x3f8, c);
}