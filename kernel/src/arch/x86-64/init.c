#include <goofy-os/cpu.h>

void cpu_init()
{
  init_gdt();
  init_idt();
}
