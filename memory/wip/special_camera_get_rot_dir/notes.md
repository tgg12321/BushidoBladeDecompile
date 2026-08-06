# special_camera_get_rot_dir (code6cac_b2_post.c) — WIP

## TL;DR (2026-08-06)
Honest floor **9** at the correct **72** insns. On main the function still carries
**4 `register asm("sN")` pins** (s2/s3/s4/s5) — the queue park_reason's "NO cheat
on main" is WRONG; the pins are cheat-asm and this function is INCOMPLETE.
The inverse solver's LEVER verdict is **not C-reachable** (see below); the
residual is an allocno-priority ordering the modelled inputs cannot produce.

## Correction to earlier notes
The 2026-06-14 claim "the masked sandbox is BLIND here" is **false**. Measured
2026-08-06: the sandbox is a real gradient — statement-order variants of the
pin-free body score 9 / 10 / 12 / 13 / 14 / 21. Iterate against it freely.

## What target wants vs what we get
Target: dest→s0, buf2_ptr→s1, index→s2, cam_base→s3, constant_80→s4, copy_end→s5.
Best reachable pure C (measured): dest→s0, buf2_ptr→s1, index→s2, constant_80→s4
correct; **cam_base and copy_end swapped** (we get cam_base→s5, copy_end→s3).

## Measured this session (first-hand)
- **All 120 permutations** of the five independent initialisers compiled with cc1
  and the landing register of each value parsed from the asm. Best family = 4/6
  registers correct; **no ordering reaches 6/6**. Family members (copy_end first):
  CADBE CDABE CDBAE CDBEA DBCAE DBCEA DBECA DCABE DCBAE DCBEA.
- Sandbox scores for that family: `DBCAE` = **9 @ 72 insns** (best), DBCEA/DBECA
  = 10, the rest 13-14. Baseline (main order, pins stripped) = 12 @ 72.
  NB the previously banked `memory/grind/.../candidate.c` also scores 9 but at
  **70** insns (copy_end lands in caller-saved t0, losing the s5 save/restore).
- ALLOCDBG on the best form: all four invariants have **nrefs=3**, so global.c
  priority `floor_log2(n)*n*10000/livelen` is decided by **live length alone**.
  Measured livelens: buf2_ptr 32, index 34, **copy_end 38**, constant_80 62,
  **cam_base 66**.

## Why the solver's LEVER verdict is not C-reachable (mechanism)
`docs/grind/inverse-sweep-2026-08-06.md` gives two 1-atom vectors for pseudo 77
(copy_end). Both are foreclosed:
1. **`pref_add` → acquire a copy preference for `$s5`.** `global.c set_preference`
   only records a preference from a SET between a pseudo and a **hard reg**.
   `$s5` (and every callee-saved reg) **never appears in pre-RA RTL** — verified
   on this function's `.lreg` dump: the only hard GPRs present are v0 v1 a0 a1 a2
   ra. Additionally `prune_preferences` (global.c:897) strips **all call-used**
   regs from a call-crossing allocno's preferences, so no argument/return flow can
   leave a surviving preference either. Only a `register asm()` pin could create
   one — the forbidden construct.
2. **`refs_down` 3→1.** copy_end's RTL references are exactly two: the def
   (insn 27, loop depth 1) and the copy loop's terminating compare (jump_insn 87,
   loop depth 2). `flow.c` weights refs by loop depth ⇒ 1+2 = 3 is the **minimum**
   for any live variable here. refs=1 would need a variable with no def or no use.

Brute-forcing the solver's own forward model over refs 1..40 × livelen 1..160 for
every focus pseudo: the ONLY reachable configurations are refs=1 (impossible),
refs=2 with livelen ≥ 51, or refs=3 with livelen ≥ 76. copy_end's live length is
capped at **38** because its last use (the copy loop) structurally **precedes**
cam_base's and constant_80's last uses (the final `cdrom_BcdToFrames` /
`func_800372F4` argument). No byte-neutral later reference to copy_end exists.
⇒ copy_end can never out-live them, so it can never sort last. This corroborates
the s2/s3 grind sessions' "hard-capped ~L38" finding from an independent angle.

## Pending policy question (do NOT treat as cleared)
`tmp/scv2/DBCAE.c` (the 9 @ 72 form) differs from the pin-free main body ONLY by
reordering the five mutually-independent initialisers. cheat-reviewer 2026-08-06
returned **NEEDS_USER**: is "reordering pre-existing independent initialiser
statements purely to steer global.c allocno priority, derived by exhaustive
permutation search, with no program-logic rationale" legitimate plain C, or does
it need its own SOTN-evidence pass like do-while-zero / dead-store did? Until the
owner rules, this ordering is **NOT banked** as the candidate and must not be
cited as a cleared lever.

## Floor
- main honest distance (pins stripped): 12 @ 72 insns
- banked grind candidate: 9 @ 70 insns
- best measured form this session: 9 @ 72 insns (ordering pending policy ruling)
