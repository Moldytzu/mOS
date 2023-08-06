#include <stdint.h>
#include <main/panic.h>

uintptr_t __stack_chk_guard = 0x595e9fbd94fda766;

__attribute__((noreturn)) void __stack_chk_fail(void)
{
    panick("Stack smashing detected");
    while (1)
        ;
}