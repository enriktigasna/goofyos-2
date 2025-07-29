extern dump_regs
global double_fault_stub

double_fault_stub:
	push rbp
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rsi
	push rdi
	push rdx
	push rcx
	push rbx
	push rax
	mov rbx, [rsp+0x90]
	mov rax, [rsp+0x78]
	push rbx
	push rax
	mov rdi, rsp
	call dump_regs