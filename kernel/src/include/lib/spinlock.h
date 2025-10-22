#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	_Atomic uint8_t state;
	int interrupts_state;
} spinlock_t;

#define NEW_SPINLOCK(x) spinlock_t x = {0, 0}

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);
