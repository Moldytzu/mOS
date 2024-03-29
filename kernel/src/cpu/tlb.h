#pragma once
#include <misc/utils.h>
#include <cpu/control.h>

ifunc void tlbFlushAll()
{
    controlLoadCR3(controlReadCR3());
}

ifunc void tlbFlush(void *page)
{
    iasm("invlpg (%0)" ::"r"(page)
         : "memory");
}