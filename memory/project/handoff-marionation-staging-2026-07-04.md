---
name: handoff-marionation-staging-2026-07-04
description: Handoff from the 2026-07-03/04 marionation_Exec + cpu_side_move_dir_4 session — owner-sanctioned staging rule, floors 6→4 / 4→2, both residuals mechanism-closed, permuters own the search
metadata:
  type: project
---

# Handoff — marionation_Exec / cpu_side_move_dir_4 (2026-07-03 → 07-04)

## Mission context
Standing user directive: crack `marionation_Exec` (src/system.c:499, **42 regfix
rules** — the most-cheated function in the codebase) to COMPLETED-C, using its
twin `cpu_side_move_dir_4` (src/system.c:388, 5 rules) as the testbed. Both
share the same printf-block codegen problem; a crack on either transfers.

## The session's decisive event: the owner sanction
The layer-1 cheat-reviewer twice FAILed the "stage a value through an existing
dead variable" technique (it strips the scheduler's load-late LAUNCH priority —
sched.c `birthing_insn_p`: multi-set dests never LAUNCH). Surfaced to the owner
with an SOTN census; **owner sanctioned it 2026-07-03** and it is now codified in
**`.claude/rules/staged-value-reused-variable.md`** (plain-language, with bounds:
value real+used, variable exists for a real job, prior value provably dead, full
`/* FAKE: ... */` annotation, dual review unchanged). Indexed in
`codegen-technique-index.md`. Commits: `d9e490c0` (rule), `6999016b` (the
review-compliant state that preceded the ruling).

## Current state (all committed; oracle green, main clean)
| Function | Floor | candidate.c form | Residual |
|---|---|---|---|
| cpu_side_move_dir_4 | **masked 2 / raw 2** (was 4) | h5: `v0 = idx_1494[1]; v0 <<= 2; arg5 = *(s32 *)(v0 + (s32)tbl_125c);` + t0 multi-set chain + pp alias, fully FAKE-annotated | ONE adjacent pair swap `{sll a0 ↔ addu v0}` @54-55 |
| marionation_Exec | **masked 4** (was 6, ten-session floor) | mh5: identical mirror of the twin's form | the SAME pair @56-57 (2) + region-3 (2) |

WIP checkpoints: `memory/wip/cpu_side_move_dir_4/` + `memory/wip/marionation_Exec/`
(notes.md = current-state mechanism maps; meta.json floors updated). Key commits
this session: `172245c3` (floors), `87dec99d` (region-3 map), `b1efad38` (g3 frontier).

## Why the residuals are hard (mechanism-closed, not effort-closed)
All findings instrumented via the env-gated debug cc1 (`tmp/gccdbg`, rebuild:
`make cc1 CFLAGS="-g -fgnu89-inline"`; BB2_QTY/ALLOC/SCHED/FINDREG_DEBUG, -dS/-dd/-dl).

1. **The pair swap** (both fns): both insns carry LAUNCH; luid (statement order)
   forces sll-first. Reversing the statement order hits a qty birth-order tie
   (p106/val5) that breaks registers (6). ~45 forms measured.
2. **The g3 second frontier (twin)**: `t0 <<= 2` late + t0's addu IN the call
   arg **fixes the pair**; residual becomes a pure v1↔a0 exchange — t0's tied
   qty (pri 5000) allocates before arg5 (pri 3333) and steals v1. Flip needs
   arg5-qty refs≥4 via a LOCAL copy-tie; every vehicle measured dead: fresh
   single-set copies fold (cse copy-prop), fn-scope carriers go global/s0-spill,
   v0-carrier scrambles (16), a2v presequence re-times (16-17), 2-ref t0 forms
   hit the equiv-sink (14), declaration scope is invisible (REG_BASIC_BLOCK is
   ref-derived). candidate.c deliberately stays h5 (masked 2 < g3's 6).
3. **Region-3 (marionation only)**: our cc1's dbr steals `move a1,s4` past the
   sb into the beqz-a2 delay slot; target has nop + the move after. C shapes
   that refuse the steal exist (dst2 init inside an `if (a1 != 0)` guard — first
   ever 179/179 forms) but the guard's extra a1-ref lifts the a1-holder's
   allocno pri 930→1176 past the lui-base pair's 933 (a 3-point margin) and
   permutes four s-regs. Counter-levers closed by arithmetic (livelen can't
   extend, the ref can't drop without breaking the matching loop-1, embedded
   assign CSEs back to base).

## What is RUNNING (check before relaunching)
Two decomp-permuter campaigns in WSL (`pgrep -af permuter.py`), both on the
honest splice-compile metric (the stock permuter prunes other fn bodies and its
weighted score diverges — ALWAYS re-verify finds via the engine sandbox; this
session all 4 of its "finds" were honestly worse and were rejected):
- `tmp/perm_csmd4` — twin, **g3 base** (weighted ~30). Relaunch after reboot:
  `python3 tmp/csmd4_perm_base4.py` then from tools/decomp-permuter:
  `python3 permuter.py ../../tmp/perm_csmd4 --best-only --stop-on-zero -j 10`
- `tmp/perm_mar6` — marionation, **mh5 base**. Rebase script: `tmp/mar_perm6_setup.py`.
Finds land in `tmp/perm_*/output-*/source.c`; verify with `tmp/perm_finds_verify.py`.
(tmp/ is gitignored — these scripts regenerate from the WIP notes if lost.)

## Recommended next session
1. **Don't hand-iterate these two further** — the analytic space is measured
   and ledgered (see the "ruled out" sections in both WIP notes; do NOT
   re-derive). The permuters are the correct search now.
2. **Work the queue top** (`func_80045294`, text1a_c.c, distance 2, 0 rules;
   327 active remain) while the permuters grind.
3. On a permuter zero: apply the find, `retire` the rules (5 twin / 42
   marionation), full-build SHA1 gate, layer-1 + layer-2 adversarial review of
   every FAKE construct (the annotations + lever-exhaustion ledgers are already
   in place), `queue done`, delete the WIP dirs.
4. A twin crack transfers to marionation region-1 (candidates are mirrored);
   region-3 then remains as marionation's last gap.
