# Evidence bank — func_8002D320

## Session 1 (recon, 2026-08-31) — DISTANCE 0 reached

- [s1] Chassis at dispatch: src/code6cac_b.c:860 = `INCLUDE_ASM("asm/funcs", func_8002D320);`
  (asm-until-matched migration). Queue distance 32 was the RETIRED chassis's floor
  (cheat-stripped body with the coordinate/LZCR stores deleted) — never the honest
  floor of a clean body. Dossier: aliases none, rules keyed none, canonical-listed
  False, CONSISTENCY OK.
- [s1] First honest body (retired body minus all cheats + brief levers 1+2 + the
  twin's Judge-endorsed narrow island forms): `sandbox func_8002D320 --disable all`
  → **score 3, build 118 vs target 120**. Every register seat already matched;
  the ENTIRE residual was the final `if (y + x < min_y) return 0; return 1;` pair
  store-flag-folded to `slt; xori v0,v0,1` where target keeps the unfolded diamond
  (`bnez; move v0,zero` delay; `addiu v0,zero,1`). Disassembly: tmp/grind/func_8002D320/s1/build_dis.txt.
- [s1] Why the twin's 17-session allocator wall does NOT exist here: the extra
  leading `flag` parameter shifts obj→$a1, pos→$a2, threshold→$a3, r_sq→$t0, so
  every register seat target uses downstream (x/dist_sq/disc/sqrt chain in $a0 =
  flag's dead seat; neg_threshold+min_y in $a2 = pos's dead seat; max_y in $a3 =
  threshold's seat after last read; y in $a1 = obj's seat after last read) is a
  naturally-freed argument register. No conflict-graph fight; honest C allocates
  to target directly.
- [s1] Brief LEVER 1 confirmed as-written: `else if (min_y < y_low) max_y = y_low;`
  emits target's `slt $v0,$a2,$v1` (cse does NOT constant-fold min_y=0 into the
  compare across the bgez branch). Brief LEVER 2 confirmed: early-return tail, no
  carrier.
- [s1] Tail-fold measurements (the only search this function needed):
  v1 plain early-return: 3/118 (folds). v2 result-carrier nest: 4/119
  (rejected/tail-result-carrier-nest-score4.c). v3 goto-reject: 3/118
  byte-identical to v1 (jump threading re-folds). v4 inverted sense
  (`if (y+x >= min_y) return 1; return 0;`): 3/118 byte-identical.
- [s1] CLOSER: the twin's L1 construct — `if (y + x < min_y) { z = 0; return z; }`
  (staged-value-reused-variable; z's rotated-Z value dead after `z * z`) →
  **score 0, 120/120**, FAKE-annotated in source. Tail diamond verified in
  disassembly (bnez v0; move v0,zero; ... bnez v0; move v0,zero; li v0,1).
- [s1] Island provenance (all inherited, none re-derived): vector island = TWO
  asm blocks (twin [s3]: two-statement split required), addresses computed in C
  (`vin = (s32*)(obj+0xF8)` / `vout = (s32*)(obj+0x100)`), template = `addu $t4,%0,$zero`
  preamble + cop2 ops only, `"$12","memory"` clobbers — the exact narrow form the
  2026-07-30 twin Judge ruling demanded (docs/grind/decisions.md, func_8002EA24
  FAIL entry: "bind the already-computed address as the operand ... let the
  template contain only addu $t4"). LZCS island verbatim from the authorized
  sibling func_800274BC (src/code6cac_b.c:292, `"=m"(sp_var)`, `"r"(x)`, `"$12"`).
  Cluster membership: .claude/rules/cop2-addressing-preamble-cluster.md:73
  (func_8002D320, 1 idiom site). All four mechanical-check conditions hold:
  sandbox 0; zero pins/barriers/move-aliasing; in-island GPR = addressing
  preamble only; layer-2 + verify-oracle --rebuild owed at integration.
- [s1] END STATE: COMPLETED-INLINE-ASM-CANONICAL (canonical cop2 islands +
  pure C elsewhere), via the judge-sole-gate rule-3 pipeline grant path — the
  driver writes the inline_asm_canonical.txt entry, not the session.
