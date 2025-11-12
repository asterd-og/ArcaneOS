#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
	_Atomic uint8_t state;
} spinlock_t;

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);
