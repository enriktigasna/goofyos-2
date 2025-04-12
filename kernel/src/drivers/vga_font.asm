section .rodata
global vga_font
align 4

vga_font:
    incbin "src/drivers/TOSH-SAT.F16"
