# Evidence bank — func_8002BC68

- == imported from memory/wip notes.md ==
# func_8002BC68 (code6cac_b.c) — GTE-LZCS sibling of func_8002BEA0

(Historical: prior WIP called this BLOCKED behind the sibling's /100
magic-const-ordering wall. Session s1 DISPROVED that transfer — see below.)

- Direct sibling of func_8002BEA0 (same file, same structure), but the /100
  block's second operand is `arg0` (a register), NOT the 0x44C constant —
  so BEA0's magic-vs-0x44C delay-slot ordering wall DOES NOT EXIST here.
- GTE block is canonical-asm (mtc2/swc2 LZCS), verdict ASM-PARTIAL 2/130.
  Final state must be COMPLETED-INLINE-ASM-CANONICAL (like authorized
  siblings func_8001A67C / func_800274BC), never COMPLETED-C.

## s1 (2026-07-28, recon) — floor 14 → 2; distance-0 form found, ruling-pending

Measured floor history THIS session:
1. HEAD form (raw .word blocks + t4 pin + do-while-0 + "" barrier): **14**.
2. Transplant BEA0-candidate form (consolidated GTE `__asm__` block with
   "$12" clobber, explicit `v1_m >>= 1` before the lbu, block-scoped
   division temps `temp_v0 = arg0 - 0x64` / `temp_v1_3`): **9** (129→130
   insns fixed; srl insert/delete + reorder rules' diffs gone).
3. Rewrite else arm as native `var_a0 = (arg0 - (s32)var_t0) / 16;`
   (target's else arm IS GCC 2.7.2's canonical signed-/16 expansion:
   subu; bgez w/ sra in delay; addiu 0xF; sra — m2c had hand-expanded it,
   which forced different pseudos): **9 → 2**. This ONE change fixed BOTH
   the $2<->$3 else-arm rename cluster AND the beqz delay-slot ordering
   (delay slot now takes `lui $a0,hi(magic)` exactly like target) AND the
   sll-chain regs. The "magic-const ordering wall" inherited from BEA0's
   diagnosis never materialized once the /16 was written naturally.
4. Split-numerator probe (`s32 num = (...)*0x50; var_a0 = num/100;`):
   codegen-NEUTRAL (still 2, identical bytes). Reverted to inline form.
5. Extended GTE-block clobber list to `"$12","$13","$14","$15"`: **0**
   (130/130). Form saved to clobber-form-ruling-pending.c; reverted in
   src pending owner ruling. Layer-1 cheat-reviewer: FAIL (register-pin-
   by-clobber-spelling absent independent footprint evidence).

## The 2-insn residual — precise mechanism (RTL-proven, artifacts s1)

Build `mfhi $13; sra $3,$13,5` vs target `mfhi $24; sra $3,$24,5`.
- greg dump (tmp/grind/func_8002BC68/s1/bc68.i.greg): highpart pseudo 111
  is allocated to hard reg 64 (HI itself). Insn 192 `(set (reg:SI 13 t5)
  (reg:SI 64 hi))` has insn code -1 → it is a RELOAD-emitted move, not an
  allocated pseudo. The register choice is reload1.c's, not global.c's.
- reload1.c (frozen tools/gcc-2.7.2): `last_spill_reg = -1` per function;
  first reload gets `spill_regs[0]`. `order_regs_for_reload()` sorts
  potential spill regs by (uses ASC, regno ASC) and EXCLUDES every hard
  reg explicitly mentioned in the RTL (bad_spill_regs). Our asm's "$12"
  clobber is why $12 is skipped and $13 wins. For target's $24, hard regs
  $13,$14,$15 must have been explicitly mentioned in the original
  function's RTL (they have zero pseudo uses in target — no instruction
  touches them — so exclusion is the only route).
- COROLLARY: **no pure-C construct can move this residual.** C code cannot
  mention hard regs; only asm-level constructs enter bad_spill_regs. The
  honest pure-C-lever space for these 2 insns is EMPTY — the lever surface
  is the canonical asm island's register-usage metadata, which is an
  authorization-time question, not a grind question.
- Sibling sweep (all asm/funcs with `mtc2 $t4,$30`): the only functions
  whose target shows a reload-class mfhi after the LZCS island are BC68
  and BEA0 — BOTH show `mfhi $t8` (BEA0's regfix carries the identical
  `mfhi $13->$24` subst pair). No counter-example exists: no LZCS-island
  function's target shows a reload-mfhi at $t5. func_800274BC (authorized,
  "$12"-only clobber) has NO mfhi/reload in its target at all — its
  minimal clobber list is unconstrained by bytes, so it is NOT evidence
  against the t4-t7 footprint theory. func_8001A820's mfhi $t2 is a
  heavily-used spilled local (45 refs; function is asmfix whole-file) —
  non-discriminating.

## Transfer notes
- The `/16` native-division insight transfers NOWHERE in BEA0 directly
  (BEA0's else arm subtracts from const 0x44C — check whether writing its
  else arm as `(0x44C - var_t0) / 16` similarly collapses its residual;
  BEA0's floor-4 diagnosis predates this find and its 24k-iter permuter
  ran on the hand-expanded form).
- The clobber ruling, if granted, closes BEA0's mfhi subst pair too.

## Post-ruling recon session (2026-07-28) — floor 0, candidate-ready

- The owner/judge ruling REQUESTED by s1 was GRANTED (commit 104fc679,
  "grind: func_8002BC68 judge ruling"). Binding constraints: scope is
  exactly the two LZCS islands (BC68 + BEA0); end state MUST be
  COMPLETED-INLINE-ASM-CANONICAL via the standard authorization recipe
  (inline_asm_canonical.txt entry citing the ruling), never COMPLETED-C;
  the asm block's comment must state $13-$15 are bytes-forced
  reconstruction of the original island's register footprint (reload1.c
  bad_spill_regs proof, judge ruling 2026-07-28); no precedent for other
  clobber-list extensions.
- Applied candidate.c body + `"$12","$13","$14","$15"` clobbers + the
  mandated comment to src/code6cac_b.c (replacing the HEAD raw-.word +
  t4-pin + do-while-0 form).
- canonical: ASM-PARTIAL 2/130 (mtc2/swc2), unchanged.
- sandbox --disable all: **score 0, 130/130**, rules_dropped 6,
  cheat_asm_stripped 366. Artifact:
  tmp/grind/func_8002BC68/s1/sandbox_zero_post_ruling.json.
- Remaining (driver/authorization side, not grind): retire the 6
  regfix/asmfix rules + inline_asm_canonical.txt entry citing the ruling
  → COMPLETED-INLINE-ASM-CANONICAL. Same ruling closes BEA0's mfhi subst
  pair (separate function, out of this session's scope).

## Artifacts (s1)
- tmp/grind/func_8002BC68/s1/bc68.i.greg (+ .lreg/.sched2 etc.) — RTL dumps
- tmp/grind/func_8002BC68/s1/build.txt / target.txt / diff.sh — insn diffs
- tmp/grind/func_8002BC68/s1/greg.sh — dump reproduction script
