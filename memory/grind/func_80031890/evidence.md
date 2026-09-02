# Evidence bank — func_80031890

## s1 (2026-09-02, recon) — sandbox 0 on the first measured form

- **Function identity.** `func_80031890(u8 *obj, u8 *ent, s32 idx)` in
  src/code6cac_b.c. Structural sibling of the just-completed func_8002E838
  (src/code6cac_b.c:1165, Judge PASS docs/grind/decisions.md:20262): identity
  3x3 matrix at obj+0xD8, RotMatrixY/RotMatrixX, then the PsyQ libgte macro
  sequence gte_SetRotMatrix / gte_ldv0 / MVMVA / gte_stlvnl rotating the
  velocity vector at ent+0x44 in place. Head differs: a random-sign angular
  velocity nudge (`rng_Next() & 1`, sum of squares of vx/vz divided by 64,
  added to the s16 at ent+0x5E) gated on `*(s16 *)(ent+2) != 0xE`; angle1 from
  the u16 table D_8008EBA0[idx] & 0xFFF; angle2 = ((x*16 + y + z*8) & 0x7FF)
  - 0x400. Tail: velocity divided by 8 when (u32)(angle1-0x401) < 0x7FF else
  by 4, then position += velocity/2 (target's srl-31/addu/sra-1 is GCC's
  signed /2).
- **Honest floor THIS session:** `sandbox func_80031890 --disable all` =
  **0** (163/163 insns, rules_dropped 0, cheat_asm_stripped 34 = the island
  lines, which the sandbox strips as it does for every cop2 carrier; score
  stays 0 because the islands are canonical cop2, not cheat-asm).
- **Full build:** `verify-oracle` ok=true, build_sha1 ==
  62efab4f73f992798c43e8c730aa43baa10bb4fa (tmp/grind/func_80031890/s1/
  verify_oracle_s1.txt).
- **Canonical gate:** ASM-PARTIAL, "11/163 insns canonical-asm (GTE/cop2 op
  c2/ctc2/lwc2/mtc2/swc2)". scan_hand_coded: tier=LOW 1/8 (the known
  GTE-wrapper whole-function scoring artifact; tmp/grind/func_80031890/s1/
  scan_hand_coded_s1.txt). The grant door is therefore the owner-cluster
  enumeration, not the scanner tier.
- **Cluster membership.** func_80031890 is named as a confirmed
  handwritten-tagged carrier in the 2026-09-01 widened-anchor owner grant
  (docs/grind/decisions.md:18120) and in the cluster rule census
  (.claude/rules/cop2-addressing-preamble-cluster.md:156). The target's three
  materialize-then-copy preambles have non-$aN sources: `addu $t4,$s2,$zero`
  (asm/funcs/func_80031890.s:82), `addu $t4,$v0,$zero` (:95 and :107) — exactly
  the widened-anchor shape. Splat tags 8 island insns `/* handwritten
  instruction */`. func_80031890 is NOT a row in
  tools/grinder/owner_cluster_grants.txt (operator-maintained; sessions may not
  edit it). Precedent for PASS without a registry row: func_8002EA24
  (docs/grind/decisions.md:18276) — same cluster ruling, same absence, PASSed
  with an operator note; func_8002FF20 (decisions.md:19971) took the
  scope-grant route for the inline_asm_canonical.txt line.
- **Island spelling.** All four islands are character-identical to the shipped
  func_8002E838 form (src/code6cac_b.c, commit 49d6927e), itself
  character-identical to the owner-granted func_800203B4
  (inline_asm_canonical.txt:367). In-island GPR work is limited to the SDK
  macro bodies (move $12,%0 addressing preamble + the macro's own lw/lhu/sll/
  or); the address operands (mat, vec) are computed in C and bound as "r".
  Zero register pins, zero hardcoded-$N stack asm, zero barriers, zero FAKE.
- **Second-island register seat.** The target reuses `$v0` for `vec`
  (`addiu $v0,$s3,0x44` once, then `addu $t4,$v0,$zero` before both the ldv0
  and stlvnl islands). Plain `s32 *vec = (s32 *)(ent + 0x44);` bound as "r" in
  both islands lands in $v0 with no coercion — no calls follow, so the pseudo
  is caller-save-eligible and global-alloc's first free seat is $v0.
- **Head branch shape.** `adj = sum_sq / 64` vs `adj = -sum_sq / 64` in the
  two arms of `if (rng_Next() & 1)` reproduces the target's per-arm bgez/+0x3F
  fixups with the `sra 6` cross-jumped into the shared join (.L80031920). The
  s16 at ent+0x5E must be READ INTO A LOCAL BEFORE the rng_Next call (target
  `lh $s1,0x5E($s3)` precedes the jal); `+=` after the call would re-load it.
- **Retired chassis.** The pre-migration body (retired-chassis-2026-08/body.c,
  floor 36 under the old chassis) carried `register ... asm("$13"/"$14"/"$15")`
  pins and per-instruction `__asm__ volatile` islands with a move-aliasing
  `"move %0, %1"` — forbidden families (register-asm pins,
  INLINE_MOVE_ALIASING). Banked as rejected/register-pin-islands.c; not a
  starting point.
