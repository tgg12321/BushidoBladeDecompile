# Hypothesis ledger — _clr

## s1 (2026-09-15, recon)
- H1 **psyz `_clr` transplant with the DR_ENV aggregate (`GpuDrEnv D_800F1858`) is the match** — CONFIRMED: sandbox 0/151 (TU-local and header-canonical spellings both 0); full-tree oracle SHA1 == expected with candidate_merge.patch applied.
- H2 **thirteen per-word `extern u32 D_800F18xx` scalars reproduce the packet stores** — KILLED (instance; chassis HEAD f577f3ea5, no FAKE): 23/152. Mechanism: the tag expression takes `&code[8]` so GCC materialises the address in a register and reload/cse reuses it for the `code[8]` store (`sw $a3,0($a2)`); independent scalars have no address relation, so a fresh `%lo(D_800F187C)($at)` store is emitted and the register/scheduling downstream drifts.
- H3 **`*g_gpu_stat_reg` vs `*D_8009BF48` is a real codegen difference** — KILLED (instance): the normalized diff differs only in the reference side's UNRESOLVED `lui v0,0x0 / lw v0,0(v0)` (D_8009BF48 is a dlabel not in LD_SYM_FILES); same address 0x8009BF48; bytes identical. Score 4 is a scorer artifact, not a residual.

## Frontier
- (none needed for the C) — next session: apply memory/grind/_clr/candidate_merge.patch to a clean HEAD once scope_allow.txt carries `_clr include/gpu.h undefined_syms_auto.txt`, re-measure sandbox 0/151, write self_vet.md (already drafted), return candidate-ready.

## [s1] psyz PsyQ-4.0 `_clr` body transplanted onto BB2 symbols with the DR_ENV buffer declared as one aggregate (`GpuDrEnv D_800F1858`) byte-matches _clr
- mechanism: same Sony source; the aggregate reproduces the address-taken `&code[8]` base register ($a2) reused for the code[8] store and the per-word hi/lo relocs as base+N
- probe: sandbox _clr --disable all (TU-local and header-canonical spellings); verify-oracle --rebuild --allow-dirty with candidate_merge.patch on clean HEAD f577f3ea5
- result: sandbox 0/151 both spellings; build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (metrics/events.jsonl 2026-09-15T08:21:57Z)
- verdict: CONFIRMED

## [s1] thirteen per-word `extern u32 D_800F1858..D_800F1888` scalars (splat's names) reproduce the packet-buffer stores on this chassis with no FAKE constructs
- mechanism: independent scalars have no address relation, so the `&D_800F187C` register cannot be reused for the code[8] store; GCC emits a separate %lo(D_800F187C)($at) store and register/scheduling drifts downstream
- probe: sandbox _clr --disable all with memory/grind/_clr/rejected/per-word-scalars-no-base-reg-reuse.c in place
- result: 23/152 (one extra instruction vs the 151-insn target)
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD f577f3ea5, -mel -msoft-float chassis, no FAKE constructs, per-word scalar declarations TU-local in display.c

## [s1] reading the GPU status word as `*g_gpu_stat_reg` instead of `*D_8009BF48` (same address 0x8009BF48) changes the generated code for _clr on this chassis
- mechanism: suspected declaration-type difference (volatile u32 * vs volatile s32 *)
- probe: normalized-insn diff of build vs reference (tmp/grind/_clr/s1/diff.py) after sandbox scored 4
- result: the only differing lines are the REFERENCE side's unresolved `lui v0,0x0 / lw v0,0(v0)` for D_8009BF48 (a dlabel in asm/data/7D920.data.s not in LD_SYM_FILES) vs the build's resolved pair; instruction stream otherwise identical; the 4 is the known scorer symtab blind spot, not codegen
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD f577f3ea5, no FAKE constructs, TU-local GpuDrEnv form

## s2 (2026-09-15, structural) -- re-verification on HEAD 8423e1cbe; filing route via ruling-request
- H4 **the banked candidate_merge.patch still byte-matches on the current chassis (HEAD 8423e1cbe, -mel -msoft-float)** -- CONFIRMED: `git apply --check` clean; `git apply` then `sandbox _clr --disable all` -> score 0, target_insns 151, build_insns 151, rules_dropped 0 (metrics/events.jsonl 2026-09-15T15:05:53Z, session a0763666); `verify-oracle --rebuild --allow-dirty` -> ok=true, build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle. The applied diff is byte-identical to memory/grind/_clr/candidate_merge.patch (diff of `git diff` vs the banked patch is empty). No structural lever was needed: the honest floor is 0 and every structural spelling would be measured against a 0 -- nothing to sweep.
- Route finding: tools/grinder/scope_allow.txt carries NO `_clr` line (grep, this session). The driver's scope check (grind.ps1:1385-1412) runs BEFORE the outcome is read and discards a session with dirt outside src/display.c unless scope_allow.txt grants it, so a candidate-ready carrying include/gpu.h + undefined_syms_auto.txt is discarded unread, and a TU-local candidate-ready fails aggregate-merge prong (d) at layer-1 (func_80063BD0 precedent, decisions.md:27065). The s1 handoff entry (decisions.md:27134) was filed but the driver was killed by a host restart (commit 8423e1cbe) before any Judge ruling; scope_allow.txt is driver-written ONLY on a Judge ESCALATE(integration-handoff) verdict (grind.ps1:487-512, grindlib.add_scope_allow), and func_80063BD0 obtained that verdict through a `ruling-request` outcome naming its filed handoff (decisions.md:27091 "ruling: INTEGRATION HANDOFF for func_80063BD0 ..."). Therefore s2 returns `ruling-request` with the same shape; the tree is reverted clean (build files untouched at session end) and the exact tree lives in candidate_merge.patch.

## Frontier
- After the Judge ESCALATE(integration-handoff) grants `_clr include/gpu.h undefined_syms_auto.txt`: `git apply memory/grind/_clr/candidate_merge.patch`; `sandbox _clr --disable all` -> 0/151; self_vet.md is already conformant; return candidate-ready with the three files dirty (all granted + staged).

## s3 (2026-09-15, permuter modality; driver session 2)
- H5 **deleting the 12 alias rows D_800F185C..D_800F1888 from undefined_syms_auto.txt (the Judge's prong-(c) remedy) leaves the full build byte-identical to the oracle** -- CONFIRMED: `git apply` of the s1 patch + drop_rows.py; sandbox 151/151 insns (score 34 = 17 reference-side unresolved hi/lo pairs on the deleted names, tmp/grind/_clr/s2/normalized_diff.txt; scorer artifact); `verify-oracle --rebuild --allow-dirty` ok=true, SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa. Nothing but asm/funcs/_clr.s (unbuilt after the patch) referenced the deleted names.
- H6 **a permuter campaign has a gradient on this chassis** -- not run (floor already 0 by oracle; the frontier item was a symbol-config edit the permuter cannot express). Recorded as a modality note, not a hypothesis with a verdict.

## Frontier
- candidate-ready this session (tree dirty with the three granted files). If layer-1 or the Judge objects to the sandbox's FALSE 34, the answer is tmp/grind/_clr/s2/normalized_diff.txt + the oracle log: the engine's scorer does not resolve names that were removed from LD_SYM_FILES on the reference side; an engine fix (resolve reference-side relocs against the linked map) would make it read 0, but engine/ is outside the grind surface.
