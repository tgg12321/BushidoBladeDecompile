# marionation_Exec — WIP (2026-07-05 session-8: 8/8 REGS + andi's, masked 6)

## TL;DR (current state)
`tmp/vDT10.c` (saved: progress/vDT10-8of8-WITH-andi-masked6.c) = **ALL 8 callee-saved
regs match target AND both check `andi ,0xff` masks present, masked 6** (178 vs 179
insns). The full register rotation (the 10-session wall) is SOLVED with the masks
intact. candidate.c untouched, oracle green. Only 3 scheduling residuals remain.

## THE SOLUTION recipe (how vDT10 works)
GCC 2.7.2 frequency-weights refs only inside `NOTE_INSN_LOOP_BEG` loops (do/while/for,
NOT goto-loops). The candidate's outer `goto loop` leaves the body weight-1, so the
register order is wrong. Fix = candidate body + LOCAL `do{}while(0)` wrappers, each
weighting the pseudos used in the wrapped region, WITHOUT wrapping the check masks
(so `new_var` folds late → the andi's survive — a full-body do-while(0) breaks them):
- `do{}while(0)` around **do_timeout** → weights tbl (→ s5).
- `do{}while(0)` around the **poll block** (GetVblank..*D_800A147C_2=saved) → i1494/i1495.
- `do{}while(0)` around **each idx_1496 clear** (`*idx_1496=0`, `*(idx_1496-1)=0`) →
  weights i1496 above arg1. The FIRST clear is DOUBLE-nested
  (`do{do{*idx_1496=0;}while(0);}while(0)`) → i1496 pri 1600 (> arg1 952).
- new_var/new_var3 masks + the copy blocks stay OUTSIDE all wrappers.
do-while(0) adds NO runtime instructions (control-flow only) — pure RA/scheduling
weighting. **SOTN-sanctioned** (2026-07-05 research: SOTN uses do-while(0) as a
mechanism-agnostic match device — empty bodies at fn entry / straight-line prove it's
not delay-slot-only; grouped with temp-vars/self-assigns). BB2's old "delay-slot only"
narrowing in do-while-zero-exception.md was over-conservative.

## REMAINING (masked 6, 3 scheduling residuals — all on a CORRECT reg base)
1. **do_timeout pair-swap** @56/57: target `addu v0,v0,s5` then `sll a0`; ours swapped.
   Staging reorder (arg5 before t0, vDT12) flips printf-arg regs → WORSE. Leave to permuter.
2. **saved-stage**: target `lbu v0,0(v0); andi s1,v0`; ours `lbu s1,0(v0); andi s1,s1`.
   GCC coalesces the load into saved's reg (v0 dead-after). u8/s32/ptr temps + single-
   expr all fail (savedbatch.py). Coalescing decision — needs the permuter.
3. **region-3 dbr steal** (1 missing insn → 178 not 179): target `beqz a2; nop; sb
   zero,-1(s3)`; ours' dbr steals the sb into the beqz delay slot. Goto-loop → the beqz
   isn't a recognized loop-exit (see loop-note-fixes-delay-slot-steal); the clear's
   do-while(0) didn't stop it. A real outer loop would but breaks the andi's.

## Permuter (background)
On the vDT10 base (tmp/perm_mar, main PID in campaign.pid): base score 230, converging
(→220), `--stop-on-zero`. The 3 residuals are exactly its strength. Finds:
tmp/perm_mar/output-<score>-N/source.c — PROPOSALS, cheat-vet before use.

## Endgame
On masked 0 → full-build SHA1 == oracle → **dual cheat-review** (layer-1 in-session +
layer-2 fresh cheat-reviewer) of: the nested do-while(0) RA-weighting (SOTN-sanctioned,
cite the research) + the sanctioned printf staging + saved split → retire the 42 rules
+ the register pin → `queue done` → delete WIP.

## Tooling (tmp/, worktree bb2-work-marion; regen from this note if lost)
- tmp/regmatch.py <c> = reg→value map + N/8 rotation-match (clean rotation fitness).
- tmp/adiff.py <c> = LCS diff w/ branch-addr normalization. tmp/probe.py = score+ledger.
- tmp/mar.sh = WSL wrapper (`wsl bash <abs>/tmp/mar.sh tmp/<tool>.py <args>`).
- tmp/gccdbg/cc1 = instrumented cc1 (verified == canonical). m2c at tools/m2c
  (tmp/run_m2c.sh). Progress bodies in progress/.

## Variant ladder (regmatch / masked)
- candidate.c: 3/8, masked 30 (honest baseline, goto-loop, andi's present).
- vDW7 (full do-while(0)): 8/8 but NO andi's (masks materialize under LOOP_BEG). DEAD.
- vDT2 (local wrappers, do_timeout+poll): 6/8 WITH andi's (bottom-3 fixed).
- vDT8 (+ wrap both clears): 6/8, i1496 → 933 (arg1 952, gap 19).
- **vDT10 (+ double-nest first clear): 8/8 WITH andi's, masked 6** ← current best.

## DEAD ENDS (don't repeat)
- Full/partial-top do-while(0), while(1), real outer loop: break the check andi's
  (new_var materializes or cse-folds early under any LOOP_BEG touching the checks).
- staged raw byte (raw=*idx_1496; check=raw&nv): folds (raw is a proven byte) → no andi.
- arg1-hold / shared-epilogue tail: didn't drop arg1 cleanly (fixed instead by weighting
  i1496 UP via the clear-wrappers).
- staging reorder (pair-swap): flips printf-arg regs, worse.
