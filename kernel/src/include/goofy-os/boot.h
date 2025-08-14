#pragma once

void limine_init();
extern struct limine_framebuffer *__limine_framebuffer;
extern struct limine_memmap_response *__limine_memmap_response;
extern struct limine_hhdm_response *__limine_hhdm_response;
extern struct limine_smp_response *__limine_smp_response;
extern struct limine_rsdp_response *__limine_rsdp_response;