# marionation_Exec — WIP (session-9: saved FIXED masked 4; real-loop breakthrough)

## TL;DR (current state)
**BEST SCORE: `tmp/vDT30.c` (progress/vDT30-8of8-saved-fixed-masked4.c) = 8/8 regs +
andi's + saved-stage fixed, masked 4** (goto-loop + do-while(0) weighting). Only pair-swap
+ region-3 remain. candidate.c untouched, oracle green.

## SESSION-9 BREAKTHROUGH: the andi/loop tension is FALSE (real loop is viable)
Earlier I "proved" region-3 needs a real loop but a real loop breaks the andi's. **WRONG.**
The andi's break under a real loop ONLY because new_var was set INSIDE the loop. **Hoisting
`new_var=0xFF; new_var3=0xFF;` to BEFORE the `while(1)`** makes them set-once/dominating →
update_equiv_regs folds them → **andi's SURVIVE under a real loop** (vDT32, verified: no
`li sN,255`, checks stay `andi ,0xff`). So the goto-loop is NOT mandatory. Consequence:
region-3's dbr steal (which needs the loop note, per [[loop-note-fixes-delay-slot-steal]])
IS fixable in principle — the real `while(1)` gives 179 insns (target count) and s0-s5 come
out correct NATURALLY (no do-while(0) needed for those).
**Real-loop path reached 8/8 regs (vDT45, masked 23) — recipe:** real while(1) + new_var
hoisted + saved widening-temp + **neg1** (`s32 neg1; neg1=-1;` after poll, `while(i!=neg1)`
→ -1 caller-saved `li a3,-1` like tgt) + **drop the do_timeout do-while(0) wrapper** (it
boosts the D_800A11DC base above arg0) + **wrap ONLY the t0/arg5 staging** in do-while(0)
(weights tbl WITHOUT boosting the printf's D_800A11DC base). All 8 main vars land s0-s7.
**The intrinsic real-loop TAX (why masked 23 > vDT30's 4):** the loop note that fixes
region-3 ALSO enables LICM, which hoists the `D_800A11DC` base (2-insn addr) into a 9th
callee-saved reg (s8) → frame +8 → cascades. Target keeps it caller-saved (`lui at,%hi;
lw a2,%lo(at)` per-use) because its 8 vars fill s0-s7 with NO free reg. Ours has s8 free →
LICM hoists. Tried: 5 address-access forms (value-stage/ptr-add/char-cast/idx-var/a2base) —
ALL hoist; it's register-pressure, not access-form. neg1 worked for -1 (1-insn, cheap to
rematerialize) but NOT the 2-insn address. **Can't make GCC leave s8 unused.** So region-3
via real loop costs ~6+ (addr hoist) > the ~2 it saves. **vDT30 (goto, masked 4) stays
best.** Real-loop checkpoints: progress/vDT32-... and vDT45-realloop-8of8-masked23.c.

## The goto-loop recipe (how vDT30/vDT10 works — the masked-4 best)
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

## SESSION-9 UPDATE: saved-stage FIXED → masked 4 (vDT30, 8/8 regs)
`tmp/vDT30.c` (saved: progress/vDT30-8of8-saved-fixed-masked4.c) = vDT10 + the saved-stage
fix. Replace `saved=*D_800A147C_2; saved&=3;` with **`{ s32 _b; _b = *D_800A147C_2; saved =
_b & 3; }`** — a widening temp that de-ties the byte load from saved's callee-saved reg
(byte prefers its def-side v0 reuse instead of coalescing into s1). Now tgt-matches
`lbu v0,0(v0); andi s1,v0`. (Also works: `saved = 3 & *ptr` operand-swap, `explicit_ptr_byte`
— all masked 4. Widening temp is the cleanest/most defensible.) 8/8 regs intact.
Only 2 residuals left: pair-swap + region-3.

## REMAINING (masked 4, 2 residuals — pair-swap + region-3)
NOTE: masked-6-era numbering; saved-stage (was #2) is now FIXED. Remaining:
Each residual has a lever that fixes it — and each lever regresses something already
matched. The tensions are PROVEN, not just unfound (see below). The andi's require the
goto-loop (no LOOP_BEG around checks; target loads are `lbu` so `andi ,0xff` is
redundant-elided unless new_var defeats it via update_equiv_regs, skipped under LOOP_BEG
— checkasm.py confirms tgt `lbu v0,0(s3);andi a2,v0,0xff`). NOTE: goto-loop is NOT
mandatory — with new_var HOISTED a real loop keeps the andi's (session-9 breakthrough);
but the real loop pays the LICM address-hoist tax (see breakthrough section) so vDT30 wins.
1. **do_timeout pair-swap** @56/57: tgt `addu v0,v0,s5`(arg5) then `sll a0`(t0); ours swap.
   ROOT: GCC evals args R→L so arg5(5th) computes first → lower LUID → wins the tie. My
   staging computes t0 first (needed to place D_800A11D5 late + keep t0→a0). vDT23 (natural
   args, pp staged) fixes the pair-swap but mis-places D_800A11D5 → masked 18. arg5-first
   staging (vDT15) flips t0→v1. Pair-swap ⊥ register/D_800A11D5 placement — can't decouple.
   vDT25 (arg5 chain before t0 chain) FIXES the pair-swap ORDER but swaps do_timeout temps
   idx[0]↔arg5 (v1/a0, masked 10): named staging vars are long-lived→v1; tgt's natural temps
   are short-lived→a0. 3-way ⊥ {pair-swap, temps-a0, D_800A11D5-late}; vDT30 gets 2 of 3.
2. **~~saved-stage~~ FIXED** (widening temp, see session-9 update above).
3. **region-3 dbr steal** @149 (178 vs tgt 179): tgt `beqz a2;nop;sb zero,-1(s3);move
   a1,s4`; ours' dbr steals `move a1,s4`(dst2=a1) into the delay slot. ROOT (region3b.py+
   r3sweep.py): dbr steals because physical-a1(=dst2) is DEAD on the beqz-taken(→loop) path.
   Making it live (vR2 merge dst/dst2) DOES give the nop — but the merge just RELOCATES the
   steal to copy-block-1 (W1/mergetune.py: merge + double-nest arg1 restores 8/8 AND keeps
   block-2 nop, but block-1 now steals → masked 9). Emergent physical-a1 liveness at each
   beqz's loop-taken path; can't get both blocks live at once yet.

## Permuter — DEAD END (session-9)
Killed after 1.04M clean-only iters. Best find (output-210) is masked 10 by the honest
sandbox (WORSE than vDT10's 6) — its weighted score diverges (traded a reg penalty for
scheduling). No score-0. The 105 was the pre-restart volatile cheat. The residuals resist
it too.

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
- vDT10 (+ double-nest first clear): 8/8 WITH andi's, masked 6.
- **vDT30 (vDT10 + saved widening-temp): 8/8 + andi's + saved, masked 4** ← CURRENT BEST.
- vDT45 (real loop, 8/8, region-3-fixable): masked 23 (LICM addr-hoist tax). Structure-right.

## DEAD ENDS (don't repeat)
- Full-body do-while(0) breaks andi's. Real while(1) breaks them ONLY if new_var is set
  inside the loop — HOIST new_var before the loop and the andi's survive (breakthrough).
- staged raw byte (raw=*idx_1496; check=raw&nv): folds (raw is a proven byte) → no andi.
- arg1-hold / shared-epilogue tail: didn't drop arg1 cleanly (fixed instead by weighting
  i1496 UP via the clear-wrappers).
- staging reorder (pair-swap): flips printf-arg regs, worse.
