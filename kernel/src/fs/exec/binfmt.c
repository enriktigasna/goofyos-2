#include <goofy-os/binfmt.h>
#include <goofy-os/slab.h>
#include <goofy-os/uapi/elf.h>
#include <goofy-os/uapi/errno.h>

int exec_file(struct file *file) {
	struct binprm *binprm = kzalloc(sizeof(struct binprm));
	int err;

	binprm->executable = file;

	err = run_elf_file(binprm);
	if (!err)
		return 0;

	return -ENOEXEC;
}