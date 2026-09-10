# Evidence bank — func_8002EBDC

## s1 (recon, 2026-09-10) — chassis: -mel -msoft-float, zero regfix/asmfix rules

- CANONICAL gate: ASM-PARTIAL, 24/182 insns cop2 (c2/ctc2/lwc2/mtc2/swc2). The
  function is enumerated BY NAME in the 2026-08-17 owner cluster ruling's
  membership table (`.claude/rules/cop2-addressing-preamble-cluster.md:80`,
  site L44 `addu $t4,$a0,$zero -> mtc2 $t4,$30 ; nop ; nop`). It is NOT in
  `tools/grinder/owner_cluster_grants.txt` (operator-only registry); the landed
  cluster ruling naming it in the rule's census table is the authority, the
  same situation the Judge accepted for func_80031890 (decisions.md:20268).
- FUNCTION SHAPE: exact structural twin of the merged sibling func_8002E838
  (src/code6cac_b.c:1366, inline_asm_canonical.txt:373). Differences: the
  object base is the scratchpad constant 0x1F8002B8 instead of a param; the
  direction vector is a separate `s16 *` param (lh loads); the rotated vector
  is scaled (z by arg3, x/y by arg4, each signed /256) between the two MVMVA
  passes; the second pass applies the negated angles and stores to `out`.
- FLOOR: body_v1 (`tmp/grind/func_8002EBDC/s1/body_v1.c`) scored
  **sandbox --disable all == 0 (182/182, rules_dropped 0)** on the first
  measurement. Five islands per pass (LZCS/LZCR, gte_SetRotMatrix, gte_ldlv0,
  MVMVA .word 0x4A486012, gte_stlvnl), character-identical to func_8002E838's.
  Everything else is ordinary C: no pins, no aliasing blocks, no barriers, no
  FAKE constructs, no dead stores.
