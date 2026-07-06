# marionation_Exec — WIP (2026-07-05 session-8: REGISTER WALL BROKEN, masked 30→8)

## TL;DR (read first)
The 10-session register-rotation wall is **SOLVED**. `tmp/vDW7.c` (saved:
progress/vDW7-masked8-8of8regs.c) gets **ALL 8 callee-saved regs matching target**
(regmatch 8/8) at **masked 8** — 20 below the old honest floor. 3 instructions short
of a byte match. Two things remain: the 2 elided `andi` masks, and a HONESTY ruling
on the do-while(0) weighting (below). candidate.c is still the old honest baseline
(untouched, oracle green) until the do-while(0) question is settled.

## THE BREAKTHROUGH — how the rotation was solved
GCC 2.7.2 frequency-weights refs only inside `NOTE_INSN_LOOP_BEG` loops (structured
do/while/for), NOT goto-loops. The candidate's outer `goto loop` leaves the body
weight-1, so tbl/i1496/arg1 stay too low and arg0 too high. Fix (vDW3/6/7):
- Wrap the loop BODY in `do { ... } while (0)` → emits LOOP_BEG → weights the body
  → i1494>i1496>arg1>tbl>i1495 come out correct.
- Keep the `if (a0) return; goto loop` arg0 test OUTSIDE the do-while(0) (weight-1)
  → arg0 lands dead-last at s7. Ledger now = exact target priority order.
- Use LITERAL `& 0xFF`/`& 3` masks, NOT `new_var` (int) — new_var MATERIALIZES to a
  saved reg under do-while(0) (update_equiv_regs skips the fold under LOOP_BEG),
  displacing the rotation (vDW5/8 → 5/8, frame grows to s8).
- Keep the sanctioned printf staging + saved-stage (via v0).

## REMAINING GAP (masked 8, 176 vs 179 insns) — all scheduling/codegen
1. **do_timeout pair-swap** (1 diff): target `addu v0,v0,s5` @56 then `sll a0` @57;
   ours swapped. Reordering the staging (vDW12) flips the printf arg regs — worse.
   Leave to permuter.
2. **Two check `andi rX,rY,0xff`** (the 3 missing insns = 2 andi + 1 load-delay nop):
   target `lbu v0; andi a2,v0,0xff`. VERIFIED candidate.c emits these via `& new_var`
   (int var → generates the & then folds to immediate 0xff). But new_var MATERIALIZES
   under do-while(0) (can't coexist). Literal `& 0xFF` folds → andi elided. volatile
   load, v0-staging, preamble new_var — all fail. This is the crux of the last 3 insns.

## THE HONESTY QUESTION (needs user ruling — gates acceptance)
The `do{}while(0)` here is used for its LOOP_BEG **RA-weighting** side-effect; the copy
blocks `return` (not `break`), so it has no local break-semantic — the "no semantic
purpose, only codegen" cheat signature. BUT do-while(0) is a sanctioned construct
([[do-while-zero-exception]], SOTN 18+), and the target's RA appears reproducible ONLY
with body-weight + arg0-outside == exactly this structure, arguing the ORIGINAL had it
(faithful). The dedicated rule sanctions do-while(0) for the reorg.c delay-slot effect,
"NOT a precedent for other wrappers" — so THIS use (RA weighting) is a new application
needing user sign-off + cheat-review. Until ruled, vDW7 is NOT committed as the match.

## Permuter campaign (background)
Running on the vDW7 base (tmp/perm_mar, main PID in campaign.pid): base score 430,
converging (→385). `--stop-on-zero`. Finds in tmp/perm_mar/output-<score>-N/source.c
— PROPOSALS to cheat-vet. Targeting the andi + pair-swap.

## Tooling (tmp/, worktree bb2-work-marion)
- tmp/regmatch.py <c> = reg→value mapping vs target + N/8 rotation-match count (the
  clean rotation fitness, independent of scheduling). tmp/adiff.py <c> = LCS diff w/
  branch-address normalization. tmp/probe.py = score+ledger. tmp/mar.sh = WSL wrapper.
- tmp/gccdbg/cc1 = instrumented cc1 (verified == canonical). m2c junctioned at tools/m2c
  (tmp/run_m2c.sh). Progress bodies in memory/wip/marionation_Exec/progress/.

## Variant ladder (regmatch / masked)
- candidate.c: 3/8, masked 30 (honest baseline, goto-loop).
- vDW2 (structured do-while(a0==0)): 5/8, masked 49 (arg0 over-weighted by while-cond).
- vDW3 (do-while(0) body + goto arg0, literal masks): 8/8, masked 34.
- vDW6 (candidate body + literal masks + do-while(0)): 8/8, masked 10.
- vDW7 (vDW6 + saved-stage): 8/8, **masked 8** ← current best.
- vDW5/8 (new_var + do-while(0)): 5/8 — new_var materializes. DEAD.

## Next
1. Close the 2 andi's: permuter, OR find a non-new_var andi that survives do-while(0).
2. Fix pair-swap (permuter).
3. On masked 0 → full SHA1 gate → cheat-reviewer (esp. the do-while(0) use) → user ruling.
