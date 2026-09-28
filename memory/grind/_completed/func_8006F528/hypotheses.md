# func_8006F528 — hypotheses (closed 2026-09-23, manual session)

Start: interrupted Codex draft left uncommitted in src/text1b.c at 45/277.

CONFIRMED (each measured with `sandbox --disable all`, final form = 0/277):
- H1 `s16 state = D_800A3578 & 0xFF` — reproduces target `lbu v0; move s2,v0; slti v0,v0,3`
  (load pseudo copied into the call-surviving var). s32 state 44, u8/u16 38, `*(u8 *)&` read 36. 45 -> 25.
- H2 function-scope `p1` reused per block — target homes it in a1 across each branch. 25 -> 17.
- H3 block-scoped `base` per block — target puts each block's base in a different register (separate pseudos). 17 -> 5.
  (A shared function-scope base: 12. Direct `ctx[N]` re-reads: 25/45.)
- H4 switch: the first store goes through the `offset` local loaded before the branch; after that, the code
  re-reads the global `D_800A35C4` (target reloads it after the store). 5 -> 2.
- H5 `SetDrawArea(arg0[7], &rect)` directly instead of through a `prim` alias: 2 -> 0.

Ablations on final form: no_p1 19, one_base 12, no_offset 7, no_x 13, s32_state 19, u8read 9, inline ot 4.

## Layer-2 round 1 (2026-09-23): FAIL
Fresh function-scope `s32 *p1` written 5x = banned fresh multi-write carrier (y1 lineage,
decisions.md:1738/6529/10630). Reviewer ablation: single-write `p1` per block = 14/277.
Body banked in rejected/fn-scope-p1-carrier.c. All other constructs reviewed OK.

## H6 (round 2): staged-value-reused-variable
Stage `base + 0xC` through the existing `offset` (u8*, real job: the switch's D_800A35C4
draw-offset pointer; dead at every staging point). 0/277, SHA1 match. FAKE-annotated at every site.
Natural spellings measured first: single-write per-block p1 14, inline into s.p1 19,
shared base 12, read back from s.p0 5 (with fn-scope p1), direct ctx[N] 25/45.

## Layer-2 round 2 (2026-09-23): FAIL
Borrowing `offset` needed its declaration hoisted out of the switch case = invented borrow
(func_800460E4 declaration-hoist FAIL; decisions.md:6525). Body in rejected/offset-hoist-borrow-carrier.c.

## Frontier (carrier-free), 14/277 = candidate.c
Diagnosis: target homes every block's base+0xC in a1, which only a single pseudo spanning several
blocks produces (global.c conflicts v0/v1/a0 give a1). Per-block pseudos get v0/v1 from local-alloc.
Measured, didn't close: single-write p1 per block 14, `&base[3]` with s32 *base 14, s.p1 stored
early 26, read back from s.p0 26, do-while(0) around the store 29.
Open: record-typed ctx[N] with &rec->member; permuter from candidate.c; BB2_ALLOC_DEBUG dump.

## Allocation dump (2026-09-23, cc1 -da, tmp/f528_alloc)
Reuse form (tmp/f528_alloc/r.c.greg): p1 = pseudo 76, global.c, `;; 76 conflicts: 72 73 75 76 2 3 4 29` -> hard reg 5 (a1),
matching target. Its union of five block ranges conflicts with v0 (call results / state compare),
v1 (block-1 base) and a0 (&s). Per-block form: each p1 lives in one basic block, so local-alloc takes
it and gives v0/v1 (not in the .greg conflict table). No carrier-free spelling with a per-block local
can reach a1, because local-alloc picks the lowest free register.
Permuter: tmp/perm_f528 (single-function workspace, validated: reuse body 0 diff, candidate 277 insns),
campaign label carrier-free-14, launched 2026-09-23.

## Permuter campaign 1 (2026-09-23, label carrier-free-14): did not close
From candidate.c (per-block p1, 14/277). 536 s, 17,394 iterations, 35 finds, best permuter score 640
vs base 890. Best finds lower the score only by rearranging statements or borrowing `prim` via
`s.p0 = (prim = (void *)base);` (a cheat-class borrow, rejected). None reaches 0. A fresh-seed campaign 2
(label carrier-free-14-seed2) was launched to meet the ~20-30 min fresh-seed stopping rule.

## Record-typed ctx[N] respelling (2026-09-23): ruled out
`typedef struct { s32 hdr[3]; s32 data[1]; } Rec; Rec *rec = (Rec *)ctx[N]; s.p0 = rec;`:
per-block `p1 = rec->data` 14/277; `s.p1 = rec->data` directly 19/275; function-scope p1 0/277.
Same result as `base + 0xC`: the type doesn't matter; only whether one pseudo spans the blocks.

## Permuter campaign 2 (2026-09-23, label carrier-free-14-seed2): did not close
Fresh seed from candidate.c. 1,414 s (~24 min, meets the 20-30 min fresh-seed stopping rule),
46,286 iterations, 57 finds, best permuter score 635 vs base 890. Best finds keep per-block `p1`
and only reorder other statements; none reaches 0. Combined with campaign 1 (17,394 iterations,
best 640): the carrier-free space does not close, consistent with the allocation dump.

## Ruling 5 (commit 4b95070ff) adopted for the reuse form
Prong-4 receipts: per-write spelling 14/277; structural respellings (record type 14/19, readback
26, early store 26, do-while 29, &base[3] 14); permuter x2 above; allocation dump above.

## CLOSED 2026-09-23: COMPLETED-C
Layer-2 round 3 PASS under Ruling 5 (all prongs verified; bytes reproduced by the reviewer).
candidate.c = the committed body; per-write-14.c = the 14/277 receipt.
