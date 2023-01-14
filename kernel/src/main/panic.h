#pragma once
#include <misc/utils.h>

#define panick(msg) panick_impl(__FILE__, __LINE__, msg)
void panick_impl(const char *file, size_t line, const char *msg);