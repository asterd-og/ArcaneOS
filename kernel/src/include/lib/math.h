#pragma once

#include <stdint.h>
#include <stddef.h>

static inline uint64_t clamp(uint64_t x, uint64_t a, uint64_t b) {
	return x < a ? a : (x > b ? b : x);
}
