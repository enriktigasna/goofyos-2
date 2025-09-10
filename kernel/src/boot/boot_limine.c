// clang-format off
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <goofy-os/hcf.h>
#include <goofy-os/cpu.h>
#include <goofy-os/mm.h>

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

__attribute__((used, section(".limine_requests")))
static volatile struct limine_mp_request mp_request = {
    	.id = LIMINE_MP_REQUEST,
	.revision = 0,
	.flags = LIMINE_MP_X2APIC
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    	.id = LIMINE_RSDP_REQUEST,
	.revision = 0,
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    	.id = LIMINE_MODULE_REQUEST,
	.revision = 0,
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_executable_cmdline_request cmdline_request = {
    	.id = LIMINE_EXECUTABLE_CMDLINE_REQUEST,
	.revision = 0,
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;


struct limine_framebuffer* __limine_framebuffer;
struct limine_memmap_response* __limine_memmap_response;
struct limine_hhdm_response* __limine_hhdm_response;
struct limine_mp_response* __limine_mp_response;
struct limine_rsdp_response* __limine_rsdp_response;
struct limine_module_response* __limine_module_response;
struct limine_executable_cmdline_response* __limine_cmdline_response;


bool NO_FRAMEBUFFER;
void limine_init() {
        if (LIMINE_BASE_REVISION_SUPPORTED == false) {
                hcf();
        }

        // Ensure we got a framebuffer.
        if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
                NO_FRAMEBUFFER = true;
        }

	// Ensure we got memmap
	if (memmap_request.response == NULL) {
                hcf();
        }
	
	if (hhdm_request.response == NULL) {
                hcf();
        }
	
	if (mp_request.response == NULL) {
                hcf();
        }
	
	if (rsdp_request.response == NULL) {
                hcf();
        }

	if (!NO_FRAMEBUFFER)
        __limine_framebuffer = framebuffer_request.response->framebuffers[0];

	__limine_memmap_response = memmap_request.response;
	__limine_hhdm_response = hhdm_request.response;
	__limine_mp_response = mp_request.response;
	__limine_rsdp_response = rsdp_request.response;
	__limine_module_response = module_request.response;
	__limine_cmdline_response = cmdline_request.response;


	hhdm_offset = __limine_hhdm_response->offset;

	n_cpus = mp_request.response->cpu_count;
	for (int i = 0; i < n_cpus; i++)
	{
		cpu_cores[i].lapic_id = mp_request.response->cpus[i]->lapic_id;	
		cpu_cores[i].cli_count = 1;
	}
	
}
