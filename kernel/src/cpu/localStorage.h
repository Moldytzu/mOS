#pragma once
#include <misc/utils.h>

#define KERNEL_GS_BASE 0xC0000102
#define GS_BASE 0xC0000101

typedef struct
{
    void *kernelSyscallStack;
    void *userspaceSyscallStack;
    uint64_t kernel;
} cpu_local_storage_t;

void localStorageLoadKernel(cpu_local_storage_t *ptr);
void localStorageLoadUserspace(cpu_local_storage_t *ptr);
cpu_local_storage_t *localStorageGetKernel();
cpu_local_storage_t *localStorageGetUserpsace();