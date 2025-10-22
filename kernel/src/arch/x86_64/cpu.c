#include <arch/x86_64/cpu.h>

uint64_t cpu_read_msr(uint32_t msr) {
	uint32_t low = 0, high = 0;
	__asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
	return ((uint64_t)high << 32) | low;
}

void cpu_write_msr(uint32_t msr, uint64_t value) {
	__asm__ volatile ("wrmsr" : : "a"(value), "d"((uint32_t)(value >> 32)), "c"(msr));
}
