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
