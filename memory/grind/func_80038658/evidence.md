# Evidence bank — func_80038658

## [s1] 2026-08-20 — recon session, MATCHED (floor 27 → 0)

**Function identity.** 55-insn CD-load/save completion handler in
`src/code6cac_c_mid.c`. Dispatches on `D_800A31F4` (the same state global the
matched sibling `func_800383A4` drives): state **4** = reap read, state **6** =
reap write. Calls `func_800378A8()` (status reap, returns 0 = still pending),
`close(D_800A3794)`, and for the successful write path `func_8003800C(&D_800F34D8)`;
posts result codes 1/2/3/4/5/6/0xF to `D_800A379E` (s16, gp-rel) and clears
`D_800A31F4` on completion. Caller `func_80038734` (already matched, directly
below) runs `func_800383A4(); func_80038658(); return D_800A379E;`.

**Measurement ladder (all sandbox `--disable all`, 55-target):**
| form | score | insns | note |
|---|---|---|---|
| m2c shared-accumulator + compare-reuse (`var_v0=1; if (s0 != var_v0)`) | 27 | 55 | reproduces the pre-migration pinned floor exactly |
| shared-accumulator goto form, direct compares | 9 | 55 | accumulator pseudo → $v1 (greg: conflicts hard reg 2) |
| + sense-corrected branches + init-from-call staging | 10 | 55 | staging collapsed by cse; still $v1 |
| natural switch, per-arm stores, inline `D_800A379E=1/4; return;` | 11 | 58 | all $v0 ✓ but ret==0 tails never cross-jump-merge |
| natural switch + `fail` var assigned BEFORE the ret==0 branch | 18 | 56 | cse reuses fail(=1) as the `ret==1` compare const → fail crosses close() → $s0/$s1 cascade |
| natural switch + `fail` assigned INSIDE the if-arm (candidate.c) | **0** | 55 | **MATCH** |

**Load-bearing mechanisms (dump-verified):**
1. **Accumulator-in-$v0 is NOT one variable.** Target's seven `li v0,N` +
   three `sh v0` are independent per-arm temps, each first-free-allocated to
   $v0, with the shared `sh .L8003870C` produced by jump2 cross-jump from the
   per-arm `D_800A379E = N;` stores. Any C-level shared accumulator becomes a
   multi-def pseudo that .greg shows conflicting with hard reg 2 → $v1.
2. **Branch sense of the 2/3 select:** `if (ret == 1) {2} else {3}` emits
   target's `bne → else(delay 3); j (delay 2)`. The `!= 1` spelling is
   jump-inverted into the mirror image.
3. **The ret==0 fail path is a source-level shared end label**
   (`fail_store: D_800A379E = fail;` after the switch), NOT a cross-jump
   artifact — inline per-arm stores never merge because the merged block sits
   at the function END. reorg `relax_delay_slots` converts each arm's
   `bnez skip; li; j .L871C` into target's `beqz → .L871C (delay li)`.
4. **`fail = N;` must sit INSIDE the if-arm.** Spelled ahead of the branch,
   cse's extended basic block carries `fail == 1` into the close() path and
   substitutes fail for the `ret == 1` compare constant, extending fail
   across `close()` → callee-save allocation cascade (+s1 save, frame 0x20).

**Artifacts:** `tmp/grind/func_80038658/dumps/` (full `-da` dump set for the
final TU; `.greg` register dispositions cited above), sandbox object at
`tmp/sandbox/func_80038658/code6cac_c_mid.o`.

## s1-fixup (2026-08-20, annotation-fix modality)
- Layer-1 FAIL 2026-08-20 22:49 was CITATION-ONLY (docs/grind/decisions.md:9618): the construct (shared error-store label) is legitimate ordinary C, but the prior self-vet T5 misfiled it under cross-jump-store-tail-merge (mixed exit forms). Correct filing: the shared-end-label constant-fold-defeat recipe, .claude/rules/shared-end-label.md, precedent func_80077B30 (commit 0f206e59, 2026-05-16, same restructure, zero regfix rules).
- Candidate body restored into src/code6cac_c_mid.c UNCHANGED; sandbox func_80038658 --disable all re-measured 0 (55/55) this session (tmp/grind/func_80038658/s1/sandbox0.json).
- self_vet.md T5 + SANCTIONED-FAMILY-CLAIMS parenthetical re-filed accordingly; ANNOTATION-CONFORMANCE remains "n/a — no FAKE construct" (shared-end-label is a plain recipe with no FAKE prerequisites).
