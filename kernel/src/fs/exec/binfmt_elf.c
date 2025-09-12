#include <goofy-os/binfmt.h>
#include <goofy-os/printk.h>
#include <goofy-os/uapi/elf.h>
#include <goofy-os/uapi/errno.h>
#include <goofy-os/vfs.h>
#include <string.h>

bool validate_elf(struct file *exe, struct elf64_hdr *hdr) {
	long exe_size = vfs_size(exe);

	if (!memcmp(hdr->e_ident, ELFMAG, SELFMAG)) {
		return false;
	}

	if (hdr->e_ident[EI_CLASS] != ELFCLASS64) {
		return false;
	}

	if (hdr->e_ident[EI_DATA] != ELFDATA2LSB) {
		return false;
	}

	if (hdr->e_machine != EM_X86_64) {
		return false;
	}

	if (hdr->e_version != EV_CURRENT) {
		return false;
	}

	if (hdr->e_type != ET_EXEC) {
		return false;
	}

	return true;
}

int run_elf_file(struct binprm *binprm) {
	struct file *executable = binprm->executable;
	struct elf64_hdr hdr;
	int err;

	long exe_size = vfs_size(executable);

	if (exe_size < sizeof(struct elf64_hdr)) {
		return -ENOEXEC;
	}

	// TODO: Remake it into a function that reads and checks
	err = vfs_pread(executable, &hdr, sizeof(struct elf64_hdr), 0);
	if (err != sizeof(struct elf64_hdr)) {
		return -ENOEXEC;
	}

	if (exe_size < (hdr.e_shoff))
		;

	return 0;
}