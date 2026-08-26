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

## [s3] s2's KILL of the "div16 blocks $v1" hypothesis was based on the WRONG pseudo id: div16 is pseudo 129, not 127, and 129 IS the first allocno find_reg scans.
- mechanism: s2 took the pseudo ids from the `.greg` `;; N regs to allocate:` header. That header is not the allocation order and its ids do not match this function's allocnos. The instrumented cc1's second hook, `BB2_ALLOC_DEBUG=1` (global.c:601-618), dumps the real order with hard regs, nrefs, live-length and priority.
- probe: `BB2_ALLOC_DEBUG=1` over the same TU (tmp/grind/func_80023648/s3/allocdbg.sh), plus a re-run of `BB2_FINDREG_DEBUG=127` to confirm the null, plus `BB2_FINDREG_DEBUG=129`.
- result: 127 is not an allocno of func_80023648 at all — hence the "NO HIT", which is a naming artefact and not evidence about div16. The real order is `129 122 85 81 72 134 176 75 86 126 153 83 110 135 192 107 158 173`; 129 is ord=0, is scanned by find_reg, and gets $a0. s2's KILL verdict on this hypothesis is REVERSED — the hypothesis was never actually tested.
- verdict: KILLED (as a verdict about pseudo 127) / REOPENED as H9 below (for pseudo 129)

## [s3] The FIRST divergence in this function's register allocation is allocno ord=0 (pseudo 129 = div16 = `*(s16 *)(arg0 + 0x1A)`), and $v1 is denied to it by a single `regs_someone_prefers` bit — a PREFERENCE bit, not a conflict bit.
- mechanism: global.c find_reg pass 0 excludes `hard_reg_conflicts | ~regs_used_so_far | regs_someone_prefers` and returns the lowest survivor; `regs_someone_prefers` is computed in prune_preferences (global.c:882-931) as the union of `hard_reg_full_preferences` over lower-priority CONFLICTING allocnos, and those preference bits are written by `set_preference` (global.c:1671) off the RTL copy structure — which C spelling demonstrably moves.
- probe: `BB2_FINDREG_DEBUG=129` on the instrumented cc1 with the candidate body applied (log tmp/grind/func_80023648/s3/findreg_129.log), cross-read against the ALLOCDBG order table and the s2 value->seat map row "div16 ours $a0 / target $v1".
- result: CONFIRMED. 129's pass-0 state is conflicts={2,29}, someone_prefers={3}, own_copy_prefs={}, own_full_prefs={}, pass0_used={0,1,2,3,16..23,26..31}. $v1(3) is NOT a conflict — it is excluded ONLY by regs_someone_prefers. Clear that one bit and pass 0 returns 3 = $v1 = target's seat for div16, with nothing else changed. Every other allocno in the function is allocated after 129, so this is the true root and pseudo 86 (ord=8) is cascade.
- verdict: CONFIRMED

## [s3] The table-entry pseudo 86 can be driven to $a2 by making more values live across its defining insn (s2's lever A).
- mechanism: global.c find_reg pass 0 is a plain ascending lowest-free-survivor scan, so additional hard-reg conflicts push the chosen seat upward through the register file
- probe: variant `andcond` — the two guard conditions rewritten non-short-circuit as `(D_800A38BA != 0) & (*(s16 *)(arg0 + 6) == 0)`, which forces both guard values live simultaneously across the table-entry load — measured with sandbox and disassembled.
- result: KILLED. Score 47 at an unchanged 159/159 insns. The asm shows both guard temps live (`sltu $v1,$zero,$v1 / sltiu $v0,$v0,1 / and $v1,$v1,$v0`) before `lh $v0,0($a0)` — and the table entry took $v0, not $a2. Landing $a2 (hard reg 6) via conflicts alone requires 2,3,4,5 ALL excluded from 86; 4 and 5 already are, so it is a two-more-bit ask that a fixed 159-insn multiset cannot supply. Adding live values only walks the entry down the ascending list one seat at a time.
- verdict: KILLED

