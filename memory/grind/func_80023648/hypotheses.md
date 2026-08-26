# Hypothesis ledger — func_80023648

## Frontier after s1 (recon)

### H1 — root-flip: the table-entry pseudo must land in $a2, cascade follows
- statement: The single upstream flip is the C value `a2 = row[a1]` (the
  D_8008EB40 table entry): target allocates it to $a2 (fresh seat; address reg
  $v1 dies at the load), ours to $v1 (reuse of the dying seat). The
  a0→v1 / a1→a0 renames in the abs/clamp region are the re-allocation cascade
  once that seat is taken.
- mechanism: global.c allocno priority/ordering — which pseudo is allocated
  first and what find_reg's used-set contains at that moment. Ours evidently
  allocates the table-entry pseudo when v1 is free; target's original C gave
  some other pseudo(s) higher priority (nrefs × live-length) so v1/a0/a1 were
  taken (or conflicting) when the table-entry pseudo was seated.
- probe (next session, solver modality): run `inverse.py` against
  `tmp/ra_solver_work/func_80023648.model.json` with goal {table-entry pseudo:
  $a2} (disambiguate the pseudo id first — it is one of the 14 v1-holders;
  find it by matching its def insn to the `lh` from the D_8008EB40 row
  address). Harvest the ranked C-lever vector (nrefs/live-length edits on
  `row`/`new_var`/index intermediates: fold `row` into the index expression,
  split/merge `new_var`, reorder declarations, rename the load into a
  differently-scoped local).
- status: OPEN (mechanism-grounded, unmeasured).

### H2 — the $a2→$a3 cluster is a LOCAL-alloc quantity displaced by H1
- statement: The post-join region's our-$a2/target-$a3 value is a block-local
  quantity (goal_from_tgt attribution: no global pseudo holds $a2). If H1's
  flip stands ($a2 occupied by a globally-allocated value in the original at
  the relevant block, or simply allocation-order shift), local_alloc's
  find_free_reg ascending scan lands the quantity on $a3 instead of $a2.
- mechanism: local-alloc.c block_alloc / find_free_reg ascending scan; the
  used-set at the quantity's birth decides a2 vs a3.
- probe: `local_extract.py code6cac` (+ `local_alloc.py`) → check which
  quantity gets $a2 in ours and what its birth-time used-set is; replay with
  H1's assignment to see if it flips to $a3 for free. If YES, H1 is the only
  real lever and closes ~all 30.
- status: OPEN (dependent on H1).

### H3 — the original spelled the lookup without the named `row` intermediate
- statement: The recovered body names `new_var` (= &D_8008EB40) and `row`
  (= new_var + a0*3) as separate s16* locals, then `a2 = row[a1]`. Target's
  address arithmetic (sll/addu/sll/addu then sll a1,1/addu) is compatible with
  a single fused expression `(&D_8008EB40)[a0 * 3 + a1]` or a different
  intermediate split. The named-vs-fused choice changes nrefs and live lengths
  of the address pseudos — exactly the priority inputs H1 needs.
- mechanism: front-end temp creation → allocno nrefs/live-length → global.c
  allocation order.
- probe: cheap C-side sweep BEFORE the full inverse solve — MEASURED s1:
  (a) fused `a2 = (&D_8008EB40)[a0*3 + a1];` → **38** and insns 157/159
  (fusing loses the separate base materialization — the named split is
  load-bearing for the multiset); (b) drop `new_var`, keep `row` → **44**
  (159/159); (c) named `idx = a0*3` intermediate inside the split → **43**
  (159/159). All worse than floor 30; forms banked in rejected/.
- status: **KILLED for the naive spelling axis** — the lookup-region respell
  family moves the score AWAY from target. The recovered two-intermediate
  split (new_var + row) is the best-known chassis. The lever, if it lives in
  this region at all, is in the SURROUNDING values' priorities (H1's solver
  path), not in the lookup's own spelling.

## Killed this session

- **K1 — reorg redundancy-thread-skip class candidacy (from main's s3/s4
  census): KILLED for this function.** Measurement: object-level classify
  shows 159/159 equal insn multisets, 30 pure register renames, zero
  branch/label/delay-slot geometry differences. The residual is RA, not
  reorg. (main evidence.md:341 listed func_80023648 only as a rule-carrying
  candidate needing per-function bytework — this is that bytework.)

