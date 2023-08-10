#pragma once
#include <stdint.h>
#include <stddef.h>

void fontLoad(const char *path);
void fontPlot(char c, uint16_t x, uint16_t y, uint32_t colour);
void fontWriteStr(const char *str, uint16_t x, uint16_t y, uint32_t colour);