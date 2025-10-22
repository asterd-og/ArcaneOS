#pragma once

#include <kernel/kprintf.h>
#include <arch/arch.h>

#define ASSERT(x) { \
	if (!(x)) { \
		kprintf(LOG_ERROR "Assertion failed: " #x	" @ " __FILE__ ":%d\n", __LINE__); \
		arch_die(); \
	} \
}