## [s3] `regs_someone_prefers[86]` can be reached through the local-alloc seats of allocnos 107 and 110 (s2's H4).
- mechanism: set_preference writes `hard_reg_full_preferences` when one side of a SET is a hard reg or an already-locally-renumbered pseudo; prune_preferences then unions those into regs_someone_prefers of the higher-priority conflicting allocno
- probe: `BB2_FINDREG_DEBUG=107` and `=110` on the instrumented cc1 (logs tmp/grind/func_80023648/s3/findreg_107.log, findreg_110.log), plus identification of 107 and 110 from the .lreg RTL.
- result: KILLED. 107 = `*(s16 *)(arg0 + 0x1CA)` (insn 102), 110 = the `a2/4` rounding temp (`110 = 86`, then `110 = 86 + 3` under `bgez`, then `109 = 110 >> 2`). BOTH already carry hard reg 3 in `hard_reg_conflicts` — a consequence of 86 taking $v1 earlier — so neither can ever seat $v1 and neither can hand $v1 to anyone. 110's `own_full_prefs` is {5} ($a1, from the `(set (reg 5 a1) (plus ...))` at insn 116 via the copy-preference propagation) and 107's {5} was pruned away by its own $a1 conflict, leaving `own_full_prefs` empty. Neither carries a $v1 bit and neither can acquire one: preference bits only form against a hard reg or a local-alloc'd seat, and no $v1-seated local quantity is paired with 107 or 110 anywhere in this data flow.
- verdict: KILLED

## [s3] Reordering allocno priority (via nrefs / live-length edits) can make find_reg pass 0 skip $v1 under the "never allocate a register for the first time in pass 0" rule.
- mechanism: pass 0 ORs `~regs_used_so_far` into its exclusion set, so a hard reg not yet in regs_used_so_far is skipped in pass 0
- probe: read of global.c:340-372 (the regs_used_so_far seeding) plus the measured `used_so_far` line in every FINDREGDBG record this session.
- result: KILLED. regs_used_so_far is seeded with `regs_ever_live[i] || call_used_regs[i]` for every hard reg BEFORE any allocation. On MIPS $v0-$a3 and $t0-$t9 are all call-used, so 2,3,4,5,6,7 are in the set from the start and stay there — confirmed in every measured record (`used_so_far: 0 1 2 3 4 5 6 7 8 ...`). No priority reordering can make $v1 "not yet used". This closes the whole allocation-order-reshuffle axis as a route to freeing $v1.
- verdict: KILLED

## [s3] C-level operand order is a live lever on GCC 2.7.2's register preferences (and therefore on find_reg pass-0 seats) in this function.
- mechanism: set_preference (global.c:1671) unwraps an 'e'-format SET_SRC exactly once (`src = XEXP (src, 0), copy = 0`), so for `(set (reg 5 a1) (plus (reg X) (reg Y)))` only X — the FIRST plus operand — receives the $a1 preference bit. Swapping the C operand order swaps which pseudo is X.
- probe: variants `swapadd` (`a2 / 4 + *(s16 *)(arg0 + 0x1CA)`), `swapelse` (`a2 + *(s16 *)(arg0 + 0x1D8)`) and `swapboth`, each applied to src/code6cac.c and measured with sandbox, with swapadd disassembled and diffed against target.
- result: CONFIRMED as a lever, REJECTED as a fix. swapadd scores 33 and swapboth 33 (both at an unchanged 159/159), i.e. the allocation genuinely MOVED — the `lh $v0, 0x1CA($s0)` load relocated from before the `bgez` to after it and seats changed; the table entry stayed in $v1. swapelse is byte-neutral at 30. The important consequence is negative-result-proof: s2's conclusion that "spelling cannot reach the allocator here" holds only for hard_reg_conflicts bits and MUST NOT be generalised to preference bits. Forms banked to rejected/s3-swapadd-operand-order-33.c and rejected/s3-condswap-36.c.
- verdict: CONFIRMED

