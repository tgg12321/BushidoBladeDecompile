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
