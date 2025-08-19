#include <goofy-os/fbcon.h>
#include <goofy-os/spinlock.h>
#include <limine.h>
#include <stdint.h>
#include <string.h>

struct fbconsole fbcon;

void console_init(struct limine_framebuffer *framebuffer, void *buffer) {
	fbcon = (struct fbconsole){
	    .fb = framebuffer,
	    .pitch = framebuffer->pitch,
	    .framebuf = framebuffer->address,
	    .buffer = buffer,
	    .cursor_x = 0,
	    .cursor_y = 0,
	    .width_char = framebuffer->width / 8,
	    .height_char = framebuffer->height / 16,
	    .width = framebuffer->height,
	    .height = framebuffer->height,
	    .color = 0xffffff,
	    .enabled = true,
	};
}

void __console_carriage_return(void) {
	fbcon.cursor_x = 0;

	// TODO: Scroll
	if (fbcon.cursor_y < fbcon.height_char - 2) {
		fbcon.cursor_y++;
	} else {
		uint64_t shift = fbcon.pitch * 16;
		void *src = fbcon.buffer;
		void *dst = (void *)((uint64_t)fbcon.buffer + shift);
		uint64_t volume = fbcon.pitch * fbcon.height - shift;

		memmove(src, dst, volume);
	}
}

// Write glyph under cursor
void __console_write_glyph(int glyph_idx) {
	volatile uint32_t *fb_ptr = fbcon.buffer;
	uint8_t *glyph_ptr = &vga_font[glyph_idx * 16];

	for (int y = 0; y < 16; y++) {
		uint8_t row = glyph_ptr[y];

		for (int x = 0; x < 8; x++) {
			if (row & (0x80 >> x)) {
				int fb_x = fbcon.cursor_x * 8 + x;
				int fb_y = fbcon.cursor_y * 16 + y;
				fb_ptr[fb_y * (fbcon.pitch / 4) + fb_x] =
				    fbcon.color;
			}
		}
	}
}

void console_flush() {
	memcpy(fbcon.framebuf, fbcon.buffer, fbcon.height * fbcon.pitch);
}

void console_write(char *str) {
	acquire(&fbcon.lock);
	int idx = 0;
	uint8_t cur;
	while ((cur = str[idx++])) {
		if ((0x20 <= cur) && (cur < 0x80)) {
			__console_write_glyph(cur);
			fbcon.cursor_x++;
		}

		if (fbcon.cursor_x >= fbcon.width_char || cur == '\n') {
			__console_carriage_return();
		}
	}
	console_flush();
	release(&fbcon.lock);
}
