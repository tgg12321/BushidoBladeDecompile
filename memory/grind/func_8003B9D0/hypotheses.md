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

## SESSION 2 (structural, 2026-08-11)

> PROVENANCE NOTE: an earlier session-2 attempt on this function was DISCARDED by
> the driver (it never wrote its outcome JSON), but it left `candidate.c` and two
> `rejected/` forms on disk uncommitted. This session re-applied that candidate to
> `src/code6cac_c2.c` and INDEPENDENTLY RE-MEASURED it (sandbox --disable all == 6,
> build_insns 188) before continuing, so H1 below is banked on a measurement taken
> THIS session, not on the discarded session's claim.

### CONFIRMED

- **H1 — Writing the two flag-selected argument initialisations as `if/else`
  instead of "init to -1, then conditionally overwrite" closes REGION B entirely
  (the whole 7-instruction shortfall).**
  Form:
  ```c
  if (((u8 *)D_800A3878)[3] & 0x1) a3_arg = D_80101EDA; else a3_arg = -1;
  if (((u8 *)D_800A3878)[3] & 0x2) a0_arg = D_80102326; else a0_arg = -1;
  ```
  instead of `a3_arg = -1; if (...) a3_arg = D_80101EDA;`.
  Mechanism: the `else` arm makes cc1 emit a jump around it, so each join label is
  preceded by a BARRIER and by a real arm block. `cse_end_of_basic_block`
  (tools/gcc-2.7.2/cse.c:8039) ends a CSE basic block at every CODE_LABEL and only
  extends past one through the follow-jumps / skip-blocks branch at cse.c:8102-8184.
  The plain-`if` form satisfies that extension (AROUND status), so ONE CSE table
  spanned all three flag reads and cc1 emitted the `lui/lw/nop/lbu/nop` block once.
  The `if/else` form ends the CSE block at each join, so every read gets a fresh
  CSE table and cc1 re-emits the full reload — exactly target's tgt[72..77] /
  tgt[82..87] / tgt[92..96].
  Result: sandbox 21 -> **6**, build_insns 178 -> 188 (target 185), and the three
  session-1 cheat-asm constructs (the `eda` identity-reload barrier and the two
  `__asm__ __volatile__("" ::: "memory")` barriers) became UNNECESSARY and are
  deleted. **CONFIRMED, re-measured this session.**
  This also supersedes session-1 frontier item F1 (which hypothesised an
  intervening pointer store) and F2 (the volatile axis): region B needed neither.
  Form banked: `candidate.c`.

- **H2 — cc1's REGION-A fold is performed by `cse`'s `find_best_addr`, is
  UNCONDITIONAL for a non-REG address, and is driven by `qty_const`.**
  Read directly out of the frozen compiler source and cross-checked against the
  RTL dumps in `tmp/grind/func_8003B9D0/s2/`:
  * `fold_rtx` MEM case calls `find_best_addr` (cse.c:5029-5034).
  * `find_best_addr` returns early only for frame/arg-pointer addresses and for
    `CONSTANT_ADDRESS_P` (cse.c:2646-2657); otherwise, for any address that is
    **not** a bare REG, it does `validate_change (insn, loc, fold_rtx (addr, insn), 0)`
    (cse.c:2659-2663) with **no cost test whatsoever**.
  * `fold_rtx`'s operand loop substitutes a register's `qty_const` (cse.c:5171-5179).
  * `.cse` dump confirms it exactly: insn 89 `(set (reg/v:SI 94) (symbol_ref "D_80101EDA"))`
    with a `REG_EQUAL` note sets `qty_const`; insn 92 keeps `(mem:HI (reg 94))`
    (offset 0, addr IS a REG, so the fold branch is skipped); insn 97 becomes
    `(mem/s:HI (const:SI (plus:SI (symbol_ref "D_80101EDA") (const_int 1100))))`.
  **CONFIRMED.** Two corollaries that CLOSE axes:
  (a) there is no cost/ADDRESS_COST route to prevent it, so no "make the symbolic
      form look more expensive" spelling can work;
  (b) the fold happens BEFORE `find_best_addr`'s `addr_volatile` check
      (cse.c:2668-2675), so **volatile-qualifying the base or the accesses could
      not suppress it either** — the volatile axis is dead for region A on
      mechanism grounds, independently of the [[legitimate-volatile-interrupt-touched]]
      gate.

