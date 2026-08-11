# Hypothesis ledger — func_8003B9D0

## KILLED (session 1)

- **K1 — "The [[defeat-combine-symbol-fold]] pre-computed-displaced-pointer lever closes
  region A."** Mechanism claimed: giving the displaced address its own single-set local
  moves it out of combine's substitution window. Probe: `s16 *eda_alt = eda + 0x226;`
  declared at the top of the `qf & 0x30` block, all three `+0x44C` accesses rewritten as
  `*eda_alt`. Result: sandbox 21 → 21, region A bytes unchanged.
  **KILLED.** The rule's precondition ("at least one call between the pointer's definition
  and the displaced access") does not hold for two of the three sites, and the extra local
  is constant-folded before combine.
  Form banked: `rejected/displaced-pointer-own-local.c`.

- **K2 — "Re-basing the pointer so the wanted-displacement site becomes offset 0 closes
  region A."** Probe: `s16 *eda = &D_80102326;` with `eda[-0x226]` for the low halfword.
  Result: sandbox 21 → **24** (worse); the fold simply migrated to the three now-displaced
  low-halfword sites. **KILLED**, and it generalises: the fold keys on the displacement
  being non-zero, independent of sign and of which symbol names the base. Every
  pointer-rebasing / offset-respelling variant is dead by this measurement — do not
  re-probe that axis.
  Form banked: `rejected/rebase-at-other-end.c`.

- **K3 — "Distinct scoped locals per flag read force cc1 to re-load the flag word
  (region B)."** Mechanism claimed: separate declarations break the CSE equivalence.
  Probe: each flag test wrapped in its own block with its own `u8 *rN = (u8 *)D_800A3878;`.
  Result: region B unchanged — one load, `$v1` reused across both join labels.
  **KILLED.** Corollary established: cc1's CSE table survives these join labels
  (LABEL_NUSES == 1), so "put the reads in different basic blocks" is NOT a lever here.
  Form banked: `rejected/scoped-read-locals.c`.

## CONFIRMED (session 1)

- **C1 — Region B is a memory-invalidation problem, not a control-flow or spelling
  problem.** Probe: moved the function's own `D_800A390F = 0;` statement between the two
  flag `if`s. A second `lbu v0,3(a1)` reload appeared (build_insns 178 → 180) while the
  `lw` of `D_800A3878` stayed shared. So: a write to memory invalidates the *dereference*
  (unknown-alias MEM) and forces the re-read; a write to a different NAMED symbol does not
  invalidate the symbol MEM. Target reloads BOTH the `lw` and the `lbu` at all three sites.
  Score stayed 21 (the `sb` lands in the wrong place), so this exact placement is not the
  answer — but the mechanism is now nailed down.

## LIVE FRONTIER (for the next session)

- **F1 (region B, highest value — it is the whole 7-insn shortfall).** The original C must
  contain, between the flag reads, a store whose address cc1 cannot resolve to a named
  symbol (a store *through a pointer*), which invalidates every MEM including
  `D_800A3878` itself and forces the full lui/lw/lbu reload at each site. Candidate real
  statements already in the function that could legitimately sit there: the
  `D_800A390F = 0;` write expressed through the pointer the function already holds
  (`p[...] = 0` style, if 0x800A390F is reachable as a field of the `D_800A3878` object or
  of another live pointer), or the `eda`/`q` pointer writes of the preceding block being
  ordered later. Next probe: enumerate which of the function's existing stores can be
  legitimately spelled as a pointer store, place each between the reads, and measure — do
  NOT invent a store (that is a dead-store cheat); the store must be one the function
  already performs.
- **F2 (region B, alternative mechanism).** If no honest pointer-store placement produces
  three full reloads, the remaining explanation is that the original reads were
  *volatile-qualified* — which puts this on the [[legitimate-volatile-interrupt-touched]]
  two-prong gate (is `D_800A3878` asynchronously mutated by an identifiable IRQ handler?).
  Next probe: grep every writer of `D_800A3878` across `src/` and `asm/funcs/` and classify
  whether any is IRQ/callback-reached. This is a gate to EVALUATE, not a construct to
  reach for — a negative census closes the axis.
