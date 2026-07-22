# Hypothesis ledger — AddTbpOfst_80047EE8

## s1 (2026-07-21, recon) — floor 15 -> 10

- H1 CONFIRMED — single-walker merge: the old candidate split the walker into
  `cached` (s32) + `p` (u32*); target carries ONE register ($s0) end-to-end.
  Merging into one pointer variable: 16 -> 14. (Baseline re-measure of the
  wip-imported form was 16, not 15 — minor drift.)
- H2 CONFIRMED — live `first = saved + (new_var2 << 2)` precompute inside the
  loop (sibling InitHiraRmd_80047FBC committed lever 1) flips whole-function
  RA so the prologue stages arg0 through $s0 (sw s0; move s0,a0; sw s2;
  move s2,s0; addu s0,s0,a1 — exact target shape): 14 -> 11, insns 52 -> 53/53.
- H3 KILLED — FAKE arg0=0 WITHOUT the H2 staging is INERT at both prologue
  positions (score 14, prologue byte-identical). See
  rejected/fake-arg0-zero-without-staging.c.
- H4 CONFIRMED — FAKE arg0=0 ON TOP of H1+H2 flips the second-pointer binding
  (addu s0,a0,v0 -> addu s0,s2,v0): 11 -> 10. Same cse2 canonical-register
  mechanism + same lever as sibling s6 (Judge-PASSed there). Exhaustion
  prerequisite for shipping it NOT yet discharged on this function.
- Residual at 10 = EXACTLY the 32-byte unused frame (2x addiu sp + 8 save/
  restore offsets; every other insn identical — tmp/grind/AddTbpOfst_80047EE8/
  s1/diff_p6.out). Identical species to sibling's pending owner-escalation.
- Probe note: `s32 buf[8]` unused array is STRIPPED by the sandbox
  (cheat_asm_stripped 381 -> 382, score unchanged) — the frame gap cannot even
  be measured via the cheat form, let alone closed by it.

## [s1] Merging the split cached/p variables into one walking pointer matches target's single-register ($s0) carry chain
- mechanism: target allocates arg0-copy -> add -> reload -> loop walker to ONE pseudo; two source variables split the live range and let the short first range land in $a1
- probe: rewrote prologue as p=(u32*)arg0; saved=(s32)p; p=p+off; sandbox --disable all
- result: 16 -> 14
- verdict: CONFIRMED

## [s1] A live precompute of the call's first arg (first = saved + (new_var2<<2)) inside the loop flips whole-function RA to stage arg0 through $s0 in the prologue
- mechanism: sibling InitHiraRmd_80047FBC committed lever 1: the extra loop-body pseudo referencing saved shifts allocation so GCC emits sw s0; move s0,a0; sw s2; move s2,s0; addu s0,s0,a1 instead of copy-propping to move s2,a0
- probe: added s32 first local + used as call arg; sandbox
- result: 14 -> 11, build insns 52 -> 53/53, prologue shape == target
- verdict: CONFIRMED

## [s1] FAKE arg0=0 dead store alone (without the staging lever) breaks the cse2 class and materializes the $s0 staging
- mechanism: dead-store-fake-exception family; expected to break {arg0,p,saved} canonical-register equivalence
- probe: arg0=0 /* FAKE */ at both prologue positions on the single-walker form, no first-precompute; sandbox + prologue disasm
- result: INERT: score 14 both positions, prologue byte-identical; flow deletes the dead set before it matters when no staged allocation exists
- verdict: KILLED

## [s1] FAKE arg0=0 on top of the staging levers closes the second-pointer binding diff (addu s0,a0,v0 -> addu s0,s2,v0)
- mechanism: cse2 canonical-register substitution folds base==arg0 and picks $a0; the annotated dead store kills the $a0 association (sibling s6 mechanism, Judge-PASSed there)
- probe: arg0=0 /* FAKE */ after saved=(s32)p on the H1+H2 form; sandbox + full-stream diff
- result: 11 -> 10; instruction stream now identical to target EXCEPT the 10 frame-offset insns
- verdict: CONFIRMED

