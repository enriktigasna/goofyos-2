#pragma once;

struct file;

struct binprm {
	struct file *executable;
};

int run_elf_file(struct binprm *binprm);
int exec_file(struct file *file);