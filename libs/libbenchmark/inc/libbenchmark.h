#pragma once
#include <stdint.h>

#define BENCHMARK(resultVariable, workload) \
    {                                       \
        benchmarkStart();                   \
        workload;                           \
        resultVariable = benchmarkEnd();    \
    }

void benchmarkStart();
uint64_t benchmarkEnd();