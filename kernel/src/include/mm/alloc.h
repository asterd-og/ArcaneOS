#pragma once

#include <stdint.h>
#include <stddef.h>

void alloc_init();
void *alloc(size_t size);
void free(void *ptr);
