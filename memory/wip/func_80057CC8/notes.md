# func_80057CC8 — WIP checkpoint

## TL;DR (2026-06-14)
Masked floor **9 -> 3** by reassociating BOTH `p`-pointer adds from
`(table + offset)` to `(offset + table)`. That is the whole win and it is
clean pure C (integer add reassociation). The remaining 3-insn gap is a
register-COALESCING decision the masked sandbox cannot drive below 3, and the
function additionally needs its `asm("s3")` pin and all 7 regfix-subst rules
to drop for COMPLETED-C. Blocked as a register-allocation plateau.

## Resume steps
1. Apply `candidate.c` to `src/text1b.c` (replaces func_80057CC8, ~line 11840).
2. Confirm floor: `tools/eng.ps1 sandbox func_80057CC8 --disable all` -> score 3.
3. Iterate on the coalescing gap below; do NOT re-derive the rejected forms.

## The remaining gap (the 3)
Target first `p`:  `addu v0,v0,a2` (offset+table, dest = the offset reg v0),
then `lh a0,0(v0)` / `lh a1,2(v0)`.
Ours:              `addu v1,v0,a2` (allocates a NEW reg v1), `lh ...0(v1)` etc.
=> GCC does not coalesce the offset temp into the pointer dest for the FIRST p.
The SECOND p coalesces correctly (v1->v1). The only structural asymmetry is
the `if ((s16)prev_idx < 0) { prev_idx = arg0[3]-1; }` reload: prev_idx has a
two-def (phi) live range, next_idx (set in a block) has a single def.

Also masked-out but real for the full SHA1: target holds prev_idx in `a0` and
table in `a2`; ours has them swapped. And `next_idx` must land in `s3`
naturally (the pin currently forces it).

## Ruled out (all measured this session; none a cheat, all >= baseline-3)
- explicit `s32 off`/`sidx` local reused for both p adds — floor 4
- ternary / single-def prev_idx with explicit sidx local — floor 5
- pointer arithmetic `table + (sidx<<1)` — floor 9
- prev_idx declared `s32` with `(u16)` casts — floor 8
- removing the `asm("s3")` pin — masked floor unchanged at 3 (masking is
  blind to the pin; only full SHA1 can judge it)

## Live hypotheses (see meta.json next_hypotheses)
- Targeted permuter from candidate.c (CAVEAT: text1b.c is the highest-cascade
  file; ~50% apply-failure rate — PERMUTER_PIPELINE.md, permuter-closability).
- `-da` / ALLOCDBG dump to see why first-p offset temp is not coalesced.
- Force prev_idx to a single def so its live range matches next_idx's.

## Why blocked (not matched)
COMPLETED-C needs 0 rules + 0 pins + SHA1==oracle. The 7 regfix-subst rules
rename exactly the registers this gap is about (a0<->a2, dest v0<->v1) and the
s3 pin forces next_idx; the explored pure-C forms do not make GCC produce that
allocation. Genuine register-allocation plateau — needs a deeper coalescing
lever or a (cascade-risky) permuter pass.
