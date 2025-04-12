#include <goofy-os/cpu.h>
#include <goofy-os/hcf.h>
#include <goofy-os/boot.h>
#include <goofy-os/fbcon.h>
#include <goofy-os/printk.h>
#include <stdint.h>
#include <stddef.h>
#include <limine.h>


/*
 * 1. Initialize early console
 * 2. Initialize CPU state with IDT, GDT, etc.
 * 3. Initialize memory manager
 */

void kmain() {
        cpu_init();
        limine_init();
        console_init(__limine_framebuffer);

        printk("Welcome to GoofyOS\n");


        hcf();
}
