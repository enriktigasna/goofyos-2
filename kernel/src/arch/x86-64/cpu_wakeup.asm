global cpu_wakeup
extern new_cpu_wait
extern boot_context_cr3

cpu_wakeup:
    ; Get APIC ID
    mov eax, 1
    cpuid
    shr ebx, 24

    ; Index into kernel boot context stacks and get from there
    mov rax, [boot_context_cr3]
    mov cr3, rax
    jmp new_cpu_wait