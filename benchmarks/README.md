# Same-first-pass scaling benchmark

Build from the repository root (POSIX/Linux):

```sh
gcc -O2 -std=c99 -Wall -Wextra -pedantic \
  -D_POSIX_C_SOURCE=200809L -DBENCHMARK_LARGE_FIXTURES \
  -include benchmarks/traditional_second_pass.h \
  benchmarks/benchmark.c benchmarks/traditional_second_pass.c \
  firstPass.c secondPass.c file.c -o /tmp/matala14-benchmark
/tmp/matala14-benchmark
```

The default second-pass measurement uses 101 pairs and 10 warmup pairs per
size. The new end-to-end measurement uses 5 pairs and 1 warmup pair per size
because rebuilding the existing 11-bucket symbol table for every large run is
expensive. Each mode alternates execution order independently. Override either
sample count with odd values from 3 to 10001:

```sh
/tmp/matala14-benchmark 501 11
```

The first argument selects second-pass pairs; the second selects total pairs.
Output reports second-pass medians in microseconds and total medians in
milliseconds, each with `100 * (rescan_median - fix_median) / rescan_median`
and verification status. A negative reduction means the measured fix-list
path was slower; results are not clamped. Five total samples give an initial
estimate, so small differences should be interpreted cautiously.

## What is compared

A calls the existing `fixAndPatchList()`. B opens and rereads `.am`, parses
instructions, tracks operand positions starting at 100, and patches symbolic
operands using the completed symbol table. B never consults the fix list and
rewrites a backward reference that pass 1 already resolved. It does not
generate instruction/data words or change IC/DC.

For the second-pass-only measurements, the harness generates input and calls
the existing `firstRound()` once per size. Before every pair it restores two separate instruction arrays from that
untouched first-pass snapshot. Metadata, symbol table, fix list, and data array
are shared: these direct internal reference passes do not mutate them. Each
approach has its own ERROR flag and writable instruction array. Thus both
start from exactly the same first-pass state, without repeated expensive
symbol-table construction. Restoring bytes with memcpy includes uninitialized
reserved slots; they are never interpreted or compared before patching.

Every warmup and measured pair must finish without ERROR and match all
instruction words and ARE flags, counters, sizes, and data words/ARE. The
initial code length and unresolved count are checked against the generator's
expected values. Unused `MachineCode.place` and structure padding are excluded.
Equality checks compare the two existing behaviors, not an independent ISA
correctness oracle.

For second-pass-only results, only the two pass calls are timed using
CLOCK_MONOTONIC. Generation, allocation, first pass, restoration, correctness
checks, sorting, and cleanup are excluded.
Rescan time includes open/read/close and parsing. Warmups and preceding setup
keep the source in the filesystem cache; this does not measure cold disk I/O.
Clock overhead is included in both measurements.

## End-to-end: same-first-pass comparison

After the second-pass measurements, the same generated file is used for fresh
independent assembler states on every end-to-end pair, including warmup:

- Fix-list path: start timer, `firstRound()`, `fixAndPatchList()`, stop timer.
- Rescan path: start timer, `firstRound()`, `traditionalSecondPass()`, stop timer.

Both paths use the exact existing first pass and therefore both allocate and
build the fix list. This measures the specified same-first-pass pipeline, not
a classic assembler whose first pass omits that work. Total times are directly
measured intervals, not sums of independently measured medians.

First-pass file reads, parsing, symbol construction, code allocation/generation,
and fix-list construction are included. Fixture generation, initial zeroing of
the state, verification, output-file writing, and cleanup are excluded. Neither
`preAssembler()` nor `secondRound()` is called. Every completed pair must match
all instruction/data words and ARE flags, counters and array sizes, with no
ERROR flags; expected code length is also checked. Both states are freed only
after timing and verification. The file remains warm from setup and prior runs.

The source labels still occupy just 11 hash buckets. Although the forward
target has a short lookup chain, creating all other labels can dominate the
first-pass runtime at larger sizes. The total-runtime reduction therefore need
not resemble the roughly 96% reduction in second-pass time.

## Generated fixtures and limits

Fixtures have exactly 1,000, 10,000, 50,000, and 100,000 instruction lines.
For N lines there are N-2 forward `mov TARGET, r1` references, one backward
`mov START, r2`, and a final `TARGET: stop`: 3N-2 code words. Every instruction
has a unique label to accommodate the existing first pass's extra token advance
on unlabeled instructions. Incidental labels are generated outside TARGET's
hash bucket, keeping target lookup short as sizes increase. HASHSIZE remains
11; this is a controlled, single-target workload, not a hash-collision study.

Generated input lives in a unique temporary directory under `/tmp` and is
removed after successful or handled-failure runs. The checked-in small
`benchmarks/fixture.am` is preserved. A short `fixture` basename respects the
existing file helpers' 20-byte filename buffers.

The normal assembler cannot hold these larger inputs: RAMSIZE is 4096.
The benchmark build force-includes its own header, which includes guarded
`passes.h` and then overrides RAMSIZE to 1,048,576 only when
BENCHMARK_LARGE_FIXTURES is defined. Both approaches use this same build.
No assembler source/header is edited; the regular Makefile build is unaffected.
The benchmark refuses to run without its large-fixture build flag.

Encoding remains 12-bit, so larger addresses are truncated exactly as in the
existing assembler. These are synthetic scaling fixtures, not executable
programs within the original machine's address space. Output equality includes
that existing truncation behavior. Direct internal references are supported;
relative and external operands remain outside scope. First-pass and rescan
parsing limitations are not repaired by the benchmark. Existing assembler
compiler warnings may still appear.

## Example measured results

One run in this workspace using the documented optimized build. Second-pass
medians use 101 measured pairs after 10 warmups; same-first-pass total medians
use 5 measured pairs after 1 warmup. All pairs in both modes matched words and
ARE flags. All times below are milliseconds.

| Lines | Second-pass fix | Second-pass rescan | Reduction | Total fix | Total rescan | Total reduction |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1,000 | 0.054700 | 1.451906 | 96.23% | 3.786 | 7.003 | 45.94% |
| 10,000 | 0.411902 | 11.605945 | 96.45% | 148.035 | 156.380 | 5.34% |
| 50,000 | 2.419501 | 60.541123 | 96.00% | 4,814.576 | 4,518.387 | -6.56% |
| 100,000 | 5.032633 | 119.451356 | 95.79% | 26,952.630 | 28,220.155 | 4.49% |

The large first-pass costs dominate these totals. Timing variability in those
costs can exceed the second-pass savings, as illustrated by the negative total
reduction at 50,000 lines. These five-sample total medians are descriptive of
this run, not evidence of a consistent end-to-end speedup. Use more total
samples for a stronger estimate; the default full run can take several minutes.
