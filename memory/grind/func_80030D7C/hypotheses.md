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