## H9 (LIVE FRONTIER) — `regs_someone_prefers[129]`'s single $v1 bit is contributed by allocno 122 (`abs_val`), whose `hard_reg_full_preferences` is measured to be exactly {3}; removing that contribution seats div16 in $v1 and unwinds the whole cascade.
- mechanism: prune_preferences (global.c:882-931) walks allocnos least-important-first and, for each higher-priority allocno i, unions `hard_reg_full_preferences[j]` for every lower-priority j with CONFLICTP(i,j). 122 is ord=1, exactly one step below 129 (ord=0), and is the ONLY allocno in this function measured to carry a $v1 full-preference bit.
- next probe: (1) PROVE the attribution before spending any C: add a one-line env-gated dump inside prune_preferences printing, for allocno 129, each contributing j and its full-preference set — or, cheaper and with no compiler edit, run `BB2_FINDREG_DEBUG` over EVERY allocno in the ord list and confirm 122 is the unique $v1-preferrer, then confirm CONFLICTP(129,122) by checking whether abs_val's live range overlaps the `*(s16 *)(arg0 + 0x1A)` read in the .lreg RTL. (2) Then attack 122's preference, NOT its seat: find where `hard_reg_full_preferences[122]` acquires bit 3 by locating the SET in the .lreg/.greg RTL that pairs 122 with a $v1-seated local quantity, and re-spell the abs/clamp block so that pairing does not form (candidate spellings: reassociate `*(u16 *)(arg0 + 0x14E) - abs_val`; move the `div16` read above the abs/clamp block so 122 and 129 no longer overlap, which kills the CONFLICTP prong instead of the preference prong; express the clamp as a different comparison so the negate/clamp copy chain changes shape). (3) Every candidate must be measured with sandbox at 159/159 — a form that changes the insn count is not a candidate.

## [s3] s2's KILL of the 'div16 blocks $v1' hypothesis was based on the wrong pseudo id: div16 is pseudo 129, not 127, and 129 IS the first allocno find_reg scans.
- mechanism: s2 took pseudo ids from the .greg ';; N regs to allocate:' header, which is not the allocation order and whose ids do not match this function's allocnos. The instrumented cc1's second hook, BB2_ALLOC_DEBUG=1 (global.c:601-618), dumps the real order with hard regs, nrefs, live-length and priority.
- probe: BB2_ALLOC_DEBUG=1 over the TU (tmp/grind/func_80023648/s3/allocdbg.sh), a re-run of BB2_FINDREG_DEBUG=127 to confirm the null, and BB2_FINDREG_DEBUG=129.
- result: 127 is not an allocno of func_80023648 at all, so s2's 'NO HIT' was a naming artefact and not evidence about div16. Real order: 129 122 85 81 72 134 176 75 86 126 153 83 110 135 192 107 158 173. 129 is ord=0, is scanned by find_reg, and takes $a0. s2's KILL verdict on this hypothesis is reversed - it was never actually tested. Also corrected: 130->134, 131->135; the mflo temps 134/176/192/158 sit in hard reg 65 = LO, not a GPR; pseudo 173 gets no hard register at all.
- verdict: KILLED

## [s3] The FIRST divergence in this function's register allocation is allocno ord=0 (pseudo 129 = div16 = *(s16 *)(arg0 + 0x1A)), and $v1 is denied to it by a single regs_someone_prefers bit - a PREFERENCE bit, not a conflict bit.
- mechanism: global.c find_reg pass 0 excludes hard_reg_conflicts | ~regs_used_so_far | regs_someone_prefers and returns the lowest survivor; regs_someone_prefers is built in prune_preferences (global.c:882-931) as the union of hard_reg_full_preferences over lower-priority CONFLICTING allocnos, and those bits are written by set_preference (global.c:1671) off the RTL copy structure.
- probe: BB2_FINDREG_DEBUG=129 with the candidate body applied (tmp/grind/func_80023648/s3/findreg_129.log), cross-read against the ALLOCDBG order table and s2's value->seat map row 'div16 ours $a0 / target $v1'.
- result: CONFIRMED. 129's pass-0 state: conflicts={2,29}, someone_prefers={3}, own_copy_prefs={}, own_full_prefs={}, pass0_used={0,1,2,3,16..23,26..31}. $v1(3) is NOT a conflict - it is excluded only by regs_someone_prefers. Clear that one bit and pass 0 returns 3 = $v1 = target's div16 seat, with nothing else changed. Every other allocno is allocated after 129, so 129 is the true root and pseudo 86 (ord=8) is cascade.
- verdict: CONFIRMED

