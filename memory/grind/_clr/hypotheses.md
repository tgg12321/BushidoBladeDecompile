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
