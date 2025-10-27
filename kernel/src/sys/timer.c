#include <sys/timer.h>
#include <mm/alloc.h>
#include <kernel/kprintf.h>

timer_t *timer_create(void *fn_oneshot) {
	timer_t *timer = (timer_t*)alloc(sizeof(timer_t));
	timer->oneshot = fn_oneshot;
	return timer;
}
