#include <lib/spinlock.h>
#include <arch/interrupts.h>
#include <arch/arch.h>

void spinlock_acquire(spinlock_t *lock) {
	lock->interrupts_state = interrupts_set(0);
	while (__atomic_test_and_set(&lock->state, __ATOMIC_ACQUIRE))
		arch_pause();
}

void spinlock_release(spinlock_t *lock) {
	__atomic_clear(&lock->state, __ATOMIC_RELEASE);
	interrupts_set(lock->interrupts_state);
}
