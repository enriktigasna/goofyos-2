global set_gdt

set_gdt:
        lgdt  [rdi]
        lea rax, [rel .complete_flush]
        push 0x08
        push rax
        retfq

.complete_flush:
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        ret

