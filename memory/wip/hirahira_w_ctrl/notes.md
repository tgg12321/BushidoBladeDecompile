# hirahira_w_ctrl — WIP (opened 2026-08-04)

`src/text1a.c:753`. **16 regfix rules + 1 `tools/prologue_config.json` entry +
a register pin in source** (`register s32 *s7_a4 asm("s7") = a4;`) at HEAD.
No asmfix / frame_fix entries.

## Where it stands

| | score | frame | insns |
|---|---|---|---|
| committed HEAD | 27 | 72 | 132 |
| r1-r2 candidate | 23 | 72 | 132 |
| **`candidate.c` (r3-r16)** | **5** | 72 | 132 |
| target | 0 | 72 (`0x48`) | 132 |

Frame, instruction count, the prologue, the whole of loop 1, the schedule and
the epilogue all match target exactly. `candidate.c` is pure C — no register
pin, no `__asm__`, no volatile, no dead code.

**CORRECTION (operator, 2026-08-04 ~08:20) still stands: the pin retirement is
NOT a free commit.** It is inert under the honest sandbox but with the 16 regfix
rules ENABLED the emission shifts and the full build MISMATCHES (measured: SHA1
8e1160d1). The pin retires only as part of the COMPLETION, when all rules go.

## The residual: five instructions, one cluster

```
  72  TGT addiu s3,s7,32     OURS addiu s6,s7,32
  99  TGT move  a1,s3        OURS move  a1,s6
 110  TGT move  a0,s7        OURS move  a1,s6
 111  TGT move  a1,s3        OURS move  a2,s8
 112  TGT move  a2,s8        OURS lw    a0,88(sp)
```

Two causes: (a) target gives loop 2 its **own** `a4 + 0x20` register — `s3`,
shared with the by-then-dead loop1 `stptr` — where we reuse loop 1's `s6`;
(b) the parameter substitution that bought 17→11 costs a stack reload at
loop2's `func_800523E0` where target has `move a0,s7`.

## Method — ALLOCDBG priority arithmetic drove all of it

`pri = floor_log2(nrefs) * nrefs / livelen * 10000` (re-verified here). Tools:
`tmp/allocone.sh` (isolates the function into its own TU so there is no
block-alignment guesswork — allocno numbering, nrefs, livelen and pri are all
per-function, so an isolated compile gives the identical table), plus
`tmp/allocpick2.py` / `tmp/allocmatch.py` for the in-context log.

Allocno table of the 5-point form (all eight callee-saved registers now match
target):

| reg | role | nrefs | livelen | pri |
|---|---|---|---|---|
| s1 | a1 | 16 | 100 | 6400 |
| s2 | a2 | 16 | 100 | 6400 |
| s3 | stptr (loop1) | 7 | 41 | 3414 |
| s0 | stptr2 (loop2) | 6 | 47 | 2553 |
| s4 | i | 8 | 98 | 2448 |
| s5 | tbl | 4 | 48 | 1666 |
| s6 | out2 | 6 | 90 | 1333 |
| s7 | a4 carrier | 5 | 95 | 1052 |
| s8 | a3 | 4 | 100 | 800 |

## The six levers (27→5) — narrative in git history (r16 commit 44ab9f6e)
## Measured negatives — in git history (r16 commit); do not re-run the operand-order, split-init(+2), or alias(0) sweeps
## Resume here — the number to beat

Start from `candidate.c` (5). The task is a two-line spec:

> Give loop 2 its own `a4 + 0x20` allocno with **pri in (1052, 1666)** — at
> livelen 42 that needs nrefs 4, and nrefs 4 is only reachable together with a
> live range of 48–76 — **while restoring `move a0,s7`** at loop2's
> `func_800523E0` (i.e. finding a different way to hold the carrier's priority
> below `out2`'s 1333 than the parameter substitution).

Alternatively raise `tbl` from 1666 to ~2083 (nrefs 4 → 5) so that a split
`out2` at nrefs 4 / livelen 42 (1904) lands between `tbl` and `a4`. Both routes
need a **+1** reference lever; every one found so far moves +2 (split-init) or 0
(alias, copy-propagated). That is the open question.

The directed permuter is the untried rung — 132/132 at offset 0 with only five
instructions differing is a much better starting basin than saTan4FireDisp had.
Per [[grinder-permuter-orphan-stop-gate]] it must be supervised and harvested
inside the same session.

Instruments: tmp/hw*.py sweeps, tmp/hw_bank.py, tmp/sbs.sh, tmp/frame_probe.sh, tmp/allocdbg.sh, tmp/allocpick.py, tmp/perm_hw workspace.

## Permuter rung — CLOSED (62k iters, nothing beats candidate-5; details in git history)
## RA-solver session (2026-08-04 afternoon, tools/ra_solver)

Candidate-5 model validates 11/11. Solver findings:
- The 5-insn cluster = out2 (pseudo 86) CONFLICTS with stptr (87) in our
  spelling (one variable reused across both loops = one pseudo). Removing
  the conflict in the model gives out2->s3(19) = target.
- Real split spelling (out2b for loop2): out2b DOES land s3 (model 12/12
  confirmed) but re-shuffles {loop1-out, a4, saved} into a 3-cycle;
  sandbox 16 (worse than 5).
- Solver on the split model, spec {87:19, 86:22, 77:23, 75:30}: NO single
  atom; sufficient PAIRS include (86 refs+1 & 86 pref+22), (79 refs+1 &
  86 refs+1), (77 pref->23 & 86 live-8), etc.
- Tested spelling for 86 refs+1 (split-init the preamble def, sanctioned
  family): sandbox still 16 — the +1 did not materialize as modeled.
  NEXT: extract the split2 variant model (tmp/hw_split2.sh --keep, then
  extract) and compare 86 actual nrefs/pri vs predicted; then realize the
  next pair from the HIT list. Scripts: tmp/hw_solver.sh, tmp/hw_split*.sh.
- The candidate-5 form in candidate.c remains the best (apply as base).
