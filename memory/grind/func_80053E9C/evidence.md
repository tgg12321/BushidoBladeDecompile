# func_80053E9C evidence

## 2026-09-24 independent reconstruction

- Canonical gate: `C`, hand-coded tier `LOW`; pure C is required.
- Target: 347 instructions, 36 branches, three calls, and a variable-length
  packed-data walk.
- The archived `pre-include-asm-body.c` is explicitly a placeholder, not prior C.
- Fresh m2c (`mipsel-gcc-c`, valid syntax) produced a 329-instruction body at
  honest score 223 after the minimum pointer/type repairs.
- A separate semantic reconstruction in `candidate.c` reached honest score
  217 with 336 instructions.  The remaining diff has 29 source-level, 20
  operand-only, and six masked cascade hunks.  The dominant unresolved region
  is the signed fixed-point normalization/intersection block; the target has a
  48-byte frame and two real stack spill/reloads that neither reconstruction
  reproduces without inventing storage.
- No candidate was promoted to `src/text1b.c`; main retains `INCLUDE_ASM` as
  required for an incomplete function.

## Measured attempts

1. Natural typed reconstruction from assembly: score 235/347, 338 instructions.
2. Fresh literal m2c reconstruction with type repairs: score 223/347, 329 instructions.
3. Integer cursor plus exact per-read increments and target statement order:
   score 219/347, 332 instructions.
4. Split signed-normalization reads, scoped polygon locals, explicit polygon
   do-loop, and direct result-to-work assignment: score 217/347, 336 instructions.

No FAKE construct, inline assembly, volatile coercion, register pin, fabricated
aggregate storage, or toolchain change was attempted.

## Measurement note (operator review 2026-09-24)

`candidate.c` returns `s32`, but src/text1b.c carries three
`extern void func_80053E9C();` forward declarations (lines ~1531/1566/1597,
used only to store the function's address). The sandbox fails with
"conflicting types" unless those are retyped to `extern s32 func_80053E9C();`
— with that edit the 217/347 floor reproduces. The retype must land together
with the eventual match.