## [s2] A pure spelling of the saved/p init chain (const, decl-order, split-init, retype, rebind) can flip the second-pointer binding to addu s0,s2,v0 without the FAKE arg0=0 store
- mechanism: cse2 canonical-register substitution puts {arg0, p, saved} in one equivalence class and picks $a0 for the add; the question was whether any C-level spelling changes the class or the canonical pick
- probe: On the floor-10 chassis, swept: V2 const decl-init saved, V3 decl-order swap, V4 split-init reversal (saved=arg0; p=(u32*)saved), V5 u32-typed saved, V6 two-statement rebind from saved; each measured with sandbox --disable all; control V1 = FAKE removed
- result: V1 control 11; V2=V3=V4=V5=V6 all 11 (53/53 insns) — byte-identical to the no-FAKE control; the FAKE form remains uniquely 10. Matches sibling s8 (const discarded at RTL) and s9 (pseudo order is first-USE LUID) exactly
- verdict: KILLED

## [s2] Re-associating the second-pointer offset as a mask (saved + (v_off & ~3)) instead of the srl/sll shift pair reaches the target binding
- mechanism: different RTL for the offset computation could break the cse2 substitution site or change combine's shape
- probe: V7: p = (u32*)(saved + (s32)(v_off & ~3)) on the floor-10 chassis; sandbox --disable all
- result: 14, build insns 52 vs target 53 — WORSE and byte-diverging: target carries the two-shift pair, the mask spelling emits andi and cascades
- verdict: KILLED

## [s3] A pure-C LIVE-locals shape reserves the target's phantom 32-byte frame (vars=32) without emitting stores or changing the 53-insn stream
- mechanism: phantom-frame-slots-gcc272 — GCC 2.7.2 reload reserves spill slots (counted by get_frame_size) for pseudos it keeps in registers; witnessed at vars=8 in tslLineG5Init from an s16 pair feeding `(a&~b)&1`. Frontier F2 claimed this body's s16->SImode call-arg widening + u32 word local would trigger a comparable (32-byte) reservation.
- probe: cc1 .frame instrument (framedump.sh) + 9-variant grid (sweep.py, frame_grid.md): base, fn-scope hoist, u64 word, struct record, staged temps, folded himode-pair, written array, u64 pair, and a genuinely-live tslLineG5Init-clone guard (v08).
- result: EVERY stream-preserving variant = vars=0 (frame 40, distance 10). v08 (exact tslLineG5Init trigger, live guard on a real global store) = vars=0 — the mechanism does NOT fire here. Only a WRITTEN s32 rec[6] reaches vars=24, at +6 diverging stores the target lacks (forbidden dead-array, no carve-out).
- verdict: KILLED. The 32-byte phantom reservation is not reproducible by any pure-C live-locals shape in this body; it requires the forbidden unwritten/written dead array. Structural axis exhausted. Same endgame-lock species as sibling InitHiraRmd_80047FBC.

## [s3] A pure-C LIVE-locals shape reserves the target's phantom 32-byte frame (vars=32) without emitting stores or diverging from the 53-insn stream.
- mechanism: phantom-frame-slots-gcc272: GCC 2.7.2 reload reserves spill slots (counted by get_frame_size) for pseudos it keeps in registers; witnessed vars=8 in tslLineG5Init from an s16 pair feeding (a&~b)&1. Frontier F2 claimed this body's s16->SImode call-arg widening + u32 word local would trigger a ~32-byte reservation.
- probe: Built the cc1 .frame instrument (framedump.sh); ran a 9-variant grid (sweep.py) reading vars= and sp-store count: base, fn-scope hoist, u64 word, struct record, staged temps, folded himode-pair, written array, u64 pair, and v08 = the exact tslLineG5Init trigger (live s16 pair (hv&~hm)&1 guarding a real global store).
- result: Every stream-preserving variant = vars=0 (frame 40, distance 10). v08 (exact tslLineG5Init trigger) = vars=0 — the mechanism does NOT fire in this body. Only a written s32 rec[6] reaches vars=24, at +6 diverging sp-stores the target lacks (forbidden dead-array; no carve-out, matching sibling s7: target has ZERO sw in the vars region).
- verdict: KILLED

## [s3] The candidate floor-10 form re-measures distance 10 this session.
- mechanism: single-walker + first-arg precompute + FAKE arg0=0 staging (s1/s2 levers); residual is purely the frame-offset insns.
- probe: Applied candidate.c to src/text1b.c; `& tools/wteng.ps1 main sandbox AddTbpOfst_80047EE8 --disable all`.
- result: score 10, target_insns 53, build_insns 53, scorable true. Confirmed floor=10 this session.
- verdict: CONFIRMED
