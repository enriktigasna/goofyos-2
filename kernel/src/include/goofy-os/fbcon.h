#pragma once
#include <goofy-os/spinlock.h>
#include <limine.h>
#include <stdbool.h>
#include <stdint.h>

#define FRAMEBUFFER_SIZE (1920 * 1080 * 4)

struct fbconsole {
	struct limine_framebuffer *fb;
	uint16_t pitch;
	uint16_t cursor_x;
	uint16_t cursor_y;
	uint16_t width_char;
	uint16_t height_char;
	uint16_t width;
	uint16_t height;
	uint32_t color;
	struct spinlock lock;
	void *framebuf;
	void *buffer;
	bool enabled;
};

extern uint8_t vga_font[];
extern struct fbconsole fbcon;
void console_init(struct limine_framebuffer *framebuffer, void *buffer);
void console_write(char *str);
