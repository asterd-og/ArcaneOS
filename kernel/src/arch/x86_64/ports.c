#include <arch/x86_64/ports.h>

void out8(uint16_t port, uint8_t data) {
	__asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port) : "memory");
}

uint8_t in8(uint16_t port) {
	uint8_t data = 0;
	__asm__ volatile ("inb %1, %0" : "=a"(data) : "Nd"(port) : "memory");
	return data;
}

void out16(uint16_t port, uint16_t data) {
	__asm__ volatile ("outw %0, %1" : : "a"(data), "Nd"(port) : "memory");
}

uint16_t in16(uint16_t port) {
	uint16_t data = 0;
	__asm__ volatile ("inw %1, %0" : "=a"(data) : "Nd"(port) : "memory");
	return data;
}

void out32(uint16_t port, uint32_t data) {
	__asm__ volatile ("outl %0, %1" : : "a"(data), "Nd"(port) : "memory");
}

uint32_t in32(uint16_t port) {
	uint32_t data = 0;
	__asm__ volatile ("inl %1, %0" : "=a"(data) : "Nd"(port) : "memory");
	return data;
}
