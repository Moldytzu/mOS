#include <libbenchmark.h>
#include <mos/sys.h>

void benchmarkStart(benchmark_t *ctx)
{
    ctx->startNanos = sys_time_uptime_nanos();
}

uint64_t benchmarkEnd(benchmark_t *ctx)
{
    ctx->endNanos = sys_time_uptime_nanos();
    ctx->elapsedNanos = ctx->endNanos - ctx->startNanos;
    ctx->elapsedMiliseconds = ctx->elapsedNanos / 1000000;
    return ctx->elapsedMiliseconds;
}
