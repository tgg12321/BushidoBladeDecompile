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
