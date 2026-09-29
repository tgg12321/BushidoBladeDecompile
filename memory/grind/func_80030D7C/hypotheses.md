# Hypothesis ledger — func_80030D7C

## s2 (2026-09-29, manual)
- KILLED: `contact` local set pre-loop (80); scr+0x30 as a compiler temp at both sites (hoisted,
  folded to 0x1F8002E8, spilled — v1 178); shared `flag` local R5/R7; s32 state (55);
  funnel keeping a `state` local (30) or only its tests local (F2 30).
- KILLED (rest a3 cluster): `s16 rest` (24); state+rest computed before the dot product (57, sched1
  keeps it early).
- OPEN: ang -> s4 and rest -> a3 (see evidence s2). Candidates: a single generic local for
  ratan2/collision results and one for amt/rest (Ruling 11 route: needs (D) dumps + search), or a
  one-variable-per-value spelling that makes ang cross a call / rest conflict with a1,a2.
- CONFIRMED (s2): one local for heading+collision result (`temp`, s4) and one for turn
  amount+restitution (`work`, a3) -> 0/709; relaunch needs no local (re-read obj+0x56) -> 0.
  Submitted under Ruling 11 with the (D) record in evidence.md s2. nrm[0..2] in the reflection
  (N2) 0; nrm also in the ny tests (N1) 3.
- KILLED one-variable-per-value spellings: see evidence.md s2 (D)(3)/(4) list.
- s2 layer-2 FAIL #1 (record, not body): per-value probes/dumps did not match the landed body.
  Re-measured on the exact body (onevar_PV 21, PVa 4, PVb 17, PV structural 21-53, campaign C best
  75/no 0). Alias row D_8008E19E removed at landing (prong (c)).
- s2 layer-2 FAIL #2 (prong (A) scope): temp/work moved to the top of the loop body (0/709);
  per-value PV2 re-scoped per (C)(1) (21), ablations 4/17, reviewer proposals R1-R4 21/71/21/24.
