#ifndef __BENCH_H__
#define __BENCH_H__

#include <stdint.h>

// Kernel benchmarks, built with -DINCRO_BENCH=ON. kernel-csharp/Bench.cs runs the same workloads in
// the same order, so the two kernels can be compared. Each prints "[BENCH] <name>: <cycles>".
static inline uint64_t bench_now(void) { return __builtin_ia32_rdtsc(); }

void run_benchmarks(uint64_t boot_cycles);

#endif
