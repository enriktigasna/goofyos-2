#pragma once
#include <goofy-os/spinlock.h>
#include <limine.h>
#include <stdint.h>

struct fbconsole {
	struct limine_framebuffer *fb;
	uint16_t cursor_x;
	uint16_t cursor_y;
	uint16_t width;
	uint16_t height;
	uint32_t color;
	struct spinlock lock;
};

extern uint8_t vga_font[];
extern struct fbconsole fbcon;
void console_init(struct limine_framebuffer *framebuffer);
void console_write(char *str);