- ORDINARY-C FACTS THAT MATCHED WITHOUT COERCION (bank for siblings):
  - `u8 *scr = (u8 *)0x1F8002B8;` local: the first store (`scr+0xFA`, before
    the LZC branch) folds to the absolute `lui $at / sh ...(0x1F8003B2)` form
    while every store after the join uses the `$s0` base — cse.c folds the
    constant only inside the first extended basic block; the `lui/ori $s0`
    materialization lands in the beqz delay slot. No trick needed.
  - `(v * k) / 256` on an s32 emits the target's `mult; mflo; bgez; addiu 0xFF;
    sra 8` sequence exactly, with the scheduler interleaving the next `lw/mult`
    into the gap.
  - Identity-matrix block written twice as nine `*(s16 *)(scr+N) = ...` stores;
    the 0x1000 constant is held in `$s2` across both blocks by cse (no
    constant-holder needed).
  - `dist_sq = dir[0]*dir[0] + dir[2]*dir[2]` on `s16 *` gives the separate
    lh/mult/mflo pairs and the `sltiu 0x400` unsigned guard from `(u32)dist_sq
    < 0x400`.
- OBJECT MODEL: D_8008D118 (g_isqrt_lut, byte LUT) — declared TU-locally
  `extern u8 D_8008D118;` at src/code6cac_b.c:278 (no header decl; DATA MODEL
  flag = "decl NONE in include/*.h"). Access spelled
  `*(((u8 *)&D_8008D118) + idx)` exactly as in the six matched users in this
  TU (lines 284, 747, 1383, 1405, 1514, 1532). MATCHES (measured score 0 with
  this declaration; both `lui $at,%hi / addu $at,$at,idx / lbu %lo($at)`
  sites reproduce). No declaration fix needed; the header-decl absence is a
  hygiene note only, not a codegen signal.
- ORACLE: see s1 hypotheses.md H1 result for the verify-oracle --rebuild SHA1.
- Honest bucket per the 2026-09-02 owner Ruling A: COMPLETED-INLINE-ASM-CANONICAL
  (allowlist line operator/driver-written via the owner-cluster door on the
  Judge PASS path). Never COMPLETED-C.

## s1b (recon re-dispatch after the 8438e66dc merge refusal, 2026-09-10) — chassis HEAD 6c886096c (-mel -msoft-float, zero regfix/asmfix rules)

- RE-MEASURED THIS SESSION: candidate.c applied to src/code6cac_b.c ->
  `sandbox func_8002EBDC --disable all` = **score 0, 182/182, rules_dropped 0,
  cheat_asm_stripped 22** (the 22 are the nine islands' own lines the sandbox
  strips; the honest body is byte-exact). Judge PASS on this exact body is on
  record (state.json review_ledger, hash 1b44e6afef0b58e6, 2026-09-10 08:39).
- WHY THE MERGE WAS REFUSED (mechanics, read from the driver): grind.ps1's PASS
  path calls `grindlib.py island-count` (audit_asm_cheats.scan_c_body_smuggled_work
  + _is_whitelisted_insn, tools/audit_asm_cheats.py:694-711): whitelist = nop,
  cop2/ctc2/mtc2/mfc2/cfc2/lwc2/swc2, and cop2-class `.word`s. Our nine islands
  split 2 whitelisted (the two MVMVA `.word 0x4A486012`) + 7 flagged (LZCS with
  its addu/addiu preamble; 2x gte_SetRotMatrix with move/lw; 2x gte_ldlv0 with
  move/lhu/lhu/sll/or; 2x gte_stlvnl with move). 7 > 0 and not allowlisted =>
  `grant_canonical_asm` (grindlib.py:1648): STRONG scan tier OR a row in
  tools/grinder/owner_cluster_grants.txt. `scan_hand_coded --single` = tier LOW
  1/8 (S4 front-loads only; tmp/grind/func_8002EBDC/s1/ not needed — the driver
  printed the same tier in the constraint). No registry row. REFUSED.
- THE GRANT THAT COVERS THIS FUNCTION ALREADY EXISTS: func_8002EBDC is enumerated
  BY NAME in the 2026-08-17 cluster ruling's census table
  (.claude/rules/cop2-addressing-preamble-cluster.md:80, row `func_8002EBDC |
  L44 addu $t4,$a0,$zero -> mtc2 $t4,$30 ; nop ; nop`, LZCS sub-family). Its
  other six preambles copy from $s1 (gte_SetRotMatrix x2, .s L93/L155-ish) and
  $s3 (gte_ldlv0 / gte_stlvnl) — non-$aN sources, covered by the 2026-09-01
  widened-anchor owner GRANT (decisions.md:18082; rule :140-160). Ten splat
  `/* handwritten instruction */` tags in asm/funcs/func_8002EBDC.s (L45, L92-99,
  L105, L113-114, L163...). Island spelling character-identical to the merged
  siblings func_8002E838 (inline_asm_canonical.txt:373) / func_80031890 (:374).
- THE "ISLANDS ARE C-EXPRESSIBLE" BRANCH OF THE REFUSAL IS MEASURED DEAD ON THIS
  CHASSIS (H2 below): the second gte_ldlv0 pack written as C
  (`u32 packed = *(u16*)vec | ((u32)*(u16*)((u8*)vec+4) << 16)` + single-insn
  mtc2/lwc2/nop islands) scores 7 (181/182): GCC 2.7.2 folds the scratchpad base
  and emits `lhu v0,172(s0); lhu v1,168(s0); sll; or; mtc2 v1` — seats $v0/$v1
  off $s0, never the target's `addu $t4,$s3,$zero` copy + $t5/$t6 pair, and the
  lwc2 base becomes $s3 instead of $t4. tmp/grind/func_8002EBDC/s1/
  probe_c_pack_objdump.txt lines ~163-168 vs asm/funcs/func_8002EBDC.s L100-106.
  Same class result as func_800204C0 s1b H5, func_80019310 s3 H10,
  func_800300B4 H4/H29/H30 (local-alloc.c:2207/2249).
- PRECEDENT FOR THE DISPOSITION: func_80019310 (decisions.md:24678, s1 recon ->
  owner-gated -> operator row 2cef233c -> unpark -> merged 3869ca31
  COMPLETED-INLINE-ASM-CANONICAL) and func_800204C0 (decisions.md:25556, rotated,
  awaiting its row). This session files the same-shaped record (decisions.md
  2026-09-10 entry) and returns owner-gated. NOTHING is grindable here: the body
  is at 0 and Judge-PASSed; the only missing surface is operator-only.

- [s1] OBJECT MODEL: D_8008D118 (g_isqrt_lut, byte LUT; DATA MODEL flag 'decl NONE in include/*.h') — MATCHES (measured score 0 this session with the TU-local `extern u8 D_8008D118;` at src/code6cac_b.c:278 and the `*(((u8 *)&D_8008D118) + idx)` spelling used by the six matched users in this TU); no declaration fix needed, the header-decl absence is hygiene only.

- [s1] Honest floor re-measured 0 (182/182) on HEAD 6c886096c with memory/grind/func_8002EBDC/candidate.c applied; Judge PASS on this exact body is on record (state.json review_ledger hash 1b44e6afef0b58e6).

- [s1] Island census against the driver's whitelist (tools/audit_asm_cheats.py:694-711): 9 islands, 2 whitelisted (MVMVA .word 0x4A486012 x2), 7 flagged (LZCS, gte_SetRotMatrix x2, gte_ldlv0 x2, gte_stlvnl x2) — the '7 non-cop2-whitelist islands' in the banked constraint.

- [s1] Grant coverage: func_8002EBDC is enumerated by name in the cluster rule census table (.claude/rules/cop2-addressing-preamble-cluster.md:80, $a0-source LZCS site .s L44); its seven other preambles copy from $v0/$s1/$s3/$s6 (.s L49, L89, L100, L111, L160, L171, L181), covered by the 2026-09-01 widened-anchor grant (decisions.md:18082); ten splat handwritten-instruction tags in asm/funcs/func_8002EBDC.s.

- [s1] scan_hand_coded --single func_8002EBDC = tier LOW 1/8 (S4 only), so the STRONG door is closed; no row in tools/grinder/owner_cluster_grants.txt (operator-only), so the registry door is closed until the operator adds it.

- [s1] C-respell probe of the gte_ldlv0 pack: score 7 (181/182) — GCC seats the loads in $v0/$v1 off $s0 and drops the $t4 copy; consistent with func_800204C0 s1b H5, func_80019310 s3 H10, func_800300B4 H4/H29/H30.

- [s1] Disposition record filed: docs/grind/decisions.md '2026-09-10 — func_8002EBDC (src/code6cac_b.c) — CANONICAL-ASM GRANT PATH: blocked at the operator registry row' with the exact one-line registry remedy and the unpark/resubmit steps.

- [s1] src/code6cac_b.c restored to HEAD (INCLUDE_ASM) before finishing; the proven form lives in memory/grind/func_8002EBDC/candidate.c.
