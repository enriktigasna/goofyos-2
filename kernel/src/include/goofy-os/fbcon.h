#pragma once
#include <limine.h>
#include <stdint.h>

extern uint8_t vga_font[];
void console_init(struct limine_framebuffer *framebuffer);
void console_write(char *str);
