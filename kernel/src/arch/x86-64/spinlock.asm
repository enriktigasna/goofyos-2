global raw_acquire

raw_acquire:
	mov rax, 1
	pause
	xchg rax, [rdi]
	test rax, rax
	jnz raw_acquire
	ret