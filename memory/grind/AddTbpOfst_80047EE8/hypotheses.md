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
