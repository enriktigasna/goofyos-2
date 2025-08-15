global current_cpuid

current_cpuid:
    push rbx
    mov eax, 1
    cpuid
    shr ebx, 24
    mov eax, ebx
    pop rbx
    ret