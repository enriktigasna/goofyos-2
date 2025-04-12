#include <limine.h>
#include <goofy-os/fbcon.h>
#include <stdint.h>

struct fbconsole {
        struct limine_framebuffer *fb;
        uint16_t cursor_x;
        uint16_t cursor_y;
        uint16_t width;
        uint16_t height;
        uint32_t color;
        // spinlock_t lock
};

struct fbconsole fbcon;

void console_init(struct limine_framebuffer* framebuffer) {
        fbcon = (struct fbconsole) {
                .fb = framebuffer,
                .cursor_x = 0,
                .cursor_y = 0,
                .width = framebuffer->width / 4,
                .height = framebuffer->height / 8,
                .color = 0xffffff
        };
}

void __console_carriage_return(void) {
        fbcon.cursor_x = 0;

        // TODO: Scroll
        fbcon.cursor_y++;
}

// Write glyph under cursor
void __console_write_glyph(int glyph_idx) {
        volatile uint32_t *fb_ptr = fbcon.fb->address;
        uint8_t *glyph_ptr = &vga_font[glyph_idx * 16];

        for (int y=0; y<16; y++) {
                uint8_t row = glyph_ptr[y];

                for (int x=0; x<8; x++) {
                        if (row & (0x80 >> x)) {
                                int fb_x = fbcon.cursor_x * 8 + x;
                                int fb_y = fbcon.cursor_y * 16 + y;
                                fb_ptr[fb_y * (fbcon.fb->pitch / 4) + fb_x] = fbcon.color;
                        }
                }
        }
}

void console_write(char *str) {
        int idx = 0;
        uint8_t cur;
        while ( (cur = str[idx++]) ) {
                if ((0x20 <= cur) && (cur < 0x80)) {
                        __console_write_glyph(cur);
                        fbcon.cursor_x++;
                }

                if (fbcon.cursor_x >= fbcon.width || cur == '\n') {
                        __console_carriage_return();
                }
        }
}

