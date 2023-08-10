#pragma once
#include <stdint.h>
#include <stddef.h>

// cursor coordinates
extern uint16_t cursorX, cursorY;

void cursorUpdate();
void cursorRedraw();