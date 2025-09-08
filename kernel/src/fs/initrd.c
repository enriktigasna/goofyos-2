#include <goofy-os/boot.h>
#include <goofy-os/printk.h>
#include <goofy-os/uapi/stat.h>
#include <goofy-os/vfs.h>
#include <stddef.h>
#include <string.h>

#define USTAR_REG '0'
#define USTAR_REG_2 '\0'
#define USTAR_DIR '5'

struct posix_ustar_header {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag;
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];

	char pad[12];
};

int oct2bin(char *str, int size) {
	int n = 0;
	for (int i = 0; i < size; i++) {
		n *= 8;
		n += str[i] - '0';
	}
	return n;
}

struct posix_ustar_header *ustar_next(struct posix_ustar_header *header) {
	// Align to 512 and add another 512 block
	int filesize = oct2bin((char *)&header->size, 11);
	int aligned = (((filesize + 511) / 512) + 1) * 512;

	struct posix_ustar_header *new = (void *)header + aligned;
	if (strcmp((char *)&new->magic, "ustar"))
		return NULL;

	return new;
}

void unpack_ustar(int length, struct posix_ustar_header *fs) {
	for (; fs; fs = ustar_next(fs)) {
		printk("%s\n", fs->name);
		if (!strcmp(fs->name, "./"))
			continue;

		int err;
		switch (fs->typeflag) {
		case USTAR_DIR:
			err = vfs_mkdir(fs->name, NULL, 0);
			printk("mkdir response %d\n", err);
			break;
		case USTAR_REG_2:
		case USTAR_REG:
			err = vfs_create(fs->name, NULL, 0);
			printk("create response %d\n", err);
			break;
		}
	};
}

// After we fill our tmpfs with ustar we can free module pages
void unpack_initrd() {
	if (!__limine_module_response ||
	    __limine_module_response->module_count == 0) {
		printk("[!] NO MODULES FOUND. Cannot load initrd.\n");
		return;
	}
	struct limine_file **modules = __limine_module_response->modules;
	struct limine_file *initrd = NULL;
	for (int i = 0; i < __limine_module_response->module_count; i++) {
		if (!strcmp((char *)modules[i]->path, "/boot/initrd")) {
			printk("Found initrd!\n");
			unpack_ustar(modules[i]->size, modules[i]->address);
		}
	}
}