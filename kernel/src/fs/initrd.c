#include <goofy-os/boot.h>
#include <goofy-os/printk.h>
#include <stddef.h>

void unpack_initrd() {
	if (!__limine_module_response ||
	    __limine_module_response->module_count == 0) {
		printk("[!] NO MODULES FOUND. Cannot load initrd.\n");
		return;
	}
	struct limine_file **modules = __limine_module_response->modules;
	struct limine_file *initrd = NULL;
	for (int i = 0; i < __limine_module_response->module_count; i++) {
		printk("%s\n", modules[i]->string);
	}
}