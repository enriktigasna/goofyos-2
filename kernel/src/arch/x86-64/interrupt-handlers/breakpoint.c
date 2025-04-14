#include <goofy-os/printk.h>

void breakpoint_interrupt() {
        printk("Hello from int3\n");
        return;
}
