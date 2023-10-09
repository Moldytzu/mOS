#include <mos/sys.h>
#include <mos/drv.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const volatile drv_metadata_section_t metadata __attribute__((section(".mdrivermeta"))) = {
    .friendlyName = "blank",
};

void _mdrvmain()
{
    // use this as a template for other drivers

    while (1)
        ;
}