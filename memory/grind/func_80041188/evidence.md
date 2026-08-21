# Evidence bank — func_80041188

- [s1 recon 2026-08-21] Chassis re-verified on current main: canonical verdict C
  (132 insns, distance 27); HEAD honest floor 27; candidate.c applied to
  src/text1a_pre.c (function name kept func_80041188, body otherwise verbatim)
  scores **5** — the wip-import floor is chassis-valid, no re-measurement of
  banked spelling conclusions needed. Residual fdiff re-run and byte-identical
  to the recorded 5-insn loop-2 cluster (artifact:
  tmp/grind/func_80041188/s1/fdiff_residual.txt). The candidate edits are IN
  PLACE in src/text1a_pre.c at session end.
- [s1 recon] Sibling scan: no hit in tmp/duplicates_leads.txt; the committed
  source's own annotation says "kengo:MED | my_hirahira/hirahira_w_ctrl | 132i
  | x2 size collision" — a Kengo name lead only, no transplantable body.
- [s1 recon] Untried sanctioned lever noticed for the open "+1 reference"
  problem: duplicated-statement-into-arms (the proven byte-free reg_n_refs
  lift for global-RA priority walls, per codegen-technique-index). Never
  attempted on this function per the ledger. See hypotheses.md H2.

- WIP rejected_form: {'form': 'split `out2` into per-loop locals (target demonstrably has two a4+0x20 registers)', 'score': 17, 'reason': 'each half scores nrefs 3 / livelen 42 -> pri 714, BELOW a3 (800), so it takes s8. At livelen 42 the reachable priorities are 714 (nrefs 3) and 1904 (nrefs 4); the window that would work is (1052, 1666) and nothing lands inside.'}

- WIP rejected_form: {'form': 'split-init accumulation on out2 (`out2 = (s32 *) pa4; out2 = ... + 0x20;`, and the +0x10/+0x10 staged spelling)', 'score': 17, 'reason': "copy-propagated away before flow analysis, so unlike stptr it adds NO references to the loop1 copy. It does take on loop2's copy (nrefs 5, pri 2040 -> s3), but loop1's stays at 714."}

- WIP rejected_form: {'form': 'def-position sweeps: all 6 orders of the three pre-loop initialisers x 3 positions of the loop2 init', 'score': 18, 'reason': "out2's live length only moves 88-91 (the window at nrefs 6 is [71,81]); stptr1's live length floors at 41. Exhausted - do not re-run."}

- WIP rejected_form: {'form': 'drop the a4 carrier entirely (use the parameter everywhere)', 'score': 21, 'reason': 'the parameter pseudo then has livelen 190 (pri 736) and sinks below a3 (808)'}

- WIP rejected_form: {'form': "parameter substitution at any a4 site other than loop2's func_800523E0", 'score': 42, 'reason': 'costs an instruction (133 or 134)'}

- WIP rejected_form: {'form': 'inline `a4 + 0x20` at the call sites (per-loop LICM hoist)', 'score': 25, 'reason': 'the hoisted pseudo scores nrefs 4 / livelen 20 -> pri 4000, far above the (1052,1666) window'}

- WIP rejected_form: {'form': 'u8 * retypings of the carrier / out2 / stptr; extra aliasing locals for out2 and a3', 'score': 17, 'reason': 'inert or worse - alias copies are propagated away before flow, so they never reach reg_n_refs'}

- WIP rejected_form: {'form': 'plain operand-order swaps without a cast (offset + a1 / offset + a2 / both)', 'score': 6, 'reason': 'inert; only the explicit (s32) cast moved the first add'}

- == imported from memory/wip notes.md ==
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


- [s1] HEAD honest floor 27 (canonical verdict C, pure-C target, 132 insns); candidate floor 5 with edits in place in src/text1a_pre.c

- [s1] Residual byte-identical to the recorded cluster: loop2's a4+0x20 needs its own allocno in s3 (pri window (1052,1666) at livelen ~42 => nrefs 4) and move a0,s7 must return at loop2's func_800523E0 (currently lw a0,88(sp) from the lever-4 parameter substitution)

- [s1] Open problem unchanged: a +1 reg_n_refs lever — every spelling banked so far moves +2 (split-init) or 0 (alias, copy-propagated); RA-solver sufficient PAIRS banked in evidence.md

- [s1] Untried sanctioned family identified for the +1: duplicated-statement-into-arms (byte-free ref-lift for global-RA priority walls) — hypotheses.md H2, rule must be read end-to-end before use

- [s1] No sibling/duplicate shortcut exists (tmp/duplicates_leads.txt negative; Kengo lead is name-only)
