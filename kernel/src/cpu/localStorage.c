#include <cpu/localStorage.h>
#include <cpu/msr.h>

void localStorageLoadUserspace(cpu_local_storage_t *ptr)
{
    wrmsr(GS_BASE, (uint64_t)ptr); // set gs base in msr
}

void localStorageLoadKernel(cpu_local_storage_t *ptr)
{
    wrmsr(KERNEL_GS_BASE, (uint64_t)ptr); // set gs base in msr
}

cpu_local_storage_t *localStorageGetKernel()
{
    return (cpu_local_storage_t *)rdmsr(KERNEL_GS_BASE);
}

cpu_local_storage_t *localStorageGetUserspace()
{
    return (cpu_local_storage_t *)rdmsr(GS_BASE);
}