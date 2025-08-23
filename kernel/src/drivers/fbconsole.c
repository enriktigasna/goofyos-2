#include <goofy-os/boot.h>
#include <goofy-os/fbcon.h>
#include <goofy-os/spinlock.h>
#include <goofy-os/vmalloc.h>
#include <limine.h>
#include <stdint.h>
#include <string.h>

#define BLACK 0
#define RED 1
#define GREEN 2
#define BROWN 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define LIGHT_GREY 7

#define DARK_GREY 8
#define LIGHT_RED 9
#define LIGHT_GREEN 10
#define LIGHT_BROWN 11
#define LIGHT_BLUE 12
#define LIGHT_MAGENTA 13
#define LIGHT_CYAN 14
#define WHITE 15

#define COLOR(fg, bg) (fg | (bg << 4))

#define BG(col) (col >> 4)
#define FG(col) (col & 0b1111)

struct fbconsole fbcon;

uint32_t ansi_colors[] = {
    0x000000, // black
    0xaa0000, // red
    0x00aa00, // green
    0xaa5500, // brown
    0x0000aa, // blue
    0xaa00aa, // magenta
    0x00aaaa, // cyan
    0xaaaaaa, // light grey
	      //
    0x555555, // dark grey
    0xff5555, // red
    0x55ff55, // green
    0xffff55, // brown
    0x5555ff, // blue
    0xff55ff, // magenta
    0x55ffff, // cyan
    0xffffff, // white
};

inline void put_glyph(struct fbconsole_queue_item todo) {
	volatile uint32_t *fb_ptr = fbcon.framebuf;
	uint8_t *glyph_ptr = &vga_font[todo.c * 16];

	for (int y = 0; y < 16; y++) {
		uint8_t row = glyph_ptr[y];

		for (int x = 0; x < 8; x++) {
			int fb_x = todo.x * 8 + x;
			int fb_y = todo.y * 16 + y;
			if (row & (0x80 >> x)) {
				fb_ptr[fb_y * (fbcon.pitch / 4) + fb_x] =
				    ansi_colors[FG(todo.col)];
			} else {
				fb_ptr[fb_y * (fbcon.pitch / 4) + fb_x] =
				    ansi_colors[BG(todo.col)];
			}
		}
	}
}

void console_flush() {
	for (int i = 0; i < fbcon.queue_items; i++) {
		put_glyph(fbcon.queue[i]);
	}
	fbcon.queue_items = 0;
}

static void queue_push(struct fbconsole_queue_item todo) {
	uint16_t *target =
	    &fbcon.term_buffer[todo.y * fbcon.width_char + todo.x];

	if (*target & 0xff == todo.c && *target >> 8 == todo.col) {
		return;
	}

	if (fbcon.queue_items >= fbcon.width_char * fbcon.height_char) {
		console_flush();
	}

	fbcon.term_buffer[todo.y * fbcon.width_char + todo.x] =
	    todo.c | (todo.col << 8);
	fbcon.queue[fbcon.queue_items++] = todo;
}

void scroll() {
	for (int i = 1; i < fbcon.height_char; i++) {
		for (int j = 0; j < fbcon.width_char; j++) {
			uint16_t tar =
			    fbcon.term_buffer[i * fbcon.width_char + j];
			queue_push(
			    (struct fbconsole_queue_item){.c = tar & 0xff,
							  .col = tar >> 8,
							  .x = j,
							  .y = i - 1});
		}
	}
	// Clear bottom
	for (int j = 0; j < fbcon.width_char; j++) {
		queue_push(
		    (struct fbconsole_queue_item){.c = ' ',
						  .col = fbcon.color,
						  .x = j,
						  .y = fbcon.height_char - 1});
	}
	fbcon.cursor_x = 0;
}

void console_carriage_return() {
	if (fbcon.cursor_y + 1 >= fbcon.height_char) {
		scroll();
	} else {
		fbcon.cursor_x = 0;
		fbcon.cursor_y++;
	}
}

void console_write_glyph(char c) {
	queue_push((struct fbconsole_queue_item){
	    .c = c,
	    .x = fbcon.cursor_x,
	    .y = fbcon.cursor_y,
	    .col = fbcon.color,
	});
}

void console_write(char *str) {
	acquire(&fbcon.lock);
	int idx = 0;
	uint8_t cur;

	while ((cur = str[idx++])) {
		if ((0x20 <= cur) && (cur < 0x80)) {
			console_write_glyph(cur);
			fbcon.cursor_x++;
		}

		if (fbcon.cursor_x >= fbcon.width_char || cur == '\n') {
			console_carriage_return();
		}
	}
	console_flush();
	release(&fbcon.lock);
}

void console_init(struct limine_framebuffer *framebuffer) {
	if (NO_FRAMEBUFFER)
		return;
	fbcon = (struct fbconsole){
	    .fb = framebuffer,
	    .pitch = framebuffer->pitch,
	    .framebuf = framebuffer->address,
	    .cursor_x = 0,
	    .cursor_y = 0,
	    .width_char = framebuffer->width / 8,
	    .height_char = framebuffer->height / 16,
	    .width = framebuffer->height,
	    .height = framebuffer->height,
	    .color = COLOR(WHITE, BLACK),
	    .enabled = true,
	};
	fbcon.queue = vzalloc(fbcon.width_char * fbcon.height_char *
			      sizeof(struct fbconsole_queue_item));
	fbcon.term_buffer =
	    vzalloc(fbcon.width_char * fbcon.height_char * sizeof(uint16_t));

	for (int i = 0; i < fbcon.width_char; i++) {
		for (int j = 0; j < fbcon.height_char; j++) {
			put_glyph((struct fbconsole_queue_item){
			    .x = i, .y = j, .c = ' ', .col = fbcon.color});
		}
	}
}