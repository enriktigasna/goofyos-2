#include <goofy-os/fbcon.h>
#include <goofy-os/spinlock.h>
#include <limine.h>
#include <stdint.h>
#include <string.h>

struct fbconsole fbcon;

void console_init(struct limine_framebuffer *framebuffer) {
	fbcon = (struct fbconsole){.fb = framebuffer,
				   .cursor_x = 0,
				   .cursor_y = 0,
				   .width = framebuffer->width / 8,
				   .height = framebuffer->height / 16,
				   .color = 0xffffff};
}

void __console_carriage_return(void) {
	fbcon.cursor_x = 0;

	// TODO: Scroll
	if (fbcon.cursor_y < fbcon.height - 2) {
		fbcon.cursor_y++;
	} else {
		uint64_t shift = fbcon.fb->pitch * 16;
		void *src = fbcon.fb->address;
		void *dst = (void *)((uint64_t)fbcon.fb->address + shift);
		uint64_t volume = fbcon.fb->pitch * fbcon.fb->height - shift;

		memmove(src, dst, volume);
	}
}

// Write glyph under cursor
void __console_write_glyph(int glyph_idx) {
	volatile uint32_t *fb_ptr = fbcon.fb->address;
	uint8_t *glyph_ptr = &vga_font[glyph_idx * 16];

	for (int y = 0; y < 16; y++) {
		uint8_t row = glyph_ptr[y];

		for (int x = 0; x < 8; x++) {
			if (row & (0x80 >> x)) {
				int fb_x = fbcon.cursor_x * 8 + x;
				int fb_y = fbcon.cursor_y * 16 + y;
				fb_ptr[fb_y * (fbcon.fb->pitch / 4) + fb_x] =
				    fbcon.color;
			}
		}
	}
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

		if (fbcon.cursor_x >= fbcon.width || cur == '\n') {
			__console_carriage_return();
		}
	}
	release(&fbcon.lock);
}
