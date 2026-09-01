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

## Session 3 (recon/integration, 2026-09-01) — Judge-passed form re-applied, DISTANCE 0 re-proven

- [s3] LEDGER-VS-HISTORY RECONCILIATION (read this before trusting older
  entries): the candidate.c/self_vet.md committed by 3b56b688 (layer-1 FAIL
  bank, 2026-08-31 21:06) were the STALE staged-z form — the tail
  `if (y + x < min_y) { z = 0; return z; }` that layer-1 FAILED and the driver
  BANNED. The form the Judge actually PASSED and ESCALATEd on at 21:22
  (decisions.md:16948 packet) — a dedicated `ret` result variable set in both
  arms with a dead `ret = 1;` inside the zero arm, under
  dead-store-fake-exception (func_80078EC0 confirmed closure,
  dead-store-fake-exception.md:107-128) — was never banked to candidate.c.
  Session 3 reconstructed it from the Judge packet + the rule's closure shape.
- [s3] Re-measured with the reconstructed body applied over the INCLUDE_ASM at
  src/code6cac_b.c:860: `sandbox func_8002D320 --disable all` -> score 0,
  120/120, rules_dropped 0, cheat_asm_stripped 46 (the three cop2 islands,
  masked by design). Artifact: tmp/grind/func_8002D320/s3/sandbox_0.json.
  candidate.c and self_vet.md now carry THIS form (staged-z fully absent;
  staged-value-reused-variable not cited anywhere for the tail).
- [s3] `verify-oracle --rebuild` correctly REFUSED on the dirty tree
  (dirty-build-inputs guard) — full-build SHA1 proof is the driver's step at
  staging, per pipeline design. The islands' bytes were already proven on main
  by the driver before the 2026-08-31 Judge cycle (the packet records
  full-build SHA1 == oracle).
- [s3] Grant state confirmed on-disk: inline_asm_canonical.txt:366 carries the
  executed OWNER-CLUSTER pipeline grant (2026-08-31); borderline.md:335 and
  decisions.md:16948 record it; dossier reports canonical-listed True,
  CONSISTENCY OK. Binding constraint decisions.md:16969: grant covers ONLY the
  three enumerated cop2 islands, no GPR asm outside them, and the
  FAKE-annotated dead store must remain annotated verbatim.

## Post-reset session 1 (recon, 2026-08-31) — discard fixed, DISTANCE 0 re-proven

- [s1'] WHY THE PRIOR SESSION WAS DISCARDED (root cause, so it never recurs):
  the driver's banned-construct validator string-matches the self-vet against
  state.json's banned_constructs list. The prior self_vet.md QUOTED the banned
  tail spelling verbatim (in a paragraph explaining the diff does NOT contain
  it) and named the banned family slug while disavowing it — the validator
  matched the quoted fragments and discarded the session. Lesson, binding for
  every future session on any function: NEVER reproduce a banned construct's
  literal spelling or its banned citation slug ANYWHERE in self_vet.md, even
  to deny it. Describe by paraphrase and point at the rejected/ bank instead.
  (state.json itself is driver-owned and carries the canonical ban text; the
  session must not copy it.)
- [s1'] Session number reconciliation: the driver reset state.json on
  2026-09-01T01:54Z after the discard (session_count 0, floors empty), so
  this session is dispatched as "session 1" even though evidence above
  records pre-reset sessions 1/3. The earlier entries remain valid history;
  floors quoted from them are chassis-relative to the same INCLUDE_ASM
  chassis, which is unchanged (verified: src line 860 was still INCLUDE_ASM
  at dispatch).
- [s1'] The Judge-passed ret-form body (candidate.c, unchanged from the [s3]
  reconstruction) applied over src/code6cac_b.c:860 →
  `sandbox func_8002D320 --disable all` = **score 0, 120/120,
  rules_dropped 0, cheat_asm_stripped 46** measured THIS session
  (tmp/grind/func_8002D320/s1/sandbox_0.json). Edits left in place in src/.
- [s1'] self_vet.md rewritten clean: same six-test content, all banned-string
  literals and the banned family slug removed by paraphrase; candidate.c
  header likewise scrubbed. Both family claims carry verbatim scope sentences
  + file:line precedents (dead-store-fake-exception.md:121,
  cop2-addressing-preamble-cluster.md:73).