### KILLED

- **K4 — "Give the far halfword its own pointer at its own symbol (`s16 *edb =
  &D_80102326;`) so both pointers are used at displacement 0."**
  Probe: two independent base pointers, all six accesses at offset 0.
  Result: sandbox 6 -> **17** (much worse), build_insns 188 -> 189.
  **KILLED.** The second `la` is two always-executed insns and a second live
  symbol-address pointer across `func_8003AFFC()` costs a second callee-saved
  register, perturbing the surrounding allocation. Target holds exactly ONE base
  register, so removing the displacement also removes the shared base — the wrong
  direction. Form banked: `rejected/two-independent-pointers.c`.

- **K5 — "m2c renders the block as plain global member accesses
  (`D_80101EDA.unk0` / `.unk44C`), so the original C had no pointer variable."**
  Probe: dropped the pointer entirely; six plain global reads/writes.
  Result: sandbox 6 -> **23** (much worse), build_insns 188 -> 187; every access
  assembles to its own lui/%lo pair.
  **KILLED**, and it proves the converse: GCC 2.7.2 has NO pass that hoists a
  repeated symbolic address into a shared base register (`find_best_addr` bails
  immediately on `CONSTANT_ADDRESS_P`, cse.c:2656; nothing in `cse_insn` /
  `cse_process_notes` materialises a constant into a fresh pseudo). Therefore
  target's `la $s0, D_80101EDA` + `0($s0)`/`1100($s0)` shape CAN ONLY come from a
  pointer variable in the C — the candidate's shape is right, and m2c's `SYM.unkNNN`
  rendering carries no evidence about the C spelling here.
  Form banked: `rejected/plain-global-accesses-no-pointer.c`.

- **K6 — "Put the `la` in an earlier cse basic block so the base pseudo's constant
  is unknown at the displaced use."** (Structural closure of the boundary axis;
  the hoist probe itself is banked in `rejected/eda-hoisted-out-of-qf-block.c`,
  measured 6 -> 10.)
  **KILLED ON MECHANISM, not just on that one probe.** In target the base is
  materialised at tgt[55..56] (`lui s0` / `addiu s0`) and the FIRST displaced use
  is tgt[58], with only `lh s1,0(s0)` (tgt[57]) in between — no branch, no label,
  no call. A cse basic-block boundary between the definition and the first
  displaced use is therefore geometrically impossible in target's own instruction
  stream. Any future probe of the form "separate the pointer definition from the
  displaced access by control flow" is dead before it is written.

- **K7 (re-measure of session-1 K1, run by the discarded session and left banked)
  — the [[defeat-combine-symbol-fold]] "own local for the displaced address"
  lever, re-run AFTER region B was closed** (session 1's F3 asked for exactly this
  re-measure because regions A and B might be coupled through `$s0` liveness).
  Result: sandbox 6 -> 6, build_insns 188 -> 188, region A bytes unchanged.
  **KILLED — K1 is dead independently of region B; the two regions are NOT coupled.**
  Form banked: `rejected/displaced-pointer-own-local-remeasured.c`.

### LIVE FRONTIER (for the next session)

- **F1 (region A — the whole remaining 6 points).** Per H2 the fold fires iff the
  base pseudo has a `qty_const` at the displaced MEM, and cse.c:5171-5176 skips the
  substitution only when `qty_const` is `REG` or `PLUS` (a bare PLUS, NOT a
  `(const (plus ...))` — the latter is what `eda + 0x226` canonicalises to, which
  is why K1/K7 fold). So the exact property target's base pseudo must have is:
  **its cse quantity must carry no constant, or carry one whose RTX code is REG or
  bare PLUS.** Next probe: instrument or read the `.cse` dump for a MATCHED sibling
  in this TU that already emits register+displacement addressing off a symbol base,
  find what its base pseudo's quantity looks like, and derive the C shape from
  that. `grep -l '[0-9]\+(\$s[0-9])' asm/funcs/*.s` cross-referenced against
  functions with zero rules is the cheap way to find such a sibling.

- **F2 (region A, alternative).** Determine empirically where `qty_const` can
  legitimately end up as a REG: `insert` sets it from a class member with
  `is_const` (cse.c:1377-1397), so a pointer whose class is joined to another
  register's class BEFORE the constant is seen is the candidate shape. Next probe:
  build a minimal standalone .c reproducing the `la` + `0(base)` + `1100(base)`
  shape and sweep pointer-initialisation spellings against `-da` `.cse` dumps until
  one keeps `(mem (plus (reg) (const_int 1100)))`; only then port it back. Doing
  this in a 40-line scratch TU is far cheaper than sandbox round-trips on the real
  file.

- **F3 (orthogonal, evidence-backed, does NOT affect region A).** m2c infers
  `func_8003AFFC(D_800A3878)` — i.e. the callee takes the `D_800A3878` object
  pointer as an argument (`$a0` holds it live from tgt[48] through the `jal` at
  tgt[68] with no reload). Our C calls it with no arguments. This is a genuine
  semantic difference in the decompilation, not a codegen trick; it is currently
  byte-neutral here but should be corrected if the prototype surface allows, and
  it may matter for `func_8003AFFC` itself when that function is worked.

## [s2] Writing the two flag-selected argument initialisations as if/else (`if (flag) a3_arg = D_80101EDA; else a3_arg = -1;`) instead of `a3_arg = -1; if (flag) a3_arg = D_80101EDA;` closes REGION B, i.e. makes cc1 re-emit the full lui/lw/nop/lbu/nop reload of ((u8 *)D_800A3878)[3] at each of the three flag-test sites, exactly as target does.
- mechanism: An else arm makes cc1 emit a jump around it, so each join CODE_LABEL is preceded by a BARRIER and by a real arm block. cse_end_of_basic_block (tools/gcc-2.7.2/cse.c:8039) ends a CSE basic block at every CODE_LABEL and only extends past one via the follow-jumps / skip-blocks branch at cse.c:8102-8184. The plain-if form satisfies that extension (AROUND status), so ONE CSE table spanned all three flag reads and the reload block was emitted once; the if/else form ends the CSE block at each join, so every read gets a fresh CSE table and the full reload is re-emitted.
- probe: Applied the form to src/code6cac_c2.c (also deleting all three session-1 cheat-asm constructs, which became unnecessary), ran `sandbox func_8003B9D0 --disable all`, and re-ran the normalized-insn side-by-side diff against build/src/code6cac_c2.o. Re-measured a second time after every other probe was reverted.
- result: sandbox 21 -> 6; build_insns 178 -> 188 (target 185). The normalized diff is now 58 equal head insns, three isolated 2-insn-vs-1-insn replacements, and a 113-insn byte-equal tail. Region B is completely gone. Session-1 frontier items F1 (intervening pointer store) and F2 (volatile-qualified flag reads) are moot -- region B needed neither, so the IRQ-writer census F2 asked for does not need to be run.
- verdict: CONFIRMED

## [s2] REGION A's symbol fold is performed by cse's find_best_addr, is UNCONDITIONAL for any address that is not a bare REG, and fires solely because the base pseudo carries a qty_const.
- mechanism: fold_rtx's MEM case calls find_best_addr (cse.c:5029-5034). find_best_addr returns early only for frame/arg-pointer addresses and CONSTANT_ADDRESS_P (cse.c:2646-2657); otherwise, for a non-REG address, it does validate_change (insn, loc, fold_rtx (addr, insn), 0) at cse.c:2659-2663 with no cost test at all. fold_rtx substitutes a register operand's qty_const, but only when that qty_const is neither REG nor a bare PLUS (cse.c:5171-5176). qty_const is recorded when a constant enters the register's class (cse.c:1377-1397).
- probe: Read the frozen compiler source (tools/gcc-2.7.2/cse.c) and cross-checked against the per-pass RTL dumps in tmp/grind/func_8003B9D0/s2/ (tu.i.rtl / tu.i.cse).
- result: Confirmed exactly in the .cse dump: insn 89 `(set (reg/v:SI 94) (symbol_ref "D_80101EDA"))` carries a REG_EQUAL note and sets qty_const; insn 92 keeps `(mem:HI (reg 94))` because the offset-0 address IS a bare REG and the fold branch is skipped; insn 97 is rewritten to `(mem/s:HI (const:SI (plus:SI (symbol_ref "D_80101EDA") (const_int 1100))))`. Two axes are closed as corollaries: (a) the fold is not a cost decision, so no 'make the symbolic form look more expensive' spelling can work; (b) the fold at cse.c:2661 runs BEFORE find_best_addr's addr_volatile bail-out at cse.c:2668-2675, so volatile-qualifying the base or the accesses could not suppress it either -- the volatile axis is dead for region A on mechanism grounds, independent of any policy gate.
- verdict: CONFIRMED

## [s2] Giving the far halfword its own pointer at its own splat symbol (`s16 *edb = &D_80102326;`) so that both pointers are used at displacement 0 removes the fold and closes region A.
- mechanism: Session-1 K2 established that the fold follows the non-zero displacement, so making every access offset-0 should make every address a bare REG and skip the fold branch entirely.
- probe: Rewrote the qf & 0x30 block with two independent base pointers, all six accesses at offset 0; sandbox --disable all.
- result: sandbox 6 -> 17 (much WORSE), build_insns 188 -> 189. The second `la` (lui+addiu) is two always-executed insns, and holding two live symbol-address pointers across func_8003AFFC() forces a second callee-saved register and perturbs the surrounding allocation. Target holds exactly ONE base register and reaches the far halfword with `1100($s0)`, so removing the displacement also removes the shared base -- structurally further from target, not closer. Form banked at memory/grind/func_8003B9D0/rejected/two-independent-pointers.c.
- verdict: KILLED

## [s2] m2c reconstructs the block as plain global member accesses (`D_80101EDA.unk0` / `D_80101EDA.unk44C`), so the original C had no pointer variable and writing it that way will reproduce target's shared-base addressing.
- mechanism: If cc1 hoisted a repeated symbolic address into a shared base register, plain global accesses would produce target's `la $s0` + `0($s0)` / `1100($s0)` form directly.
- probe: Dropped the pointer variable entirely; wrote all six accesses as plain global reads/writes of D_80101EDA and D_80102326; sandbox --disable all.
- result: sandbox 6 -> 23 (much WORSE), build_insns 188 -> 187; every access assembles to its own lui/%lo pair and NO shared base register is created. This proves the converse and is the more valuable finding: GCC 2.7.2 has no pass that hoists a repeated symbolic address into a register (find_best_addr bails immediately on CONSTANT_ADDRESS_P, cse.c:2656; nothing in cse_insn / cse_process_notes materialises a constant into a fresh pseudo), so target's shared-base shape can ONLY come from a pointer variable in the C -- the candidate's spelling is right. It also establishes that m2c renders any register-based struct access as `SYM.unkNNN` regardless of the C spelling, so m2c's rendering is not evidence here. Form banked at memory/grind/func_8003B9D0/rejected/plain-global-accesses-no-pointer.c.
- verdict: KILLED

## [s2] Placing the pointer definition in an EARLIER cse basic block than the displaced use would leave the base pseudo without a qty_const and suppress the fold.
- mechanism: cse's new_basic_block clears the quantity tables, so a base defined in a prior block has no constant equivalence at the displaced use; cse_end_of_basic_block only extends past a jump when LABEL_NUSES (JUMP_LABEL (p)) == 1 (cse.c:8102-8106), so a multi-use join label would create a real boundary.
- probe: The hoist-out-of-the-qf-block form was measured (sandbox 6 -> 10, build_insns 188 -> 187, banked at rejected/eda-hoisted-out-of-qf-block.c: the `if (qf & 0x30)` branch is AROUND-extended so no boundary is created, and the la moves out of the if-body). This session closed the axis on geometry instead of on one more probe: read the target instruction stream around the base materialisation.
- result: KILLED ON MECHANISM. In target the base is materialised at tgt[55..56] (`lui s0` / `addiu s0`) and the FIRST displaced use is tgt[58], with only `lh s1,0(s0)` (tgt[57]) in between -- no branch, no label, no call. A cse basic-block boundary between the definition and the first displaced use is geometrically impossible in target's own output, so no C spelling that separates them by control flow can be right. Any future probe of that shape is dead before it is written.
- verdict: KILLED

## SESSION 3 (structural, 2026-08-11)

### CONFIRMED

- **H3 - Region A's fold is suppressed by, and ONLY by, a cse basic-block
  boundary between the base pointer's definition and its displaced use; an
  if/ELSE join label creates such a boundary and a plain `if` does not.**
  Mechanism: `find_best_addr` folds `(plus (reg) (const_int 1100))` whenever the
  base pseudo carries a `qty_const` (session-2 H2).  `cse_end_of_basic_block`
  (cse.c:8039) resets the quantity tables at every CODE_LABEL it cannot extend
  past; the plain-`if` join is AROUND-extended (cse.c:8102-8184), but an if/ELSE
  arm ends in an unconditional jump + BARRIER, so its join label is a real
  boundary and the base pseudo enters the next block with no constant.
  Probe: (a) a 12-variant mini-TU sweep (`tmp/grind/func_8003B9D0/s3/`), (b) the
  real function with `eda` assigned before the 0x80 test and that test spelled
  if/ELSE.
  Result: sandbox **11, build_insns 185 == target 185**; objdump shows target's
  exact `lh s1,1100(s2)` / `sh v0,1100(s2)` / `sh s1,1100(s2)`; the normalized
  diff is a 128-insn byte-equal tail plus two placement clusters.  With the plain
  `if` restored the fold returns (10 / 187).
  **CONFIRMED.  Region A is closable in pure C.**
  Form banked: `rejected/cse-boundary-diamond-closes-region-a-but-moves-magic-and-la.c`.

### KILLED

- **K8 - "Some SPELLING of the displaced access, or of the base pointer's type,
  keeps the register+displacement addressing."**  Probes (mini TU, exact build
  flags, `%lo(D_80101EDA+1100)` vs `1100($reg)` in the cc1 output): s16* array
  index; derived pointer `eda + 0x226`; struct pointer with the member at +1100;
  `u8 *` base with the displacement in bytes inside the cast (the func_8002BC68
  spelling); `u8 *` base plus a derived `u8 *edf = eda + 0x44C`; byte-cast
  `*(s16 *)((u8 *)eda + 0x44C)`; index held in a local `s32 i = 0x226`; pointer
  declared at function top and assigned inside the block; pointer defined before
  the block with an intervening CALL; pointer defined before the block with no
  diamond.  **All ten fold.**  Only the if/ELSE-diamond geometry (and the
  function-parameter control) does not.
  **KILLED - the access-spelling and pointer-type axes are closed; the lever is
  geometry, and only geometry.**

- **K9 - "The MATCHED siblings that emit symbol-base + non-zero-displacement
  addressing use a different C spelling we can copy."**  Probe: new census tool
  `tmp/grind/func_8003B9D0/s3/find_sibling.py` over all `asm/funcs/*.s`,
  intersected with the unqueued + ruleless set -> 60 functions with the shape, 30
  matched.  Read the closest analogue, `func_8002BC68` (`src/code6cac_b.c:745`),
  which uses the SAME 0x44C displacement off the neighbouring symbol D_80101EC8.
  Result: its spelling is the ordinary `u8 *t2_base = &D_80101EC8;` +
  `*((s32 *)(t2_base + 0x134))`, and that spelling FOLDS in our mini TU; what
  makes it survive there is that the pointer is defined at the top of the
  function and its uses come after a large if/else diamond - the SAME
  cse-boundary mechanism as H3.  **KILLED - there is no spelling to copy.**

### LIVE FRONTIER (for the next session)

- **F1 (the whole remaining problem - buy the boundary more cheaply).**  The
  region-A-closing form costs ~7 points on `magic` placement because the only
  available if/ELSE is the `((u8*)D_800A3878)[3] & 0x80` test, and moving
  `magic = 0x80190800` into an else arm contradicts target's prologue
  materialisation (tgt[1..3]).  What is needed is a cse basic-block boundary
  between `eda`'s definition and the `qf & 0x30` block that does NOT relocate
  `magic`.  Next probe: read `cse_end_of_basic_block` (cse.c:8039-8184) and
  enumerate EVERY condition that ends a block or refuses the AROUND extension -
  in particular `LABEL_NUSES (JUMP_LABEL (p)) != 1` (a join label targeted by TWO
  jumps, which is exactly what a short-circuit `&&` / `||` condition produces)
  and the `PATH_SIZE` cap on how many jumps a single extended block may follow.
  Then look for a statement in insns 3..54 whose target bytes are consistent with
  spelling it as a two-branch condition.  This is a search over a SMALL,
  enumerable space, and every candidate is testable in the mini TU first.

- **F2 (the other ~4 points - `la` placement).**  Even with a free boundary the
  base is materialised at mine[38..39] instead of target's tgt[55..56], because
  the pointer's assignment statement must precede the boundary.  Next probe: for
  each boundary form F1 finds, check the `la` position in the normalized diff
  BEFORE scoring - the winning form is one whose boundary sits as late as
  possible, so that `eda`'s assignment can also sit late (ideally immediately
  before the `qf & 0x30` test).

- **F3 (unchanged, orthogonal).**  m2c infers `func_8003AFFC(D_800A3878)`; our C
  calls it with no arguments, and `src/code6cac_c2.c` carries three duplicate
  `extern void func_8003AFFC(void);` declarations (lines 73, 74, 171) that should
  be reconciled to one.  Byte-neutral here; it is about correctness of the
  decompilation and about func_8003AFFC's own future grind.

## [s3] Region A's symbol fold is suppressed by, and only by, a cse basic-block boundary between the base pointer's definition and its displaced use; an if/ELSE join label creates such a boundary and a plain `if` does not.
- mechanism: find_best_addr folds (plus (reg) (const_int 1100)) whenever the base pseudo carries a qty_const (session-2 H2). cse_end_of_basic_block (cse.c:8039) resets the quantity tables at every CODE_LABEL it cannot extend past; a plain-if join is AROUND-extended (cse.c:8102-8184), but an if/ELSE arm ends in an unconditional jump + BARRIER so the join label is a real boundary and the base pseudo enters the next block with no constant.
- probe: 12-variant mini-TU sweep (tmp/grind/func_8003B9D0/s3/mini_*.c + sweep.sh, cpp -> cc1 with the exact build flags), then the real function with `eda` assigned before the 0x80 test and that test spelled if/ELSE; sandbox --disable all + objdump + normalized diff.
- result: sandbox 11 with build_insns 185 == target_insns 185; objdump shows target's exact region-A shape at all three sites (lh s1,1100(s2) / sh v0,1100(s2) / sh s1,1100(s2)); the normalized diff is a 128-insn byte-equal tail plus exactly two placement clusters (magic ~7 points, la ~4 points). Restoring the plain `if` brings the fold straight back (10 / build_insns 187), which also explains session-2's K6 number. Region A is closable in pure C; the open problem is now buying the boundary without paying for magic and la placement.
- verdict: CONFIRMED

## [s3] Some spelling of the displaced access, or of the base pointer's type, keeps target's register+displacement addressing.
- mechanism: If the fold were driven by the shape of the address expression rather than by the base pseudo's cse quantity, a different access spelling (byte offset, struct member, derived pointer, index variable) would avoid it.
- probe: Ten spellings compiled in a ~20-line mini TU with the exact build flags and checked for `%lo(D_80101EDA+1100)` vs `1100($reg)`: s16* array index; derived `eda + 0x226`; struct pointer with the member at +1100; declaration at function top with assignment inside the block; `s32 i = 0x226; eda[i]`; byte-cast `*(s16 *)((u8 *)eda + 0x44C)`; `u8 *` base with byte displacement (the func_8002BC68 spelling); `u8 *` base plus a derived `u8 *edf = eda + 0x44C`; pointer defined before the block with an intervening call; pointer defined before the block with no diamond.
- result: ALL TEN FOLD. Only the if/ELSE-diamond geometry avoids the fold (and the function-parameter control, which is not an available shape here). The access-spelling and pointer-type axes are closed.
- verdict: KILLED

## [s3] The 30 MATCHED sibling functions that emit symbol-base + non-zero-displacement addressing use a C spelling we can copy.
- mechanism: If a proven spelling existed, porting it would close region A directly.
- probe: Wrote tmp/grind/func_8003B9D0/s3/find_sibling.py, which scans every asm/funcs/*.s for `lui %hi(SYM)` + `addiu %lo(SYM)` into a register followed by a non-zero-displacement load/store off that register, and intersects the hits with the unqueued + ruleless (MATCHED) set: 60 functions have the shape, 30 are matched. Read the closest analogue, func_8002BC68 (src/code6cac_b.c:745), which uses the SAME 0x44C displacement off the neighbouring symbol D_80101EC8.
- result: The sibling's spelling is the ordinary `u8 *t2_base = &D_80101EC8;` + `*((s32 *)(t2_base + 0x134))`, which FOLDS in our mini TU. What makes it survive in that function is that the pointer is defined at the top and the uses come after a large if/else diamond -- the same cse-boundary mechanism as H3, not a different spelling. There is no spelling to copy.
- verdict: KILLED

## [s3] Region A's symbol fold is suppressed by, and ONLY by, a cse basic-block boundary between the base pointer's definition and its displaced use; an if/ELSE join label creates such a boundary and a plain `if` does not.
- mechanism: find_best_addr folds (plus (reg) (const_int 1100)) into (const (plus sym 1100)) whenever the base pseudo carries a qty_const (session-2 H2, cse.c:2659-2663 + 5171-5176). cse_end_of_basic_block (cse.c:8039) resets the quantity tables at every CODE_LABEL it cannot extend past; a plain-if join is AROUND-extended (cse.c:8102-8184), but an if/ELSE arm ends in an unconditional jump + BARRIER, so its join label is a real boundary and the base pseudo enters the next block with no constant.
- probe: 12-variant mini-TU sweep (tmp/grind/func_8003B9D0/s3/mini_*.c + sweep.sh: cpp -> cc1 with the exact build flags, counting %lo(D_80101EDA+1100) folds vs 1100($reg)), then the real function with `eda` assigned before the ((u8*)D_800A3878)[3] & 0x80 test and that test spelled if/ELSE; sandbox --disable all + objdump of the sandbox .o + the session-1 normalized-insn diff.
- result: sandbox 11 with build_insns 185 == target_insns 185; objdump shows target's exact region-A shape at all three sites (lh s1,1100(s2) / sh v0,1100(s2) / sh s1,1100(s2)); the normalized diff collapses to a 128-insn byte-equal tail plus exactly two placement clusters (magic ~7 points, la ~4 points). Restoring the plain `if` brings the fold straight back (10 / build_insns 187), which also reproduces and explains session-2's K6 number.
- verdict: CONFIRMED

## [s3] Some spelling of the displaced access, or of the base pointer's type, keeps target's register+displacement addressing.
- mechanism: If the fold were driven by the shape of the address expression rather than by the base pseudo's cse quantity, a different access spelling (byte offset, struct member, derived pointer, index variable) would avoid it.
- probe: Ten spellings compiled in a ~20-line mini TU with the exact build flags and checked for %lo(D_80101EDA+1100) vs 1100($reg): s16* array index; derived `eda + 0x226`; struct pointer with the member at +1100; declaration at function top with assignment inside the block; `s32 i = 0x226; eda[i]`; byte-cast `*(s16 *)((u8 *)eda + 0x44C)`; u8* base with byte displacement (the func_8002BC68 spelling); u8* base plus a derived `u8 *edf = eda + 0x44C`; pointer defined before the block with an intervening call; pointer defined before the block with no diamond.
- result: ALL TEN FOLD. Only the if/ELSE-diamond geometry avoids the fold (and the function-parameter control, which is not an available shape here). The access-spelling and pointer-type axes are closed.
- verdict: KILLED

## [s3] The MATCHED sibling functions that emit symbol-base + non-zero-displacement addressing use a C spelling we can copy.
- mechanism: If a proven spelling existed in the matched corpus, porting it would close region A directly.
- probe: Wrote tmp/grind/func_8003B9D0/s3/find_sibling.py, which scans every asm/funcs/*.s for `lui %hi(SYM)` + `addiu %lo(SYM)` into a register followed by a non-zero-displacement load/store off that register, and intersects the hits with the unqueued + ruleless (MATCHED) set: 60 functions have the shape, 30 are matched. Read the closest analogue, func_8002BC68 (src/code6cac_b.c:745), which uses the SAME 0x44C displacement off the neighbouring symbol D_80101EC8.
- result: The sibling's spelling is the ordinary `u8 *t2_base = &D_80101EC8;` + `*((s32 *)(t2_base + 0x134))`, which FOLDS in our mini TU. What makes it survive in that function is that its pointer is defined at the top and its uses come after a large if/else diamond -- the same cse-boundary mechanism as H3, not a different spelling. There is no spelling to copy.
- verdict: KILLED
