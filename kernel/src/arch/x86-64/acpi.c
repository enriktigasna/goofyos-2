#include <goofy-os/boot.h>
#include <goofy-os/cpu.h>
#include <goofy-os/printk.h>
#include <goofy-os/slab.h>
#include <goofy-os/spinlock.h>
#include <goofy-os/time.h>
#include <goofy-os/vmalloc.h>
#include <limine.h>
#include <uacpi/acpi.h>
#include <uacpi/tables.h>
#include <uacpi/uacpi.h>

struct ioapic ioapics[MAX_IOAPICS];

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
	*out_rsdp_address = __limine_rsdp_response->address;
	return UACPI_STATUS_OK;
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
	return vmap_contiguous(addr, len);
}

void uacpi_kernel_unmap(void *addr, uacpi_size) { vunmap_contiguous(addr); }

void uacpi_kernel_log(uacpi_log_level, const uacpi_char *msg) {
	printk("[UACPI] %s", msg);
}

void *uacpi_kernel_alloc_zeroed(size_t size) { return kzalloc(size); }

void *uacpi_kernel_alloc(size_t size) { return vmalloc(size); }

void uacpi_kernel_free(void *object) { vfree(object); }

struct acpi_entry_hdr *acpi_next(struct acpi_entry_hdr *hdr) {
	return (void *)hdr + hdr->length;
}

void parse_madt(uacpi_table table) {
	struct acpi_madt *madt = (struct acpi_madt *)table.hdr;
	struct acpi_entry_hdr *entry = (struct acpi_entry_hdr *)&madt->entries;
	struct acpi_entry_hdr *end =
	    (struct acpi_entry_hdr *)((void *)&madt->entries +
				      madt->hdr.length);

	struct acpi_entry_hdr *curr;
	for (curr = entry; curr < end; curr = acpi_next(curr)) {
		switch (curr->type) {
		case ACPI_MADT_ENTRY_TYPE_LOCAL_X2APIC:
		case ACPI_MADT_ENTRY_TYPE_LAPIC:
			struct acpi_madt_lapic *curr_lapic = (void *)curr;
			printk("FOUND APIC UID %d\n", curr_lapic->uid);
			break;
		case ACPI_MADT_ENTRY_TYPE_IOAPIC:
			struct acpi_madt_ioapic *curr_ioapic = (void *)curr;
			printk("FOUND IOAPIC ID %d\n", curr_ioapic->id);
			printk("BASE %p\n", curr_ioapic->address);
			ioapics[curr_ioapic->id].base =
			    vmap_contiguous(curr_ioapic->address, 0x1000);

			break;
		}
	}
}

void parse_hpet(uacpi_table table) {
	struct acpi_hpet *hpet = (struct acpi_hpet *)table.hdr;
	global_hpet.base = vmap_contiguous(hpet->address.address, 0x1000);
}

void acpi_init() {
	char tmp_buf[2048];

	uacpi_status ret =
	    uacpi_setup_early_table_access(tmp_buf, sizeof(tmp_buf));

	if (uacpi_unlikely_error(ret)) {
		printk("Failed to initialize early tables [%p] %s\n", ret,
		       uacpi_status_to_string(ret));
		return;
	}

	printk("Initialized early tables\n");

	uacpi_table madt;
	ret = uacpi_table_find_by_signature("APIC", &madt);
	if (uacpi_unlikely_error(ret)) {
		printk("Couldn't find MADT [%p] %s\n", ret,
		       uacpi_status_to_string(ret));
		return;
	}
	parse_madt(madt);

	uacpi_table hpet;
	ret = uacpi_table_find_by_signature("HPET", &hpet);
	if (uacpi_unlikely_error(ret)) {
		printk("Couldn't find HPET [%p] %s\n", ret,
		       uacpi_status_to_string(ret));
		return;
	}
	parse_hpet(hpet);

	return;
}