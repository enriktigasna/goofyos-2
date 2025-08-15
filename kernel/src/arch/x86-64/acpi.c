#include <goofy-os/boot.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/spinlock.h>
#include <goofy-os/vmalloc.h>
#include <uacpi/uacpi.h>

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
	*out_rsdp_address = __limine_rsdp_response->address;
	return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
	return vmap_contiguous(addr, len);
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) { vunmap_contiguous(addr); }

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char *msg) {
	printk("[UACPI] %s", msg);
}

void *uacpi_kernel_alloc_zeroed(size_t size) {
	printk("Trying too allocate %d\n", size);
	return kzalloc(size);
}

void *uacpi_kernel_alloc(size_t size) {
	printk("Trying too allocate %d\n", size);
	return vmalloc(size);
}

void uacpi_kernel_free(void *object) { vfree(object); }

void acpi_init() {
	uacpi_status ret = uacpi_setup_early_table_access(kzalloc(2048), 2048);

	if (uacpi_unlikely_error(ret)) {
		printk("Failed to initialize early tables [%p] %s\n", ret,
		       uacpi_status_to_string(ret));
		return;
	}

	printk("Initialized early tables\n");

	return;
}