- **F3 (region A).** The fold is a cc1 pass substituting the symbol-valued pseudo into the
  displaced MEM. K1/K2 killed the C-spelling levers, so the next step is forensic, not
  syntactic: run `cc1 <build flags> -da` on the preprocessed TU and read the per-pass RTL
  dumps to identify WHICH pass performs the substitution (`.cse1` vs `.combine`) and what
  its guard condition is. That names the property the base pseudo must lack. Note region A
  and region B may be coupled: `$s0` liveness across the region changes with any region-B
  fix, so re-measure A after any B change before concluding.

## [s1] The [[defeat-combine-symbol-fold]] pre-computed-displaced-pointer lever closes region A (target's `lh/sh 1100(s0)` vs our `lui`+`%lo(sym+0x44C)`).
- mechanism: cc1 substitutes the single-set symbol-valued pseudo into the displaced MEM address and folds it to a constant symbol+K address; giving the displaced address its own local was claimed to move it out of that substitution window.
- probe: Declared `s16 *eda_alt = eda + 0x226;` at the top of the `qf & 0x30` block and rewrote all three +0x44C accesses as `*eda_alt`; sandbox --disable all + normalized-insn diff.
- result: sandbox 21 -> 21, build_insns 178 unchanged, region A bytes identical (all three sites still fold). The rule's stated precondition -- a call between the pointer definition and the displaced access -- holds for only 1 of the 3 sites.
- verdict: KILLED

## [s1] Re-basing the pointer at the other end (base = &D_80102326, low halfword via eda[-0x226]) makes the wanted site offset-0 and closes region A.
- mechanism: If the fold only bites non-zero displacements, putting the wanted access at offset 0 should give register-direct addressing.
- probe: Rewrote the block with `s16 *eda = &D_80102326;` and `eda[-0x226]` for the low halfword; sandbox + diff.
- result: sandbox 21 -> 24 (WORSE). The fold migrated to the three now-displaced low sites (`lui/lh -1100`, `lui/sh -1100`). Confirms the fold keys on the displacement being non-zero, independent of sign and of which symbol names the base -- so every pointer-rebasing / offset-respelling variant is dead, not just this one.
- verdict: KILLED

## [s1] Distinct scoped pointer locals per flag test force cc1 to re-load ((u8 *)D_800A3878)[3] at each site (region B).
- mechanism: Separate declarations in separate blocks were claimed to break the CSE equivalence / extended-basic-block continuation across the two join labels.
- probe: Wrapped each flag test in its own block with its own `u8 *rN = (u8 *)D_800A3878;`; sandbox + diff.
- result: Region B unchanged -- one lui/lw/lbu block, $v1 reused across both joins. Establishes that cc1's CSE table survives these join labels (LABEL_NUSES == 1 at .L8003BB18 / .L8003BB40), so 'separate the reads by scope or control flow' is not a lever here.
- verdict: KILLED

## [s1] Region B is a memory-invalidation problem: a store between the reads forces cc1 to re-load the flag word.
- mechanism: cc1 invalidates cached MEM values on a write; a write through a pointer (unknown address) invalidates every MEM, while a write to a named symbol invalidates only same-symbol MEMs.
- probe: Moved the function's own `D_800A390F = 0;` statement between the two flag `if`s; sandbox + diff.
- result: build_insns 178 -> 180: a second `lbu v0,3(a1)` reload APPEARED. But the `lw` of D_800A3878 stayed CSE'd (D_800A390F is a different named symbol) and the relocated `sb` lands ~20 insns early, so the score stayed 21. Mechanism confirmed; this placement is not the closer. Target reloads BOTH the lw and the lbu at all three sites, so the original's intervening operation must have been a store through a POINTER.
- verdict: CONFIRMED
