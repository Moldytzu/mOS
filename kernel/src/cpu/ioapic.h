#pragma once
#include <misc/utils.h>

void ioapicRedirectIRQ(uint8_t irq, uint16_t vector, uint16_t core);
void ioapicInit();