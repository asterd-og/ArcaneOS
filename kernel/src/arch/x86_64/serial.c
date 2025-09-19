#include <arch/serial.h>
#include <arch/ports.h>

void serial_print(char *buffer) {
	while (*buffer)
		out8(0xE9, *buffer++);
}
