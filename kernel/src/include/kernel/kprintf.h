#pragma once

#include <stdint.h>

#define LOG_INFO "[ \x1b[36mINFO\x1b[0m ] "
#define LOG_OK "[ \x1b[32mOK\x1b[0m ] "
#define LOG_WARN "[ \x1b[93mWARNING\x1b[0m ] "
#define LOG_ERROR "[ \x1b[31mERROR\x1b[0m ] "

int kprintf(const char *fmt, ...);
