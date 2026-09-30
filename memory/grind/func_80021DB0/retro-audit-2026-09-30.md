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
