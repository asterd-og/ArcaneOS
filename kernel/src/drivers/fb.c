#include <drivers/fb.h>
#include <limine.h>
#include <mm/pmm.h>
#include <lib/flanterm/backends/fb.h>

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0
};

framebuffer_t early_fb;
framebuffer_t *global_fb = NULL;

void framebuffer_early_init() {
	struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
	early_fb.width = fb->width;
	early_fb.height = fb->height;
	early_fb.bpp = fb->bpp;
	early_fb.lfb = fb->address;
	early_fb.ft_ctx = flanterm_fb_init(
		NULL,
		NULL,
		fb->address, fb->width, fb->height, fb->pitch,
		fb->red_mask_size, fb->red_mask_shift,
		fb->green_mask_size, fb->green_mask_shift,
		fb->blue_mask_size, fb->blue_mask_shift,
		NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, 0, 0, 1,
		0, 0,
		0
	);
	global_fb = &early_fb; // So we can access global fb as a pointer, not necessary.
}
