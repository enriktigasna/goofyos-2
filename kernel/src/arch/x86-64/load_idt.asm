global set_idt

set_idt:
        lidt  [rdi]
        sti
        ret
