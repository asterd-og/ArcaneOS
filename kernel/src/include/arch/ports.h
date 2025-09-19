#pragma once

#include <stdint.h>

void out8(uint16_t port, uint8_t data);
uint8_t in8(uint16_t port);

void out16(uint16_t port, uint16_t data);
uint16_t in16(uint16_t port);

void out32(uint16_t port, uint32_t data);
uint32_t in32(uint16_t port);
