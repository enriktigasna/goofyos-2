void hcf() {
	__asm__ volatile("cli");
	for (;;) {
		__asm__ volatile("hlt");
	}
}
