# Hypothesis ledger - func_80089A48

## s1 (recon, 2026-09-06)
- H1 CONFIRMED - declaration fix: D_800F7298 typed as a struct with `volatile u16 raw[0x100]` and indexed `.raw[addr]` (register-base addressing) closes the whole 45-point residual: 0/178. Measured: v3 cast form `((volatile u16 *)D_800F7298)[addr]` 0/178 (same mechanism, redundant cast - not chosen); v5/final struct form 0/178.
- H2 KILLED (instance) - shared pointer-select pre-read (`rxx = flag ? shadow : mmio; var_t0 = ...` once, then per-arm re-select): 70/172; cse follows the case-dispatch branches and hoists the index shifts. rejected/ptr-select-shared-preread-score70.c.
- H3 KILLED (instance) - local pointer `rxx = D_800F7298` assigned per arm with the pre-read duplicated: 61/176. rejected/ptr-local-per-arm-score61.c.
- H4 KILLED (instance) - `*(&D_800F7298[addr])` address-of spelling: 45/178, identical bytes to the array form (expand folds it back to (plus reg sym)). rejected/addrof-element-score45.c.
- H5 KILLED (instance) - non-volatile `u16 raw[]` member: 53/177.

## s2 (recon, 2026-09-06)
- H6 KILLED (instance) - `*(D_800F7298 + addr)` pointer-arithmetic spelling on the HEAD `volatile u16[]` decl: 45/178, byte-identical to arr[addr]. rejected/decay-ptr-arith-score45.c.
- H7 KILLED (instance) - complete-size array `extern volatile u16 D_800F7298[0xCE]`: 45/178. rejected/sized-array-0xCE-score45.c.
- H8 CONFIRMED - SOTN header-canonical `union SpuUnion { SpuRXX rxx; volatile u16 raw[0x100]; }` typing of D_800F7298 (libspu_internal.h:165-170) with `.raw[addr]` access, D_800F7420 untouched: 0/178. candidate.c. Distinct from the banned single-member wrap in type (two real views, the record view being main.c's existing SpuRXX shared with the MMIO block) and in evidence (psyz 4.0 struct-typed _spu_RQ; .bss layout) - filed as ruling-request, not self-approved.
- H9 KILLED (instance) - aggregate merge of D_800F7420 into the union (`D_800F7298.rxx.key_on[k]` in SpuSetKey, `.raw[0xC4+channel]` in _spu_init): target 0/178 but SpuSetKey 14/127 (offset-0 register-base form lost). rejected/union-merge-D800F7420-spusetkey-score14.c.
- FRONTIER if the ruling refuses aggregate typing: the residual is the per-block address pseudo (target a0 in prologue, a1 in arms; params displaced to t1/t0). Only C-level ways measured so far to get a fresh pseudo per block are member-array or cast spellings; a solver classify (`inverse_compose.py classify`) of the a0/a1 seats against the H3 single-pointer form is the next mechanism probe, then per-block-scoped pointer locals as a spelling probe (unmeasured, Sony-plausibility doubtful).

## s3 (recon, 2026-09-06)
- H8 CONFIRMED (re-measured in tree, submitted) - Judge-cleared SpuUnion body applied to src/main.c: sandbox 0/178, canonical pure-C, body hash 3d9403702d133b3a == clearance. candidate-ready. No new hypotheses needed; frontier empty pending driver bytes + FINAL CALL.

## s4 (recon, 2026-09-06)
- H8 CONFIRMED (re-measured in tree, resubmitted) - identical Judge-cleared SpuUnion body: sandbox 0/178, canonical pure-C, body hash 3d9403702d133b3a == clearance. s3's discard was a self-vet tripwire false positive (CONSTRUCTS line echoed the ban's tokens), fixed in self_vet.md and verified with `grindlib.py selfvet` (exit 0). candidate-ready. Frontier empty pending driver bytes + FINAL CALL.
