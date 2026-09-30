# func_80021DB0 — retro-audit CONCERN (owner Q49), analysis 2026-09-30 (laneG) — open, src blocked (laneE)

Archived landing ledger: memory/grind/_completed/func_80021DB0/. Body: src/code6cac_tu2.c (func_80021DB0).
Concern (retro-audit 2026-09-29 batch_01, 287c00642): the loop counter `j` (two 1..40 step loops) is reused as
the chosen stage-record index (`j = D_800A38E0;` / `for (i = 0, j = 0; ...) ... j = i;` / `stage += j * 6 + 3`).
The auditor measured a fresh `sel` at 15/285; the landing claimed "zero FAKE constructs" and no ruling.
Also noticed (not in the concern): the body indexes `(&Judge)[...]` past `extern s16 Judge;` (code6cac_tu2.c:65),
the index-past-scalar idiom that 17e01239b respelled `Judge[]` in text1b.c.

Routes: (1) ordinary spelling with a separate selection variable reaching 0 (search not run); (2) Q51 SOTN
citation of a loop counter reused as a selected index; (3) Ruling 11 package — `j` fails (E) (single letter),
so a rename (e.g. `idx`, if true of every value) plus dumps, annotation and layer-2. Splice waits for laneE.

Measured 2026-09-30 (sandbox --disable all, probes-2026-09-30/, landed body 0/285): a fresh `s32 sel` for the
selected record (three writes + the `stage += sel * 6 + 3` read), declared first / before `i` / after `j` /
after `d`: 15 (285/285) in every position — 0 source-level hunks, 8 operand-only (register seats), 13 masked.
So the reuse is load-bearing only for register allocation: a Ruling 11 (D) dump proof is the natural route.

First dump look (2026-09-30, `cc1 -dl -dg -df`, flags in tmp/func_80021DB0/cmdline.txt; scratch tmp/func_80021DB0/):
reuse: `j` = pseudo 79, 28 refs over 83 insns, crosses 2 calls -> $s1 ($17) for the step loops AND the selected
record index (the target's register). Fresh `sel`: pseudo 80, 6 refs over 27 insns, crosses no call -> $a1 ($5)
(a call-crossing pseudo is confined to call-saved registers; one that crosses none takes a free caller-saved one),
while `j` keeps $s1 — the 8 operand-only hunks. Candidate Ruling 11 (D)(2) mechanism (global.c find_reg over
call-used vs call-saved sets); (D)(3)/(D)(4), the (E) rename (`j` is single-letter), (F) annotation still owed.