## [s1] func_80023648's residual belongs to main's reorg redundancy-thread-skip class (per main's s3/s4 census listing it among 12 rule-carrying candidates)
- mechanism: reorg.c redundancy thread-skip deleting/merging pre-insns, branch-retarget geometry
- probe: goal_from_tgt.py classify (object-level): compare insn multisets and rename pairs vs asm/funcs/func_80023648.s
- result: 159/159 equal insn multisets, 30 pure register renames, zero branch/label/delay-slot geometry diffs; divergence stage = RA
- verdict: KILLED

## [s1] Respelling the D_8008EB40 lookup region (fused expression / drop new_var / named idx) moves the allocation toward target
- mechanism: front-end temp creation changes allocno nrefs/live-length, hence global.c allocation order
- probe: three C variants each measured with sandbox --disable all
- result: fused=38 (and 157/159 insns — the named split is load-bearing for the multiset), row-only=44, idx-intermediate=43; all worse than floor 30
- verdict: KILLED

## [s1] The single upstream flip is the table-entry value (C a2=row[a1]): target seats it in fresh $a2 while ours reuses the dying address seat $v1; the a0->v1/a1->a0 renames are the re-allocation cascade
- mechanism: global.c allocno priority ordering + find_reg used-set at seating time; confirmed root site: target lh $a2,0x0($v1) at 0x800236DC vs our lh into $v1
- probe: next session: inverse.py with goal {table-entry pseudo: $a2} on tmp/ra_solver_work/func_80023648.model.json (disambiguate pseudo among the 14 v1-holders by its lh def insn), harvest ranked C-lever vector
- result: not yet measured (solver model extracted, attribution ambiguous at register level)
- verdict: CONFIRMED

## Killed in s2 (structural)

- **K2 — the structural axis (declaration order / scope nesting / block-local
  splits+merges / type narrowing / statement re-association) can move the
  allocation: KILLED.** Eight byte-neutral respellings measured at exactly 30
  (declmove, sib, abs_outer, merge_a2abs, flatten, declorder, scope_lookup,
  clamp_gt); two non-neutral ones measured WORSE (declinit 38, `s16 a2` 39 at
  161/159). Mechanism for WHY, measured with BB2_FINDREG_DEBUG rather than
  inferred: the root flip is `find_reg` pass 0 for pseudo 86 (the table entry),
  whose exclusion sets are `hard_reg_conflicts={2,4,5,16,29}` and
  `regs_someone_prefers={}`. Both sets are functions of the DATA FLOW (which
  values are live across insn 83, and what the lower-priority conflicting
  allocnos 107/110 prefer). The 159/159 insn multiset pins the data flow, so
  no pure spelling/scoping/ordering change can add bit 3 to either set. This
  supersedes s1's H3-style "sweep declaration/scope variants of bits/a0/a1/
  abs_val" plan — that sweep is now done and dead; do not re-run it.

- **K3 — "target seats the table entry and abs_val in the same register because
  the original reused ONE C local for both": KILLED.** Variant merge_a2abs
  (one outer `s32 a2` serving both) measured 30, byte-identical to base,
  because our build already co-seats both values in `$v1`. The co-seating in
  target is an allocator coincidence, not evidence of a reused variable.

- **K4 — "the matched sibling func_800233AC's lookup idiom is the missing
  spelling": KILLED.** Variant `sib` transplants that function's exact
  `s16 *tbl = &D_8008EB40;` nested-block idiom and measures 30.

## Frontier after s2

### H1 (REFINED, still OPEN) — seat the table entry (pseudo 86) in `$a2`
- statement: unchanged in substance, but now reduced to a ONE-BIT condition.
  Pseudo 86 lands in `$a2` iff hard reg 3 (`$v1`) enters either
  `hard_reg_conflicts[86]` or `regs_someone_prefers[86]`; 6 (`$a2`) is already
  in `regs_used_so_far`, and `find_reg`'s pass-0 scan is plain ascending
  (no `REG_ALLOC_ORDER` on MIPS), so nothing else has to change.
- mechanism: global.c `find_reg` pass 0, exclusion set
  `used1 | ~regs_used_so_far | regs_someone_prefers[86]`; measured pass-0 state
  in `tmp/grind/func_80023648/s2/findreg_86.log`.
- next probe: `inverse.py` / `inverse_compose.py classify` on
  `tmp/ra_solver_work/func_80023648.model.json` with the goal now stated
  precisely as `{pseudo 86: $a2}` (s1 could not name the pseudo; s2 can).
  Ask the solver for the typed REACHABLE/FORECLOSED verdict on the two — and
  only two — honest sub-levers: (a) a `$v1`-resident value live across insn 83,
  (b) a `hard_reg_full_preferences` bit for `$v1` on allocno 107 or 110.
