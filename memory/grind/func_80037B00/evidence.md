# Evidence bank — func_80037B00

- WIP rejected_form: pin-free + no-dummy (23; whole-function register-rotation cascade + loop scheduling shift + absent 8-byte frame)

- WIP rejected_form: pin-free + sp_dummy[2] (FAILED reviewer 2026-06-22 — dead-vars-local-array; saved under rejected/. GCC DCEs the dummy in pin-free form so the frame doesn't materialize regardless — the construct is both forbidden AND ineffective here)

- WIP rejected_form: do-while restructure pin-free (sandbox=17, build_insns=33 — worse than the goto form: GCC dropped one inner-loop instruction; the do-while loop-exit collapses one slt/bnez pair)

- == imported from memory/wip notes.md ==
# func_80037B00 — BLOCKED (multi-register rotation + phantom +8 frame)

## TL;DR
String-compare loop over D_80102810 entries (matches arg0 against each 0x15-byte
entry; 0x28 stride). HEAD "matches" via NINE `register asm()` pins
(t1,t3,t2,a3,a1,a2,t0,v1,v0) + `s32 sp_dummy[2]` frame-padding cheat —
forbidden, inert under the sandbox. INCOMPLETE.

## Why blocked (pin-free floor 23 honest insns >> HEAD's cheated 15)
candidate.c (no pins, no dummy) measures sandbox=15 weighted-masked / 23 honest
insn diff. Two coupled problems:

1. **5-way register rotation**: GCC picks t0/v1/a1/t1/a2 where target uses
   t1/a1/a2/t0/v1 — a coupled rename across 5 vars, not a single swap. The
   masked-0 score earlier observed with sp_dummy was misleading (registers
   mask out before the weighted score) — `retire` rebuilds and fails SHA1.
2. **8-byte phantom frame**: target emits `addiu sp,sp,-8` / `addiu sp,sp,8`
   wrapping a body that never touches sp. Pin-free C gives GCC no reason to
   reserve frame, so no prologue/epilogue. `s32 sp_dummy[2]` would force the
   frame ONLY when paired with the register pins (the pin reload pressure
   keeps the unused local alive). In pin-free form it DCEs away — AND it's a
   forbidden dead-vars-local-array per the cheat policy (rejected/, 2026-06-22).
3. **Inner-loop scheduling**: subtle shift in `slt`/`addiu` ordering near
   .L80037B30 — a side-effect of the different RA, will likely follow once
   the rotation is forced.

## Cluster note
func_80037AA4, file_LoadSectors, func_80037B00 ALL carry an unexplained +8
stack frame the DCE'd dummy can't supply. A single cc1psx vs cc1-mips-2.7.2
calibration on the cluster could explain whether the toolchain we're using
emits this frame differently from the original PsyQ cc1psx.

## Highest-yield untried lever
**decomp-permuter from candidate.c** directed at the 5-way register rotation.
Same shape as the marionation_Exec coupled-rotation work — may plateau, but
this is the right tool for "C is correct, registers wrong." Seed: candidate.c.

## What was tried (this/prior session)
- Pin-free goto body (m2c shape): score 15 weighted, 23 honest. (candidate.c)
- Pin-free + `s32 sp_dummy[2]`: reviewer FAIL (dead-vars-local-array).
  Importantly: GCC DCEs sp_dummy in pin-free form so the frame doesn't even
  materialize — the construct is both forbidden AND ineffective here.
- Do-while restructure: sandbox=17, build_insns=33 (worse; lost one inner
  insn).

## Floor
HEAD: 15 (cheated, 9 pins + sp_dummy, SHA1-matches).
candidate.c: 23 honest insn diff, 15 weighted-masked. NOT lowered this session.


- [s1] canonical func_80037B00 → verdict=C, distance=15 (pure-C target)

- [s1] sandbox --disable all on HEAD (pins+dummy stripped) → score=15, target_insns=36, build_insns=34, cheat_asm_stripped=27

- [s1] Target frame=8, zero body accesses to sp+0..7, zero jals → outgoing-args=0, so the 8 bytes are unambiguously a vars slot (contrast file_LoadSectors s1 which resolved its frame slack to outgoing-args=24)

- [s1] Sibling file_LoadSectors evidence bank: GCC 2.7.2 provably NEVER DCEs local-array stores (empirical test 2026-06-16)

- [s1] 2026-07-01 dead-vars-local-array carve-out gate FAILS here: requires target dead stores in the reserved slot; func_80037B00 target has zero stores in sp+0..7

- [s1] Owner standing 2026-07-27 auto-ruling: Gate 1 (STRONG scan_hand_coded S1/S2/S6 for canonical-asm) FAILS — this is a plain strncmp-style dispatch loop; Gate 2 (SOTN precedent for a sanctioned coercion) FAILS — no dead-store precedent applies. HOWEVER an unexhausted untried lever exists (directed permuter from candidate.c per WIP), so not yet a both-gates-fail-with-nothing-left endgame lock.

- [s1] Register axis: 5-way coupled rotation (t0,v1,a1,t1,a2) → (t1,a1,a2,t0,v1); not a single swap, no shared-exit CSE, no jals — call-return-* / exit-path-* / compare-operand-order patterns don't apply directly

- [s1] Rejected forms bank contains: dead-vars-local-array.c (do-not-repropose)

- [s2] s2: baseline pin-free candidate.c score=15 weighted, target_insns=36, build_insns=34, cheat_asm_stripped=8 (HEAD's pins+dummy dropped)

- [s2] s2: register-alloc-pure-c Levers A (block-local split) and B (narrow types) both exhausted for this function — no structural rewrite of the local decl set moved the score

- [s2] s2: shared-end-label makes score WORSE (not better) for this function — added live scalar amplifies the 5-way rotation tiebreak rather than resolving it

- [s2] s2: load-order restructuring is contraindicated — GCC folds paired byte-loads and drops instructions target retains

- [s2] s2: 3 KILLED + 2 INERT structural variants, all measured; structural modality now exhausted for the register-rotation axis

- [s2] s2: no rejected form was banked-cheat family; all failed on their measured score, not on policy
