#pragma once
#include <goofy-os/spinlock.h>
#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FRAMEBUFFER_SIZE (1920 * 1080 * 4)

struct fbconsole_queue_item {
	uint16_t x;
	uint16_t y;
	uint8_t c;
	uint8_t col;
};

struct fbconsole {
	struct limine_framebuffer *fb;
	uint16_t pitch;
	uint16_t cursor_x;
	uint16_t cursor_y;
	uint16_t width_char;
	uint16_t height_char;
	uint16_t width;
	uint16_t height;
	uint8_t color;
	uint16_t *term_buffer;
	struct fbconsole_queue_item *queue;
	size_t queue_items;
	struct spinlock lock;
	void *framebuf;
	bool enabled;
};

extern uint8_t vga_font[];
extern struct fbconsole fbcon;
void console_init(struct limine_framebuffer *framebuffer);
void console_write(char *str);
void serial_init();
void serial_write(char c);
