#pragma once
#include <stdint.h>

typedef struct
{
    uint64_t startNanos;
    uint64_t endNanos;
    uint64_t elapsedNanos;
    uint64_t elapsedMiliseconds;
} benchmark_t;

#define BENCHMARK(ctx, workload) \
    {                            \
        benchmarkStart(ctx);     \
        workload;                \
        benchmarkEnd(ctx);       \
    }

void benchmarkStart(benchmark_t *ctx);
uint64_t benchmarkEnd(benchmark_t *ctx);