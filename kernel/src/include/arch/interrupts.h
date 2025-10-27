#pragma once

#include <stdint.h>
#include <arch/context.h>

void interrupts_init();
void interrupts_reload();
int interrupts_set(int state);
void interrupts_set_handler(uint8_t vector, void *handler);
uint8_t interrupts_alloc_vec();
void interrupts_handle_int(context_t *ctx);
void interrupts_eoi();
