# func_8008B488 — hypotheses (manual s1, 2026-09-25)

## Frontier
The only open item is `u16 rate` shared across the five ADSR blocks (see
evidence.md item 3). Two ways forward:
1. **Owner ruling** (the same question as Ruling 8 / vmNoiseOn: SOTN-verbatim
   reuse of one scratch variable in the same Sony library function). Logged in
   docs/grind/borderline.md 2026-09-25. If allowed, land
   rejected/sotn-shared-rate-ruling5-0.c with the chassis and send it to a fresh
   layer-2.
2. **Another spelling**, starting from candidate.c (12 with the chassis), in which
   the SL block's clamped value gets a1 and SR's rate precedes smode without one
   pseudo spanning several blocks. Not found.

## Killed (measured, chassis in place unless noted)
- non-volatile RXX accesses: lbu narrowing, 34
- volatile without the `adsr &= MASK` split: 38
- SL split as well: 6
- block-scoped rate/mode/adsr/vol locals: 12
- per-block function-scope rate locals, either declaration order: 12
- rate declared first in each block: 12
- SOTN if/else clamp: 10 (392 insns: loads the field twice)
- ternary clamp: 12. s32 per-block rates: 15
- sharing subsets: {DR,SL} 18, {SR,RR} 4 (the rest close only with SL sharing)

## Not tried yet
- permuter campaign from candidate.c. `import.py` cannot take main.c directly:
  it needs the file in the Makefile's dry-run output, so a hand workspace like
  tools/mar_perm_workspace.sh (full-TU compile + function extract) is needed
- instrumented global-alloc dump (tools/gcc-2.7.2/cc1 with BB2_*_DEBUG) to confirm
  the a0-conflict reading in evidence.md item 3