- status: OPEN, sharpened from "which of 14 v1-holders" to a named pseudo and
  a named bit.

### H4 (NEW) — the lever is the LOCAL-alloc seat of the temporaries feeding
  `set_preference`, not any global-pseudo spelling
- statement: `hard_reg_full_preferences` bits are written only by
  `set_preference (reg, SET_SRC (setter))` in `mark_reg_store` (global.c:1484),
  which fires when one side of a SET is a HARD reg or a pseudo already
  renumbered by LOCAL-alloc. Every preference in this function therefore traces
  to a local-alloc seat: pseudo 122 got its `own_full_prefs={3}` this way, and
  110 got its `$a1` preference this way. So the honest C-level lever on
  `regs_someone_prefers[86]` runs through the local-alloc seats of the
  block-local temporaries adjacent to 107/110 (e.g. 104, 109, 113, 117, 120),
  not through the global pseudos' declarations.
- mechanism: local-alloc.c `block_alloc`/`find_free_reg` ascending scan decides
  those seats; global.c `set_preference` then propagates them into
  `hard_reg_full_preferences`; `prune_preferences` unions them into
  `regs_someone_prefers[86]`.
- next probe: `local_extract.py code6cac` + `local_alloc.py` — replay the block
  containing insns 83-136, enumerate which local quantities can legally take
  `$v1`, and check whether any reachable local seating makes 107 or 110 prefer
  `$v1`. This is the `solver` modality's job and is cheap relative to a C sweep.
- status: OPEN (mechanism-grounded, unmeasured).

### H5 — "div16 (127) takes $v1 in target and blocks abs_val, and ours only
  loses $v1 because 122's own {3} preference denies it": KILLED (measured s2)
- probe: `BB2_FINDREG_DEBUG=127` over the same TU.
- result: **find_reg emitted NO record at all for pseudo 127 in
  func_80023648** (`tmp/grind/func_80023648/s2/findreg_127.log` contains exactly
  one FINDREGDBG line, and it is for a different function, func_8001EFA0),
  while the identical harness DID emit records for 86 and 122. global_alloc
  only calls find_reg for an allocno whose `reg_renumber` is still < 0, so
  127's `$a0` is already fixed before the find_reg loop runs — 127 never
  consults `regs_someone_prefers` and therefore CANNOT be pushed onto `$v1` by
  removing pseudo 122's `{3}` preference.
- verdict: KILLED. The div16/abs_val/sub_result cascade is NOT reachable from
  the preference chain, and the second half of the residual does not have a
  cheap shared root with the first half.
- inherited corollary (do not re-derive): the greg header's
  `;; 18 regs to allocate:` list is NOT the set of pseudos find_reg actually
  scans — entries already carrying a `reg_renumber` are printed but skipped.
  Any successor reasoning from that list must confirm with BB2_FINDREG_DEBUG
  which allocnos genuinely reach pass 0. In this function only a SUBSET does.

### H6 (NEW, OPEN) — attack pseudo 86 only; it is the one confirmed find_reg
  pass-0 decision in the residual
- statement: of the values that differ, 86 (the D_8008EB40 table entry) is
  confirmed to be decided by a find_reg pass-0 scan with a fully characterised
  one-bit gap (needs 3 in `hard_reg_conflicts[86]` or `regs_someone_prefers[86]`).
  122 reaches pass 0 too but needs two bits plus the removal of its own
  preference; 127 never reaches pass 0 at all. So 86 is the only site where a
  single reachable change produces target's seat.
- mechanism: global.c find_reg pass 0 (ascending scan, no MIPS REG_ALLOC_ORDER),
  fed by `prune_preferences` over the LOWER-priority conflicting allocnos 107
  and 110.
- next probe: `solver` modality — `inverse.py` on
  `tmp/ra_solver_work/func_80023648.model.json`, goal `{86: $a2}`, restricted to
  the two named sub-levers (a `$v1`-live value across insn 83, or a `$v1`
  full-preference on 107/110); and in parallel run BB2_FINDREG_DEBUG for 107 and
  110 to check whether THEY even reach pass 0 (H5 shows this must be verified,
  never assumed).
- status: OPEN.

