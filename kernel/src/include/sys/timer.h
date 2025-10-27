#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct timer {
	void(*oneshot)(struct timer *timer, uint64_t ms, uint8_t vector);
} timer_t;

timer_t *timer_create(void *fn_oneshot);
