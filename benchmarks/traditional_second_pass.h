#ifndef BENCHMARK_TRADITIONAL_SECOND_PASS_H
#define BENCHMARK_TRADITIONAL_SECOND_PASS_H

#include "../passes.h"

/* Force-include this header in every benchmark translation unit. This only
 * lifts the allocation ceiling; word encoding and hash-table size stay intact. */
#ifdef BENCHMARK_LARGE_FIXTURES
#undef RAMSIZE
#define RAMSIZE 1048576
#endif

/* Full .am path. Supports valid direct internal operands, immediates,
 * registers, and zero-operand instructions. Sets ERROR on unsupported input. */
void traditionalSecondPass(const char *filename, AssemStrct *assem);

#endif
