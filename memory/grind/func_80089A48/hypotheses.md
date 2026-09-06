# Hypothesis ledger - func_80089A48

## s1 (recon, 2026-09-06)
- H1 CONFIRMED - declaration fix: D_800F7298 typed as a struct with `volatile u16 raw[0x100]` and indexed `.raw[addr]` (register-base addressing) closes the whole 45-point residual: 0/178. Measured: v3 cast form `((volatile u16 *)D_800F7298)[addr]` 0/178 (same mechanism, redundant cast - not chosen); v5/final struct form 0/178.
- H2 KILLED (instance) - shared pointer-select pre-read (`rxx = flag ? shadow : mmio; var_t0 = ...` once, then per-arm re-select): 70/172; cse follows the case-dispatch branches and hoists the index shifts. rejected/ptr-select-shared-preread-score70.c.
- H3 KILLED (instance) - local pointer `rxx = D_800F7298` assigned per arm with the pre-read duplicated: 61/176. rejected/ptr-local-per-arm-score61.c.
- H4 KILLED (instance) - `*(&D_800F7298[addr])` address-of spelling: 45/178, identical bytes to the array form (expand folds it back to (plus reg sym)). rejected/addrof-element-score45.c.
- H5 KILLED (instance) - non-volatile `u16 raw[]` member: 53/177.