## [s3] The table-entry pseudo 86 can be driven to $a2 by making more values live across its defining insn (s2's lever A).
- mechanism: find_reg pass 0 is a plain ascending lowest-free-survivor scan, so additional hard-reg conflicts push the chosen seat upward through the register file.
- probe: Variant andcond - the two guards rewritten non-short-circuit as (D_800A38BA != 0) & (*(s16 *)(arg0 + 6) == 0), forcing both guard values live across the table-entry load - measured with sandbox and disassembled.
- result: KILLED. Score 47 at an unchanged 159/159. The asm shows both guard temps live (sltu $v1,$zero,$v1 / sltiu $v0,$v0,1 / and $v1,$v1,$v0) before lh $v0,0($a0) - and the table entry took $v0, not $a2. Landing $a2 (reg 6) by conflicts alone needs 2,3,4,5 ALL excluded from 86; 4 and 5 already are, so it is a two-more-bit ask a fixed 159-insn multiset cannot supply. Extra live values only walk the entry down the ascending list one seat at a time.
- verdict: KILLED

## [s3] regs_someone_prefers[86] can be reached through the local-alloc seats of allocnos 107 and 110 (s2's H4).
- mechanism: set_preference writes hard_reg_full_preferences when one side of a SET is a hard reg or an already-locally-renumbered pseudo; prune_preferences then unions those into the higher-priority conflicting allocno's regs_someone_prefers.
- probe: BB2_FINDREG_DEBUG=107 and =110 (tmp/grind/func_80023648/s3/findreg_107.log, findreg_110.log) plus identification of both from the .lreg RTL.
- result: KILLED. 107 = *(s16 *)(arg0 + 0x1CA) (insn 102); 110 = the a2/4 rounding temp (110 = 86, then 110 = 86 + 3 under bgez, then 109 = 110 >> 2). BOTH already carry hard reg 3 in hard_reg_conflicts - a consequence of 86 taking $v1 earlier - so neither can ever seat $v1 or hand it to anyone. 110's own_full_prefs is {5} and 107's {5} was pruned away by its own $a1 conflict, leaving it empty. Neither carries a $v1 bit and neither can acquire one: preference bits only form against a hard reg or a local-alloc'd seat, and no $v1-seated local quantity is paired with 107 or 110 in this data flow.
- verdict: KILLED

## [s3] Reordering allocno priority (via nrefs / live-length edits) can make find_reg pass 0 skip $v1 under the 'never allocate a register for the first time in pass 0' rule.
- mechanism: Pass 0 ORs ~regs_used_so_far into its exclusion set, so a hard reg not yet in regs_used_so_far is skipped in pass 0.
- probe: Read of global.c:340-372 (the regs_used_so_far seeding) plus the measured used_so_far line in every FINDREGDBG record taken this session.
- result: KILLED. regs_used_so_far is seeded with regs_ever_live[i] || call_used_regs[i] for every hard reg BEFORE any allocation. On MIPS $v0-$a3 and $t0-$t9 are all call-used, so 2,3,4,5,6,7 are in the set from the start and stay there - confirmed in every measured record. No priority reordering can make $v1 'not yet used'. This closes the entire allocation-order-reshuffle axis as a route to freeing $v1.
- verdict: KILLED

