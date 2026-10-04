# Two-Pass Assembler in C

A two-pass assembler written in C as part of my Computer Science studies.

The main design goal of the project was to avoid rereading and reparsing the source file during the second pass. Instead, the assembler resolves as much as possible during the first pass and stores unresolved forward references in a fix list so they can be patched directly once the symbol table is complete.

## Design

During the first pass, the assembler:

- Reads the preprocessed `.am` file
- Builds the symbol table
- Generates instruction and data words
- Resolves symbols that have already been defined
- Stores unresolved forward references in a fix list together with the location that must later be patched

For example, if an instruction references a label that has not yet been defined, the assembler reserves the appropriate location in the instruction array and adds the label and its index to the fix list.

After the first pass is complete, the symbol table contains the required addresses. The second logical pass therefore does not need to read the source again — it simply walks through the fix list and patches the unresolved references.

## Implementation

The project uses several data structures to keep the assembler state organized and avoid global variables.

A central `Assemstrct` structure contains the main assembler state, including:

- Symbol table
- Fix list
- Instruction array
- Data array
- Instruction and data counters
- Error state

The symbol table and macro table use hash tables, while linked lists are used where dynamically growing collections are needed, including unresolved references and external-symbol usage.

External symbol references are accumulated during assembly and written to the `.ext` output file after reference resolution is complete.

Errors are tracked through the assembler state. If errors are detected, output files are not generated.

## Output Files

For each valid input file, the assembler may generate:

- `.am` — preprocessed source after macro expansion
- `.obj` — assembled machine code
- `.ent` — entry symbols, when present
- `.ext` — external symbol references, when present

Output files are not generated if assembly errors are detected.

## Performance Experiment

After completing the original assembler, I wanted to test whether the fix-list design actually reduced the work required during reference resolution.

I therefore implemented a comparison second pass that follows the more traditional approach: reopening the preprocessed source file, reading and parsing the instructions again, and locating symbolic operands from the source.

The benchmark compares this source-rescanning implementation with the original fix-list approach.

| Instruction lines | Fix-list second pass | Source rescan | Reduction |
| ----------------: | -------------------: | ------------: | --------: |
|             1,000 |             0.055 ms |      1.452 ms |    96.23% |
|            10,000 |             0.412 ms |     11.606 ms |    96.45% |
|            50,000 |             2.420 ms |     60.541 ms |    96.00% |
|           100,000 |             5.033 ms |    119.451 ms |    95.79% |

In this synthetic workload, the fix-list approach reduced second-pass reference-resolution time by roughly **96%** because it avoids reopening and reparsing the source.

The improvement to the complete first-pass + second-pass pipeline is much smaller because the first pass performs most of the assembler's work. The benchmark is therefore mainly intended to compare the cost of the two reference-resolution approaches.

More details about the benchmark methodology and limitations are available in [`benchmarks/README.md`](benchmarks/README.md).

## Build

Compile the assembler using:

```bash
make
```

Then run the executable with one or more assembly source files:

```bash
./myAssembler example.as
```

To remove compiled object files and the executable:

```bash
make clean
```

## Project Structure

- `myAssembler.c` — program entry point
- `preAssembler.c` — macro preprocessing
- `firstPass.c` — first assembly pass and symbol-table construction
- `secondPass.c` — unresolved-reference patching and output generation
- `file.c` — file-related utilities
- `passes.h` — assembler structures and declarations
- `examples/` — sample assembly source files
- `benchmarks/` — comparison implementation and performance benchmark

## Background

This project was originally developed as a university assignment and later extended with the benchmarking experiment to examine one of the design decisions I made during the original implementation.
