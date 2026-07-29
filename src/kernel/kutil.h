#include <stdint.h>
#include <stddef.h>
#pragma once

extern "C" void* memset(void* dst, int value, size_t n);
extern "C" int memcmp(const void* a, const void* b, size_t n);
extern "C" void* memcpy(void* dst, const void* src, size_t n);