## [s3] C-level operand order is a live lever on GCC 2.7.2's register preferences (and therefore on find_reg pass-0 seats) in this function.
- mechanism: set_preference (global.c:1671) unwraps an 'e'-format SET_SRC exactly once (src = XEXP (src, 0), copy = 0), so for (set (reg 5 a1) (plus (reg X) (reg Y))) only X - the FIRST plus operand - receives the $a1 preference bit. Swapping the C operand order swaps which pseudo is X.
- probe: Variants swapadd (a2 / 4 + *(s16 *)(arg0 + 0x1CA)), swapelse (a2 + *(s16 *)(arg0 + 0x1D8)) and swapboth, each applied to src/code6cac.c and measured with sandbox; swapadd additionally disassembled and diffed against target.
- result: CONFIRMED as a lever, rejected as a fix. swapadd = 33 and swapboth = 33, both at an unchanged 159/159 - the allocation genuinely moved (the lh $v0, 0x1CA($s0) load relocated from before the bgez to after it, and seats changed), though the table entry stayed in $v1. swapelse is byte-neutral at 30. The load-bearing consequence is negative-result-proof: s2's 'spelling cannot reach the allocator here' holds only for hard_reg_conflicts bits and must NOT be generalised to preference bits.
- verdict: CONFIRMED

## [s4] The permuter, run on a chassis-faithful standalone workspace, can find C respellings that lower the honest floor below 30 — i.e. s2/s3's "spelling cannot reach the allocator here" is false as stated.
- mechanism: decomp-permuter randomizes the C AST (variable reuse, pointer intermediates, statement fusion, operand reassociation) far outside the neighbourhood a human enumerates by hand. s3 had already CONFIRMED that C-level operand order moves GCC 2.7.2 register PREFERENCES (set_preference / prune_preferences); the permuter simply searches that same preference-reachable space exhaustively instead of one spelling at a time.
- probe: three campaigns via tools/permuter_campaign.py (vanilla / varA-reseed / varL-reseed) on tmp/perm_23648_s4{,b,c}. The workspace was validated BEFORE launch: base.o vs target.o = 159/159 insns with exactly 30 mismatching lines, identical to the sandbox residual, so permuter score movement is real chassis movement. Every proposed delta was then re-spelled by hand into src/code6cac.c and measured with `sandbox func_80023648 --disable all`.
- result: CONFIRMED, decisively. Floor 30 -> 22 -> 15, all at an unchanged 159/159. Two independent levers: (varA) reusing the existing `div16` local to carry `(s16)new_14e` for the `limit <` comparison, 30 -> 22; (varL) naming the table-element address, `ent = &row[a1]; a2 = *ent;`, 22 -> 15. Campaign 1: 14 finds / ~12 min / best permuter score 120 from base 180. Campaign 2: 10 finds / ~21 min / best 80 from base 120. Campaign 3 (reseeded from the 15-floor body): 17032 iterations, ZERO finds — that basin is dry.
- verdict: CONFIRMED

## [s4] The other permuter-proposed deltas are additive with varA/varL.
- mechanism: if each delta perturbs a different preference edge, stacking them should compose.
- probe: seven stacked variants, each applied to src and measured at 159/159 — varB (pointer local for the tail 0x14E store), varD (`abs_val` reused for the 0xD8 accumulate temp), varE (`tbl_val = speed`), varF (D+E), varG (`a1 = a2` in the else arm), varH (fused `*(s16 *)(arg0+0x14E) = (new_14e = sub_result)`), varK (`argp = (s16 *)arg0` for the two func_8001F860 calls), varKL (K stacked on L).
- result: KILLED. Nothing composes. On top of varA: varB 30 (neutral AND a dead construct — dropped), varD 23, varE 24, varF 24, varG 22 (neutral), varH 22 (neutral), varK 17. varK at 17 is better than varA's 22 but worse than varL's 15, and varKL — the two pointer intermediates stacked — regresses to 24, i.e. `argp` and `ent` actively fight each other for the same seat rather than adding. The levers are competing spellings of one allocation decision, not independent knobs.
- verdict: KILLED

