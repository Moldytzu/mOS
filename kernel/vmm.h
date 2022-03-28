#pragma once
#include <utils.h>

struct pack vmm_index
{
    uint64_t PDP; 
    uint64_t PD; 
    uint64_t PT;
    uint64_t P;  
};

struct vmm_index vmmIndex(uint64_t virtualAddress);
void vmmInit();