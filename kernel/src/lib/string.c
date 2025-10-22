#include <lib/string.h>

void *memcpy(void *dest, const void *src, size_t n) {
	uint8_t *dest8 = (uint8_t*)dest;
	const uint8_t *src8 = (uint8_t*)src;
	for (size_t i = 0; i < n; i++)
		dest8[i] = src8[i];
	return dest;
}

void *memset(void *dest, int val, size_t n) {
	uint8_t *dest8 = (uint8_t*)dest;
	for (size_t i = 0; i < n; i++)
		dest8[i] = val;
	return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
	uint8_t *dest8 = (uint8_t*)dest;
	const uint8_t *src8 = (uint8_t*)src;
	if (src > dest)
		for (size_t i = 0; i < n; i++)
			dest8[i] = src8[i];
	else
		for (size_t i = n; i > 0; i--)
			dest8[i-1] = src8[i-1];
	return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
	const uint8_t *p1 = (const uint8_t*)s1;
	const uint8_t *p2 = (const uint8_t*)s2;
	for (size_t i = 0; i < n; i++)
		if (p1[i] != p2[i])
			return p1[i] < p2[i] ? -1 : 1;
	return 0;
}

int strlen(const char *s1) {
	int i = 0;
	while (s1[i]) i++;
	return i;
}
