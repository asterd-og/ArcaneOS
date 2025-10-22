#include <arch/x86_64/pit.h>
#include <arch/x86_64/ports.h>
#include <arch/interrupts.h>
#include <arch/arch.h>

uint64_t pit_counter = 0;

void pit_handler(void) {
	pit_counter++;
	interrupts_eoi();
}

void pit_init() {
	uint16_t div = PIT_FREQ / 1000;
	uint8_t mode = 0b110110;
	out8(PIT_MODE_PORT, mode);
	out8(PIT_CHANNEL0_PORT, (uint8_t)div);
	out8(PIT_CHANNEL0_PORT, (uint8_t)(div >> 8));
	interrupts_set_handler(32, pit_handler);
}

void pit_wait(uint64_t ms) {
	uint64_t expected = pit_counter + ms;
	while (pit_counter < expected) arch_pause();
}
