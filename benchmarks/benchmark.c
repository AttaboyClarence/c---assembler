#define _POSIX_C_SOURCE 200809L
#include <time.h>
#include <unistd.h>
#include "traditional_second_pass.h"

#define ITERATIONS 101
#define WARMUPS 10
#define TOTAL_ITERATIONS 5
#define TOTAL_WARMUPS 1

static double nowNs(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double)t.tv_sec * 1e9 + (double)t.tv_nsec;
}

static int compareDouble(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

static int sameWords(const AssemStrct *a, const AssemStrct *b)
{
    int i;
    if (a->ERROR || b->ERROR || a->IC != b->IC || a->DC != b->DC ||
        a->instructionArray.size != b->instructionArray.size ||
        a->dataArray.size != b->dataArray.size)
        return 0;
    for (i = 100; i < a->IC; ++i) {
        if (memcmp(a->instructionArray.item[i].word,
                   b->instructionArray.item[i].word, WORDLEN) != 0 ||
            a->instructionArray.item[i].ARE != b->instructionArray.item[i].ARE)
            return 0;
    }
    for (i = 0; i < a->DC; ++i)
        if (memcmp(a->dataArray.item[i].word, b->dataArray.item[i].word,
                   WORDLEN) != 0 ||
            a->dataArray.item[i].ARE != b->dataArray.item[i].ARE)
            return 0;
    return 1;
}

/* Labels are required to avoid the existing unlabeled-instruction parser
 * issue. Keep the target bucket free of incidental labels so this experiment
 * measures rescanning rather than an increasing target collision chain. */
static int generateFixture(int lines)
{
    FILE *fp = fopen("fixture.am", "w");
    char name[WORDSIZE], target[] = "TARGET";
    int i, candidate = 0, bucket = hashFunct(target);
    if (!fp)
        return 0;
    fprintf(fp, "START: mov TARGET, r1\n");
    for (i = 1; i < lines - 2; ++i) {
        do {
            snprintf(name, sizeof(name), "L%d", candidate++);
        } while (hashFunct(name) == bucket);
        fprintf(fp, "%s: mov TARGET, r1\n", name);
    }
    fprintf(fp, "BACK: mov START, r2\nTARGET: stop\n");
    {
        int ok = !ferror(fp);
        if (fclose(fp) != 0)
            ok = 0;
        return ok;
    }
}

/* Each interval includes a real first pass with its allocations and fix-list
 * construction. Zero-initialization, output comparison and freeing are outside. */
static int measureTotals(int lines, int iterations,
                         double *fixMedian, double *scanMedian)
{
    double *fixTimes = malloc((size_t)iterations * sizeof(double));
    double *scanTimes = malloc((size_t)iterations * sizeof(double));
    double start, fixElapsed, scanElapsed;
    int iteration, ok = 1;
    if (!fixTimes || !scanTimes) {
        fprintf(stderr, "Total benchmark allocation failed.\n");
        free(fixTimes);
        free(scanTimes);
        return 0;
    }
    for (iteration = -TOTAL_WARMUPS; iteration < iterations; ++iteration) {
        AssemStrct a = {0}, b = {0};
        if (iteration % 2 == 0) {
            start = nowNs();
            firstRound("fixture", &a);
            fixAndPatchList(&a);
            fixElapsed = nowNs() - start;
            start = nowNs();
            firstRound("fixture", &b);
            traditionalSecondPass("fixture.am", &b);
            scanElapsed = nowNs() - start;
        } else {
            start = nowNs();
            firstRound("fixture", &b);
            traditionalSecondPass("fixture.am", &b);
            scanElapsed = nowNs() - start;
            start = nowNs();
            firstRound("fixture", &a);
            fixAndPatchList(&a);
            fixElapsed = nowNs() - start;
        }
        if (!sameWords(&a, &b) || a.IC != 100 + 3 * (lines - 1) + 1) {
            fprintf(stderr, "Total output mismatch: %d lines, iteration %d.\n",
                    lines, iteration);
            ok = 0;
        }
        freeAssem(&a);
        freeAssem(&b);
        if (!ok)
            break;
        if (iteration >= 0) {
            fixTimes[iteration] = fixElapsed;
            scanTimes[iteration] = scanElapsed;
        }
    }
    if (ok) {
        qsort(fixTimes, (size_t)iterations, sizeof(double), compareDouble);
        qsort(scanTimes, (size_t)iterations, sizeof(double), compareDouble);
        *fixMedian = fixTimes[iterations / 2];
        *scanMedian = scanTimes[iterations / 2];
    }
    free(fixTimes);
    free(scanTimes);
    return ok;
}

static int runSize(int lines, int iterations, int totalIterations)
{
    AssemStrct baseline = {0}, a, b;
    MachineCode *aWords = NULL, *bWords = NULL;
    linkedList *p;
    double *fixTimes = NULL, *scanTimes = NULL;
    double start, fixElapsed, scanElapsed, fixMedian, scanMedian;
    double totalFixMedian, totalScanMedian;
    size_t bytes;
    int iteration, fixes = 0, ok = 0;
    if (!generateFixture(lines)) {
        perror("generate fixture");
        return 0;
    }
    firstRound("fixture", &baseline);
    for (p = baseline.fixList; p; p = p->next)
        ++fixes;
    if (baseline.ERROR || baseline.IC != 100 + 3 * (lines - 1) + 1 ||
        fixes != lines - 2 || baseline.externalNode != NULL) {
        fprintf(stderr, "Unexpected first-pass state at %d lines.\n", lines);
        goto cleanup;
    }
    bytes = (size_t)baseline.instructionArray.capacity * sizeof(MachineCode);
    aWords = malloc(bytes);
    bWords = malloc(bytes);
    fixTimes = malloc((size_t)iterations * sizeof(double));
    scanTimes = malloc((size_t)iterations * sizeof(double));
    if (!aWords || !bWords || !fixTimes || !scanTimes) {
        fprintf(stderr, "Benchmark allocation failed.\n");
        goto cleanup;
    }
    for (iteration = -WARMUPS; iteration < iterations; ++iteration) {
        /* Metadata, symbols, fix nodes and data come from the same snapshot.
         * These direct-internal passes only mutate their own code arrays and
         * ERROR flags. memcpy safely copies reserved, uninitialized bytes. */
        a = baseline;
        b = baseline;
        a.instructionArray.item = aWords;
        b.instructionArray.item = bWords;
        memcpy(aWords, baseline.instructionArray.item, bytes);
        memcpy(bWords, baseline.instructionArray.item, bytes);
        if (iteration % 2 == 0) {
            start = nowNs();
            fixAndPatchList(&a);
            fixElapsed = nowNs() - start;
            start = nowNs();
            traditionalSecondPass("fixture.am", &b);
            scanElapsed = nowNs() - start;
        } else {
            start = nowNs();
            traditionalSecondPass("fixture.am", &b);
            scanElapsed = nowNs() - start;
            start = nowNs();
            fixAndPatchList(&a);
            fixElapsed = nowNs() - start;
        }
        if (!sameWords(&a, &b)) {
            fprintf(stderr, "Output mismatch: %d lines, iteration %d.\n",
                    lines, iteration);
            goto cleanup;
        }
        if (iteration >= 0) {
            fixTimes[iteration] = fixElapsed;
            scanTimes[iteration] = scanElapsed;
        }
    }
    qsort(fixTimes, (size_t)iterations, sizeof(double), compareDouble);
    qsort(scanTimes, (size_t)iterations, sizeof(double), compareDouble);
    fixMedian = fixTimes[iterations / 2];
    scanMedian = scanTimes[iterations / 2];
    printf("%8d second pass: fix %.3f us, rescan %.3f us, reduction %.2f%%; match\n",
           lines, fixMedian / 1000.0, scanMedian / 1000.0,
           100.0 * (scanMedian - fixMedian) / scanMedian);
    fflush(stdout);
    if (!measureTotals(lines, totalIterations, &totalFixMedian, &totalScanMedian))
        goto cleanup;
    printf("%8d same-first-pass total: fix %.3f ms, rescan %.3f ms, reduction %.2f%%; match\n",
           lines, totalFixMedian / 1e6, totalScanMedian / 1e6,
           100.0 * (totalScanMedian - totalFixMedian) / totalScanMedian);
    fflush(stdout);
    ok = 1;
cleanup:
    free(aWords);
    free(bWords);
    free(fixTimes);
    free(scanTimes);
    freeAssem(&baseline);
    return ok;
}

int main(int argc, char **argv)
{
    const int sizes[] = {1000, 10000, 50000, 100000};
    int i, iterations = ITERATIONS, totalIterations = TOTAL_ITERATIONS, ok = 1;
    char directory[] = "/tmp/matala14-bench-XXXXXX";
    char *end;
    long requested;
    if (argc > 3) {
        fprintf(stderr, "Usage: %s [second-pass iterations [total iterations]]\n", argv[0]);
        return EXIT_FAILURE;
    }
    for (i = 1; i < argc; ++i) {
        requested = strtol(argv[i], &end, 10);
        if (*end || requested < 3 || requested > 10001 || requested % 2 == 0) {
            fprintf(stderr, "Iterations must be odd, between 3 and 10001.\n");
            return EXIT_FAILURE;
        }
        if (i == 1)
            iterations = (int)requested;
        else
            totalIterations = (int)requested;
    }
#ifndef BENCHMARK_LARGE_FIXTURES
    fprintf(stderr, "Rebuild with the large-fixture flags in benchmarks/README.md.\n");
    return EXIT_FAILURE;
#endif
    /* Short basename satisfies file.c's 20-byte filename buffer. The checked-in
     * fixture stays unchanged; generated input is removed after the run. */
    if (!mkdtemp(directory) || chdir(directory) != 0) {
        perror("create/enter temporary fixture directory");
        return EXIT_FAILURE;
    }
    printf("%d measured pairs, %d warmups per size; warm file cache; alternating order.\n",
           iterations, WARMUPS);
    printf("Same-first-pass totals: %d measured pairs, %d warmup; both paths build the fix list.\n",
           totalIterations, TOTAL_WARMUPS);
    fflush(stdout);
    for (i = 0; i < (int)(sizeof(sizes) / sizeof(sizes[0])); ++i)
        if (!runSize(sizes[i], iterations, totalIterations)) {
            ok = 0;
            break;
        }
    if (unlink("fixture.am") != 0 || chdir("/tmp") != 0 || rmdir(directory) != 0) {
        perror("remove temporary fixture");
        ok = 0;
    }
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
