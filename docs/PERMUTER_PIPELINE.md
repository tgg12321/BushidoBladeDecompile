# Permuter vs full-build pipeline divergence

Critical for understanding what `bb2_permuter.py` (and `bb2_retire.py`'s
`min_score=0` verdict) actually guarantees. **The permuter and the
full build are NOT the same.**

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
- It does NOT apply `regfix.txt` rules
- It does NOT apply `asmfix.txt` rules
- It does NOT link with other object files

## What the full build does

`make` runs the build for every `src/*.c` file in turn:

```
cpp -> cc1 -> prologue_fix -> maspsx -> as            (per-file)
   then for each file: apply regfix.txt rules
   then for each file: apply asmfix.txt rules
ld all .o files -> .elf
objcopy -> .bin
make_psexe.py -> .exe
sha1sum .exe vs original
```

### Critical differences

1. **Label numbering is file-scope, not function-scope.** GCC emits
   labels like `.L1`, `.L2`, ... and numbers them in the order they
   appear in the whole file. The permuter compiles a single function
   in isolation, so its label numbering starts fresh. Two implications:

   - The permuter's `target.s` has labels with arbitrary numbers
     (whatever the original disassembly had)
   - The full build's labels for the SAME function may differ when
     other functions in the same `.c` are added/removed/changed in size

2. **Cascade effects across functions.** A C-level change in function A
   in `src/text1b.c` shifts the line count of `text1b.c`. This:

   - Changes GCC's label numbering for ALL subsequent functions in
     `text1b.c`
   - Changes the function's instruction offset within `text1b.o`
   - May invalidate `regfix.txt` rules that target absolute labels
     (`.L<N>`) or instruction offsets in OTHER functions

3. **regfix rules run AFTER cc1+maspsx.** If a function relies on a
   regfix rule to produce its target bytes, removing or breaking the
   rule will cause that function's bytes to differ — **even though
   the function's C source didn't change**. The permuter, which doesn't
   apply regfix, may report a function as "base=0 / match" when in
   reality the regfix is doing essential work.

## Concrete example from this session (`func_8006517C`)

- Permuter said: `match=0` after 25s
- `bb2_apply_match.py` spliced the matched source into `src/text1b.c`
- `make` failed: `undefined reference to .L1077` for `func_80074B18`
- Cause: the matched source's 3-line addition shifted text1b.c's label
  numbering, so `.L1077` (referenced by `func_80074B18`'s regfix rule)
  no longer existed; the new equivalent label was `.L1081`
- `dc.sh fix-label-drift` repaired the reference, but the build STILL
  mismatched
- Cause #2: cascade impact on yet another function whose codegen also
  shifted, and `fix-label-drift` only handles undefined-label cases
- Net: 2-of-2 success rate in PERMUTER, 0-of-2 success rate when applied

## What to trust

| Source | What it verifies | What it does NOT verify |
|---|---|---|
| Permuter `min_score=0` | Function compiles to target bytes IN ISOLATION | The function's match survives splicing back into the full `.c` file |
| `bb2_apply_match.py --verify` | Full make passes AND SHA1 matches | (this IS ground truth — trust this) |
| `bb2_retire.py` end-to-end | Match found AND applied AND SHA1 matches | — (this is ground truth too) |

## Practical implications for agents

1. **Never trust `min_score=0` as a retirement readiness signal.** Always
   complete the apply+verify cycle. The `bb2_retire.py` and
   `bb2_apply_match.py --verify` tools both do this; use them.

2. **High-cascade files are dangerous.** `src/text1b.c` (~17000 lines),
   `src/main.c` (~14000 lines), and `src/code6cac.c` (~9000 lines) are
   the most cascade-prone. ~50% of session-tried matches in these files
   failed to apply due to cascade.

3. **Label-drift auto-repair only handles the simple case.** It fixes
   undefined references to renamed labels. It does NOT handle cases
   where surrounding functions' codegen shifted because the function's
   size changed. For those, you need to manually re-derive the affected
   functions' regfix rules.

4. **A match found by the permuter is a HYPOTHESIS, not a conclusion.**
   Treat the apply+full-build step as the actual test.

5. **`dc.sh verify --all`** is the authoritative function-by-function
   match check post-apply. Run it after committing a retirement to
   confirm no sibling function regressed.

## Future work

Closing this gap fully would require:
- The permuter compiling the FULL `.c` file (slower but accurate)
- OR a "splice + verify" mode that drops candidates back into src/
  before reporting match (slower but accurate)
- OR per-rule cascade detection in `bb2_apply_match.py --verify` that
  identifies which OTHER functions the retirement affects and fixes
  their regfix rules in lockstep

For now, the apply tool's auto-fix-label-drift handles the most common
case (~70% of cascade issues based on inspection of session data). The
remaining ~30% require manual investigation.
