#include <kernel/kprintf.h>
#include <lib/printf.h>
#include <arch/serial.h>
#include <drivers/fb.h>

int kprintf(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	char buffer[2048];
	const int ret = vsnprintf(buffer, 2048, fmt, va);
	va_end(va);
	serial_print(buffer);
	flanterm_write(global_fb->ft_ctx, buffer, ret);
	return ret;
}
