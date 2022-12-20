#pragma once
#include <misc/utils.h>

pstruct
{
    uint8_t keys[16];        // key buffers
    uint16_t mouseX, mouseY; // mouse coordonates
} drv_type_input_t;

extern drv_type_input_t drv_type_input_s;

void kbAppendChar(char c);
char kbGetLastKey();
char *kbGetBuffer();

void inputInit();
void inputFlush();