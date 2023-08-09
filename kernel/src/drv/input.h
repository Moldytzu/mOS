#pragma once
#include <misc/utils.h>

char kbGetLastKey();
char *kbGetBuffer();

void inputInit();
void inputFlush();

void mouseGet(uint16_t *x, uint16_t *y);