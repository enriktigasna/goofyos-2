global vm_invalidate

vm_invalidate:
    mov rax, rdi
    invlpg [rax]
    ret