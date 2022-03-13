#include <utils.h>

struct pack IDTDescriptor
{
    uint16_t size;
    uint64_t offset;
};

struct pack IDTGateDescriptor
{
    uint16_t offset;
    uint16_t segmentselector;
    uint8_t ist;
    uint8_t attributes;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
};

void IDTInit();