# func_800550E8 — WIP (blocked-lane triage 2026-06-14)

## TL;DR
Byte-array half-decay loop (text1b.c:0x800550e8). Matches ONLY with 3 regfix
substs that re-encode the addressing mode. **cc1 combine-pass single-
displacement fold**: target keeps the 0x415 in the load/store OFFSET; cc1 folds
it into the `la` symbol+addend. Same runtime access, different operand encoding.
NOT closed this pass; the SINGLE displacement gives cc1 no degree of freedom.

## The exact gap (cc1 combine)
- TARGET: `addiu v1,v1,7880` (la D_80101EC8, no addend) ; `addu v1,v0,v1` ;
  `lbu v0,1045(v1)` ; ... ; `sb v0,1045(v1)`. The +0x415 is the lbu/sb OFFSET.
- cc1: `la D_80101EC8+1045` (addend folded) ; `lbu v0,0(reg)` ; `sb v0,0(reg)`.
- 3 reciprocal substs reverse the encoding (la addend strip + 2 offset re-adds).

## Why single-displacement = no lever
Sibling func_8005509C MATCHES because it accesses TWO offsets (0x414 AND 0x415),
which forces cc1's combine to keep the base register separate (it can't fold two
different addends into one symbol). func_800550E8 has ONLY 0x415 — combine always
folds it. There is no honest second displacement to introduce.

## Resume steps (next session)
1. STRUCT-FIELD lever (HIGH value, untried): the 0x44C stride => D_80101EC8 is
   likely an array of a per-entry struct. If a u8 field sits at +0x415, access
   it as `arr[arg0].field` — cc1 emits member-offset addressing (`lbu off(base)`)
   rather than folding the symbol+addend. Needs the recovered struct layout.
2. Read GCC 2.7.2 combine.c symbol+CONST_INT fold; find the predicate the
   two-displacement sibling fails; see if a single-displacement C shape can fail
   it too.
3. If no pure-C form exists (analysis strongly suggests none): surface the
   POLICY question — reciprocal-reencoding regfix (identical bytes, operand-form
   only) acceptable, or parked indefinitely? NOT canonical-asm (hand scan LOW
   0/8); compiler-patch forbidden (no-compiler-divergence).

## Ruled out (do NOT re-derive)
- HEAD body, rules removed -> DIFF (rules load-bearing).
- fold +0x415 into p, access *p -> DIFF (moves offset out of loop; wrong).
- subscript p[0x415] -> DIFF, == HEAD (identical to *(p+0x415)).
- (prior card) uninit dual-pointer trick -> 9; split base+offset -> 12;
  for-loop -> 3 DIFF; step-based p[step+0x415] -> 10.

## Pointers
- `.claude/rules/no-compiler-divergence.md`, `.claude/rules/strength-reduce-defeat.md`
- Sibling that MATCHES: func_8005509C (two displacements 0x414+0x415).
