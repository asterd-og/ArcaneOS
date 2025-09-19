#pragma once

#include <stdint.h>
#include <lib/flanterm/flanterm.h>

typedef struct {
	uint32_t width;
	uint32_t height;
	uint8_t bpp;
	void *lfb;
	struct flanterm_context *ft_ctx;
} framebuffer_t;

extern framebuffer_t *global_fb;

void framebuffer_early_init(); // Initializes first global framebuffer
framebuffer_t *framebuffer_create(uint32_t width, uint32_t height, uint8_t bpp, void *lfb);
