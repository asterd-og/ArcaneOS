#pragma once

#include <stdint.h>
#include <stddef.h>

uint64_t cpu_read_msr(uint32_t msr);
void cpu_write_msr(uint32_t msr, uint64_t value);
