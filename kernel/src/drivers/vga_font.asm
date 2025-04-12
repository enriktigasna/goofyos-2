section .rodata
global vga_font
align 4

vga_font:
    incbin "src/drivers/VGA8.F16"
