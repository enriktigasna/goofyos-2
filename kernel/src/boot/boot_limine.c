// clang-format off
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <goofy-os/hcf.h>

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
        .id = LIMINE_FRAMEBUFFER_REQUEST,
        .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0,
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    	.id = LIMINE_HHDM_REQUEST,
	.revision = 0,
};


__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;


struct limine_framebuffer* __limine_framebuffer;
struct limine_memmap_response* __limine_memmap_response;
struct limine_hhdm_response* __limine_hhdm_response;

void limine_init() {
        if (LIMINE_BASE_REVISION_SUPPORTED == false) {
                hcf();
        }

        // Ensure we got a framebuffer.
        if (framebuffer_request.response == NULL) {
                hcf();
        }

        if (framebuffer_request.response->framebuffer_count < 1) {
                hcf();
        }
        
	// Ensure we got memmap
	if (memmap_request.response == NULL) {
                hcf();
        }
	
	if (hhdm_request.response == NULL) {
                hcf();
        }

        __limine_framebuffer = framebuffer_request.response->framebuffers[0];
	__limine_memmap_response = memmap_request.response;
	__limine_hhdm_response = hhdm_request.response;
}
