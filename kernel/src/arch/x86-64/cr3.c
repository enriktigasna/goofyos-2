#include <stdint.h>

uint64_t __readcr3() {
	uint64_t __cr3_val;
	__asm__ __volatile__("mov {%%cr3, %0|%0, cr3}"
			     : "=r"(__cr3_val)
			     :
			     : "memory");
	return __cr3_val;
}