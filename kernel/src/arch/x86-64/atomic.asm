global atomic_inc
global atomic_dec

atomic_inc:
	lock inc qword [rdi]
	ret

atomic_dec:
	lock dec qword [rdi]
	ret