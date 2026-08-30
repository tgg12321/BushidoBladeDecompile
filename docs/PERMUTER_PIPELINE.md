# Permuter vs full-build pipeline divergence

Critical for understanding what the permuter's `min_score=0` verdict
actually guarantees. **The permuter and the full build are NOT the same.**

> **Tooling note:** the cheat-invisible **`sandbox <func> --disable all`**
> scores a function in isolation, while only the full-build SHA1 gate
> (**`verify-oracle` / `make`**) is ground truth. (Historical tool names
> from the retired `dc.sh` era map onto those two.)

## What the permuter does

`permuter/<func>/compile.sh` compiles a single C source file (the
function under test) and assembles to `output.o`. Steps:

```
mipsel-linux-gnu-cpp  ->  cc1  ->  prologue_fix  ->  maspsx  ->  as
```

Then the permuter's runner compares `output.o` byte-for-byte against
`permuter/<func>/target.o` (extracted from `asm/funcs/<func>.s` at
setup time).

### What the permuter pipeline does NOT do

- It compiles ONE function in isolation, NOT the full `.c` file
- It does NOT link with other object files

## What the full build does

`make` runs the build for every `src/*.c` file in turn:

```
cpp -> cc1 -> prologue_fix -> maspsx -> multu_pad -> as    (per-file)
ld all .o files -> .elf
objcopy -> .bin
make_psexe.py -> .exe
sha1sum .exe vs original
```

### Critical differences

1. **Label numbering is file-scope, not function-scope.** GCC emits
   labels like `.L1`, `.L2`, ... and numbers them in the order they
   appear in the whole file. The permuter compiles a single function
   in isolation, so its label numbering starts fresh. The full build's
   labels for the SAME function may differ when other functions in the
   same `.c` are added/removed/changed in size.

2. **Cascade effects across functions.** A C-level change in function A
   in `src/text1b.c` shifts GCC's label numbering and instruction
   offsets for ALL subsequent functions in `text1b.c`. Anything that
   references absolute `.L<N>` labels across functions (e.g. a
   whole-body asm block naming a sibling's label) can silently break.

3. **The permuter's `target.o` is only as good as its extraction.**
   A whole-file `target.o` puts the function at a non-zero offset and
   drowns the diff signal — build the target from `asm/funcs/<func>.s`
   so the function sits at offset 0 (see [[difficult-is-not-impossible]]).

## What to trust

| Source | What it verifies | What it does NOT verify |
|---|---|---|
| Permuter `min_score=0` / `sandbox --disable all` == 0 | Function compiles to target bytes IN ISOLATION | The function's match survives in the full `.c` file + link |
| `verify-oracle --rebuild` (full-build SHA1) | Full make passes AND SHA1 == oracle | (this IS ground truth — trust this) |

## Practical implications for agents

1. **Never trust an isolated `min_score=0` / `sandbox == 0` as a
   completion signal.** Always complete the full-build verify cycle
   (`verify-oracle --rebuild`) before `queue done`.

2. **High-cascade files are dangerous.** The large TUs (`text1b.c`,
   `main.c`, `code6cac.c`) are the most cascade-prone — always
   re-verify the full build after landing a match in one.

3. **A match found by the permuter is a HYPOTHESIS, not a conclusion.**
   Treat the apply+full-build step as the actual test, and vet the
   closing form against the cheat catalog before committing
   ([[no-new-park-categories]]).