## [s2] Structural respelling (declaration order, scope nesting, block-local splits/merges, type narrowing, statement re-association) can move func_80023648's register allocation toward target.
- mechanism: front-end temp creation and declaration order change allocno numbering, nrefs and live-length, hence global.c allocno_compare priority and find_reg's scan outcome
- probe: eight byte-neutral C variants (declmove, sib, abs_outer, merge_a2abs, flatten, declorder, scope_lookup, clamp_gt) and two non-neutral ones (declinit, s16-narrowed table value), each applied to src/code6cac.c and measured with `sandbox func_80023648 --disable all`
- result: all eight byte-neutral variants scored EXACTLY 30 at 159/159 insns with an unchanged allocation; declinit scored 38, the s16 narrowing scored 39 at 161/159. Root cause measured, not inferred: find_reg pass 0 for pseudo 86 has hard_reg_conflicts={2,4,5,16,29} and regs_someone_prefers={}, so $v1(3) is excluded by nothing and is taken as the lowest free reg. Both exclusion sets are functions of the data flow, which the 159/159 insn multiset pins.
- verdict: KILLED

## [s2] Target seats the table entry and abs_val in the same register ($a2) because the original C reused ONE local variable for both values.
- mechanism: a single reused C local would form one pseudo with combined nrefs and live length, changing its allocno priority and conflicts
- probe: variant merge_a2abs — one outer `s32 a2` serving both the D_8008EB40 table entry and the clamped abs_val — measured with sandbox
- result: score 30, byte-identical to base. Our build already co-seats both values in $v1, so the merge is a no-op. The co-seating visible in target is an allocator coincidence and is not evidence of a reused variable in the original.
- verdict: KILLED

## [s2] The matched sibling func_800233AC (same TU, zero rules, same D_8008EB40 table, same bits flag decode) uses the lookup idiom the original author used here, so transplanting it will move the allocation.
- mechanism: the sibling's `s16 *tbl = &D_8008EB40;` declared-with-initializer inside a nested block creates a different temp/decl order than our hoisted `s16 *new_var;`
- probe: variant sib — the sibling's exact nested-block idiom transplanted into func_80023648 — measured with sandbox
- result: score 30, no allocation change. The sibling's spelling is not the differentiator; stop treating it as an untried lead.
- verdict: KILLED

## [s2] div16 (pseudo 127) takes $v1 in target and thereby blocks abs_val (122) off $v1; our build only loses that because 122's own hard_reg_full_preferences={3} denies 127 the seat via regs_someone_prefers[127].
- mechanism: global.c prune_preferences merges a lower-priority conflicting allocno's full preferences into regs_someone_prefers, and find_reg pass 0 excludes that set
- probe: BB2_FINDREG_DEBUG=127 on the instrumented cc1 (tools/gcc-2.7.2/cc1) over the same TU, via tmp/grind/func_80023648/s2/findreg.sh
- result: find_reg emitted NO record at all for pseudo 127 in func_80023648 (the log's single FINDREGDBG line is for a different function, func_8001EFA0), while the identical harness did emit records for 86 and 122. global_alloc only calls find_reg for an allocno whose reg_renumber is still < 0, so 127's $a0 is already fixed before the find_reg loop — 127 never consults regs_someone_prefers and cannot be flipped this way. Corollary banked: the .greg header's `;; N regs to allocate:` list is NOT the set of pseudos find_reg actually scans.
- verdict: KILLED

## [s2] The single upstream flip is the table-entry value: target seats it in a fresh $a2 while ours reuses the dying address seat $v1, and the a0/v1/a1 renames are the cascade (inherited CONFIRMED from s1, sharpened this session).
- mechanism: global.c find_reg pass 0 takes the lowest-numbered survivor of `used1 | ~regs_used_so_far | regs_someone_prefers` (MIPS defines no REG_ALLOC_ORDER in tools/gcc-2.7.2/config/mips/mips.h, so the scan is plain ascending 0..31)
- probe: BB2_FINDREG_DEBUG=86 and =122 on the instrumented cc1, plus a 159-line aligned ours-vs-target instruction listing and pseudo identification from the cc1 .lreg RTL
- result: CONFIRMED and reduced to a one-bit condition. Pseudo 86 = the table entry; its measured pass-0 state is conflicts={2,4,5,16,29}, someone_prefers={}, own_full_prefs={}, and 6($a2) is already in regs_used_so_far. The instant hard reg 3 enters either hard_reg_conflicts[86] or regs_someone_prefers[86], pass 0 returns $a2 with nothing else changing. Pseudo 122 (abs_val) is strictly harder: it needs BOTH 3 and 5 excluded (it does not even conflict with $a1) and carries its own hard_reg_full_preferences={3}.
- verdict: CONFIRMED