## [s4] STATUS OF H9/H10/H11 (the s3 frontier) — UNPROBED, AND THEIR PREMISE IS NOW STALE.
- H9/H10/H11 were not touched this session (permuter modality). They remain formally open, but every allocno id and seat they name (129 = div16, 122 = abs_val, 86 = the table entry, ord list `129 122 85 81 72 134 ...`) was measured against the 30-floor s3 body. This session's body differs in two places that both sit exactly on the abs/clamp/div16 region H9 and H10 target, so those ids and the conflict/preference sets attached to them MUST be re-measured before any H9/H10 probe is spent. More importantly, H9's whole framing assumed the residual was one preference bit away; the floor has since moved 30 -> 15 without touching that bit, so the seat map itself needs redrawing.
- next probe (for whichever session picks the RA axis back up): re-run BB2_ALLOC_DEBUG=1 and BB2_FINDREG_DEBUG against the CURRENT candidate body, produce a fresh 159-line aligned seat diff, and only then re-state H9/H10 in terms of the new allocno ids.

## [s4] The permuter, run on a chassis-faithful standalone workspace, can find C respellings that lower the honest floor below 30 - i.e. s2/s3's 'spelling cannot reach the allocator here' is false as stated.
- mechanism: decomp-permuter randomizes the C AST (variable reuse, pointer intermediates, statement fusion, operand reassociation) far outside the neighbourhood a human enumerates by hand. s3 had already CONFIRMED that C-level operand order moves GCC 2.7.2 register PREFERENCES via set_preference / prune_preferences; the permuter searches that same preference-reachable space exhaustively instead of one spelling at a time.
- probe: Three campaigns via tools/permuter_campaign.py on tmp/perm_23648_s4{,b,c}. Workspace validated BEFORE launch: base.o vs target.o = 159/159 insns with exactly 30 mismatching lines, identical to the sandbox residual. Every proposed delta was re-spelled by hand into src/code6cac.c and measured with sandbox --disable all.
- result: Floor 30 -> 22 -> 15, all at an unchanged 159/159. Lever 1 (varA): reuse the existing div16 local to carry (s16)new_14e for the 'limit <' comparison, 30 -> 22. Lever 2 (varL): name the table-element address, ent = &row[a1]; a2 = *ent;, stacked on varA, 22 -> 15. Campaign telemetry: vanilla base_score 180 / 14 finds in ~12 min / best 120; varA reseed base_score 120 / 10 finds in ~21 min / best 80; varL reseed base_score 95 / 17032 iterations / ZERO finds (basin dry, harvested + stopped).
- verdict: CONFIRMED

## [s4] The other permuter-proposed deltas are additive with varA/varL and can be stacked toward zero.
- mechanism: If each delta perturbs a different preference edge, stacking them should compose.
- probe: Seven stacked variants applied to src and measured at 159/159: varB (pointer local for the tail 0x14E store), varD (abs_val reused for the 0xD8 accumulate temp), varE (tbl_val = speed), varF (D+E), varG (a1 = a2 in the else arm), varH (fused *(s16 *)(arg0+0x14E) = (new_14e = sub_result)), varK (argp = (s16 *)arg0 for the two func_8001F860 calls), varKL (K stacked on L).
- result: Nothing composes. On top of varA: varB 30 (neutral and a dead construct - dropped), varD 23, varE 24, varF 24, varG 22 (neutral), varH 22 (neutral), varK 17. varK (17) beats varA but loses to varL (15), and varKL - the two pointer intermediates stacked - REGRESSES to 24: argp and ent contend for the same seat rather than adding. These are competing spellings of one allocation decision, not independent knobs.
- verdict: KILLED

## [s4] A plain permuter reseed off the new 15-floor body continues to pay.
- mechanism: Fresh-seed discipline: a basin yields early or not at all.
- probe: Campaign 3 (tmp/perm_23648_s4c) launched from the exact 15-floor body, base_score 95, run to 17032 iterations across ~11 min of blocking waits.
- result: ZERO finds - not one output below the base score. The basin off this body is dry; the next permuter session must reseed a STRUCTURALLY DIFFERENT chassis rather than re-run from this one. Harvested with --stop; no campaign outlived the session.
- verdict: KILLED
