#pragma once

#include <stdint.h>
#include <stddef.h>

#define PIT_FREQ 1193180
#define PIT_CHANNEL0_PORT 0x40
#define PIT_MODE_PORT 0x43

void pit_init();
void pit_wait(uint64_t ms);
