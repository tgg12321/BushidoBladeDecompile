# Hypothesis ledger — func_8002EBDC

## s1 (recon, 2026-09-10)

### H1 — sibling-template transplant closes the whole function — CONFIRMED (score 0)
- Statement: func_8002EBDC is the structural twin of the merged cluster sibling
  func_8002E838 (src/code6cac_b.c:1366, inline_asm_canonical.txt:373); writing
  its body with the same five SDK-macro islands per GTE pass and ordinary C for
  the head (ratan2 / squared magnitude / LUT lookup), the identity-matrix
  blocks, the RotMatrix* calls, the three signed /256 scalings and the negated
  second pass reaches sandbox 0 with no coercion.
- Mechanism: the target's non-island code is textbook GCC 2.7.2 output for
  those statements (cse constant folding of the scratchpad base inside the first
  EBB, `$s2` constant hold for 0x1000, the bgez/addiu 0xFF/sra 8 signed-divide
  idiom, sched interleaving of the next lw/mult into the divide gap).
- Probe: body_v1 (tmp/grind/func_8002EBDC/s1/body_v1.c) spliced in place of the
  INCLUDE_ASM, `sandbox func_8002EBDC --disable all`.
- Result: **score 0, 182/182, rules_dropped 0** on the first measurement;
  comment-only body_v2 (macro-name + inline_c.h:101-110 citations per the
  2026-09-02 Ruling A) re-measured 0. `verify-oracle --rebuild` refused
  (dirty-build-inputs guard, by design) — bytes proof is the driver's step.
- Verdict: CONFIRMED.

### H0 — DATA MODEL declaration fix (D_8008D118 header decl absent) — not a codegen signal
- The DATA MODEL flag is "decl NONE in include/*.h"; the TU-local
  `extern u8 D_8008D118;` (src/code6cac_b.c:278) reproduces both LUT sites
  byte-exact (score 0 measured with it). No declaration change needed; see the
  OBJECT MODEL entry in evidence.md.

## Frontier
- (none) — function at sandbox 0; next step is the Judge / owner-cluster grant
  door (honest bucket COMPLETED-INLINE-ASM-CANONICAL, allowlist line written by
  the driver/operator, as for func_8002E838 / func_8002EA24 / func_80031890).

## s1b (recon re-dispatch after the 8438e66dc merge refusal, 2026-09-10)

### H2 — the flagged islands' GPR preamble is C-expressible (refusal branch 1) — KILLED (instance, score 7)
- Statement: respelling the second gte_ldlv0 island's VX0/VY0 pack
  (move/lhu/lhu/sll/or) as ordinary C with single-insn cop2 islands, on the
  s1 body_v2 chassis (score 0) with no FAKE constructs present, reproduces the
  target's `addu $t4,$s3,$zero; lhu $t6,4($t4); lhu $t5,0($t4); sll; or; mtc2
  $t5,$0; lwc2 $1,8($t4)` bytes.
- Mechanism: GCC 2.7.2 never emits a redundant reg-to-reg copy of a live
  pointer with no other consumer (cse.c/combine.c fold it), and local-alloc
  seats the two short loads in the first free caller-saved regs ($v0/$v1),
  addressing them off the folded scratchpad base $s0 (offsets 168/172).
- Probe: tmp/grind/func_8002EBDC/s1/probe_c_pack.py applied to src, `sandbox
  --disable all`.
- Result: **score 7, 181/182** (tmp/grind/func_8002EBDC/s1/probe_c_pack_sandbox.txt,
  probe_c_pack_objdump.txt). Form banked at
  rejected/thin-ldlv0-c-pack-seats-v0-v1-off-s0-not-t5-t6.c.
- Verdict: KILLED (instance: this island, this chassis, zero FAKE constructs).
  Consistent with the class result already on record for the family
  (func_800300B4 H4/H29/H30; rule :159-160).

### H3 — the missing surface is the operator registry row, nothing else — CONFIRMED (read from the driver, not inferred)
- grind.ps1 PASS path -> island-count 7/no -> grant_canonical_asm(tier LOW) ->
  no owner_cluster_grants.txt row -> REFUSED. With a row present the same path
  writes the inline_asm_canonical.txt line itself (tier "OWNER-CLUSTER") and
  `queue done` lands the function as COMPLETED-INLINE-ASM-CANONICAL.

## Frontier
- (none grindable) — awaiting the operator registry row named in the
  2026-09-10 decisions.md entry; on `queue unpark`, the next session submits
  candidate.c EXACTLY (Judge clearance 1b44e6afef0b58e6 skips layer-1).

## [s1] The candidate body re-measures at honest floor 0 on the current chassis (HEAD 6c886096c, -mel -msoft-float).
- mechanism: Body unchanged since the Judge PASS; toolchain fingerprint unchanged; sandbox re-run with candidate.c applied.
- probe: candidate.c copied over src/code6cac_b.c (LF); sandbox func_8002EBDC --disable all
- result: score 0, target_insns 182, build_insns 182, rules_dropped 0, cheat_asm_stripped 22.
- verdict: CONFIRMED

## [s1] Respelling the second gte_ldlv0 island's VX0/VY0 pack (move/lhu/lhu/sll/or) as ordinary C with single-insn cop2 islands, on the score-0 candidate chassis with zero FAKE constructs, reproduces the target's addu $t4,$s3,$zero / lhu $t6,4($t4) / lhu $t5,0($t4) / sll / or / mtc2 $t5 / lwc2 $1,8($t4) bytes.
- mechanism: GCC 2.7.2 folds the scratchpad base into $s0-relative addressing (cse.c) and local-alloc seats the two lhu results in $v0/$v1; no pass emits a redundant reg-to-reg copy of a live pointer, so the $t4 preamble and the $t5/$t6 seats are hand-asm.
- probe: tmp/grind/func_8002EBDC/s1/probe_c_pack.py applied; sandbox func_8002EBDC --disable all; objdump of the disabled .o.
- result: score 7, 181/182: lhu v0,172(s0); lhu v1,168(s0); sll; or; mtc2 v1,$0; lwc2 $1,8(s3) — the move $t4 is gone and every GPR seat differs. Banked at rejected/thin-ldlv0-c-pack-seats-v0-v1-off-s0-not-t5-t6.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6c886096c chassis (-mel -msoft-float, zero regfix/asmfix rules), candidate.c body with the second gte_ldlv0 island respelled, zero FAKE constructs present

## [s1] The merge refusal's blocking surface is exactly one operator row in tools/grinder/owner_cluster_grants.txt; with that row the driver's PASS path writes the inline_asm_canonical.txt line itself.
- mechanism: grind.ps1 PASS path: island-count (audit_asm_cheats whitelist: 7 of 9 islands flagged, MVMVA .words whitelisted) -> grant_canonical_asm(tier LOW) -> registry lookup -> REFUSED; with a row, tier OWNER-CLUSTER -> allowlist line -> queue done as COMPLETED-INLINE-ASM-CANONICAL.
- probe: Read tools/grinder/grind.ps1:838-870, tools/grinder/grindlib.py:1648-1734, tools/audit_asm_cheats.py:694-711; scan_hand_coded --single func_8002EBDC (tier LOW 1/8).
- result: Confirmed from the driver source and the banked constraint text; precedent func_80019310 (decisions.md:24678 -> row 2cef233c -> merged 3869ca31).
- verdict: CONFIRMED
