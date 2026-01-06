#pragma once

#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int val, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
int strlen(const char *s1);
int strcmp(const char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
size_t strcspn(const char *s, const char *charset);
