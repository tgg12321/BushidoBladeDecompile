# func_80037B00 — BLOCKED (heavily cheated: 9 pins + frame dummy)

## TL;DR
String-compare loop over D_80102810 entries (matches arg0 against each 0x15-byte
entry). HEAD "matches" via NINE `register asm()` pins (t1,t3,t2,a3,a1,a2,t0,
v1,v0) PLUS an `s32 sp_dummy[2]` frame-padding cheat. INCOMPLETE. The pins force
BOTH the exact register layout AND the loop scheduling.

## Why blocked (pin-free floor 23 >> HEAD's cheated 15)
candidate.c (no pins, no dummy) measures 23 honest diffs -- WORSE than HEAD's
15 -- because removing the 9 pins triggers a whole-function divergence:
1. **Register-rotation cascade**: t1->t0, a1->v1, a2->a1, t0->t1, etc. -- a
   coupled multi-register rename, not a single swap.
2. **Loop scheduling differs**: the inner-loop `slt/bnez` increment ordering and
   the `addiu a2,a2,1` placement shift (GCC schedules the compare differently).
3. **8-byte frame gap**: target `addiu sp,sp,-8`/`+8`; the DCE'd sp_dummy
   doesn't reproduce it (same class as func_80037AA4 / file_LoadSectors).

## Avenues for next session (change MODALITY)
- This is orchestrator/multi-session grade: a coupled 9-register layout + loop
  scheduling + frame. decomp-permuter from candidate.c is the only realistic
  modality, and even that may plateau (cf. marionation_Exec coupled-rotation
  limit in [[register-alloc-pure-c]]).
- cc1psx calibration on the frame (8-byte gap, same as the file_LoadSectors /
  func_80037AA4 cluster -- possible systematic cc1-vs-cc1psx frame divergence in
  code6cac_c leaf functions; worth a batch calibration of all three).

## Floor
- HEAD: 15 (9 pins + sp_dummy). candidate.c: 23 (cheat-free, worse). NOT lowered.
- Cluster note: func_80037AA4, file_LoadSectors, func_80037B00 ALL show an
  unexplained +8 stack frame the DCE'd dummy can't supply -> a single cc1psx
  calibration run could resolve the frame question for the whole cluster.
