# Hypothesis ledger — func_80040D48

## [s1] H1 — prologue: two-var load (`tmp = D_800A9A10[a0]; if (!tmp) return; s4 = tmp;`) produces target's lw-$a0/beqz/copy-in-delay shape and the pin is unnecessary
- mechanism: tmp's pseudo dies before any call → caller-save ($a0 free after param a0's copy to s7); the s4 copy fills the beqz delay slot; param a0 naturally allocates to s7 (last of the call-crossing set in greg order)
- probe: edit + sandbox
- result: 34 → 31; prologue cluster (5 diffs) + pin gone
- verdict: CONFIRMED

## [s1] H2 — the s5/s6 seat swap exists because target's $s5 held ONE variable serving both the s4+0x2C role and the Copy8-loop source pointer
- mechanism: global.c allocates priority-descending; merged pseudo gets loop-weighted refs (flow.c `reg_n_refs += loop_depth`) → outranks arg5 (log2(4)*4/362≈.022) which otherwise wins s5 by .022 vs .018; measured greg order had 77(arg5) immediately before 79(ptr)
- probe: reuse `s5` as the copy-loop pointer (goto-loop spelling)
- result: 31 → 24; s5/s6 seats, sh 6($s5), all four tail-call moves, and the mflo $8 temp all now match
- verdict: CONFIRMED

## [s1] H3 — for(;;)+break with the reused outer s5 keeps the loop shape
- mechanism: (assumed loop-note neutrality)
- probe: edit + sandbox
- result: 273 insns (rotation + first-iteration load folded to s4-relative); goto-label spelling avoids it (no loop notes → loop.c can't rotate)
- verdict: KILLED as stated; goto-label spelling CONFIRMED (see rejected/for-loop-s5-reuse-rotates-copy-loop.c)

## [s1] H4 — bumping a pointer's reg_n_refs by anchoring a sibling def on it (`s3 = s5 + 0x68`, `a3p = a2p + 0x18`)
- mechanism: hoped-for combine-foldable ref surviving to flow
- probe: both spellings, sandbox each
- result: both NEUTRAL — cse1 (pre-flow) folds same-EBB derived-base sums back to the s4-relative form; the ref never reaches reg_n_refs
- verdict: KILLED for same-EBB anchors (rejected/derived-base-anchor-ref-cse-folded.c). Any ref-bump must survive cse (cross-block) or use another mechanism.

## Frontier (for s2)

1. **Class B (a2p/a3p seats, ~14 diffs):** read .lreg qty ordering +
   local-alloc.c `qty_compare` for the two qtys; then sweep honest orderings:
   decl order, def order, first-use order inside the loop (e.g. move the
   `*list3 = a2p` store or the a2p increment earlier), and whether writing the
   Copy8 copy via a2p-relative dest changes the qty birth order. Expect Class
   C to cascade — re-measure before working it separately.
2. **Class A (a4p IV bias, 4 diffs):** regenerate dumps against the CURRENT
   body, read .loop for the giv combination that re-biases a4p by +0x14;
   candidate levers: strength-reduction-visible spelling changes in the
   `do {} while (s0 < 0x12)` init loop (store order is fixed by bytes; try
   the increment position, counter direction, or a4p as the loop condition
   carrier), cross-check how sibling matched loops in this TU (func_80040B44
   init loops) avoided the bias.
3. **Class C (a2p2/-1 seats, 6 diffs):** only if it survives Class B's fix.

## Judge-facing notes
- The candidate's `tmp` (prologue) is written once, read twice (test + copy).
  If layer-1 treats it as named-intermediate-family it fails prong 1
  (once-read); before submission either verify it reads as ordinary C (the
  null-check idiom) or measure the assignment-in-condition respelling.
- The s5 reuse is variable-reuse family (staged-value-reused-variable /
  defeat-licm-hoist-var-reuse lineage): it borrows an EXISTING local for the
  loop pointer. Before candidate-ready, read
  `.claude/rules/staged-value-reused-variable.md` +
  `.claude/rules/defeat-licm-hoist-var-reuse.md` end-to-end and write the vet
  with the scope sentence quoted; the evidence here (target's own $s5 serving
  both roles) supports it as the ORIGINAL structure, which is the strongest
  possible posture — argue object-model reconstruction, not codegen lever.

## [s1] Prologue two-var load (tmp = D_800A9A10[a0]; if (!tmp) return; s4 = tmp;) reproduces target's lw-$a0/beqz/copy-in-delay shape without the asm("s7") pin
- mechanism: tmp dies pre-call -> caller-save $a0; copy fills beqz delay slot; param a0 allocates to s7 naturally (last call-crossing pseudo in greg order)
- probe: edit + sandbox --disable all
- result: 34 -> 31, prologue cluster closed, pin deleted
- verdict: CONFIRMED

## [s1] The s5/s6 seat swap (3 rule ranges) exists because target's $s5 held ONE variable: the s4+0x2C pointer AND the Copy8-loop source pointer
- mechanism: global.c priority allocation; flow.c reg_n_refs += loop_depth makes the merged pseudo outrank arg5 (measured .022 vs .018, adjacent in greg order)
- probe: reuse existing s5 local as the copy-loop pointer, goto-loop spelling; sandbox
- result: 31 -> 24; s5/s6 seats, mflo $8 temp, tail-call moves all match
- verdict: CONFIRMED

## [s1] for(;;)+break with reused outer s5 keeps the copy-loop shape
- mechanism: assumed loop-note neutrality
- probe: edit + sandbox
- result: 273 insns: loop.c rotates + cse folds first-iteration load; goto-label spelling (no loop notes) restores 272 and scores 24
- verdict: KILLED

## [s1] A pointer pseudo's reg_n_refs can be bumped by anchoring a sibling def on it (s3 = s5+0x68; a3p = a2p+0x18)
- mechanism: hoped combine-foldable extender ref surviving to flow
- probe: both spellings, sandbox each
- result: both neutral: cse1 (pre-flow) folds same-EBB derived-base sums; ref never counted
- verdict: KILLED

## [s2] H5 — the Copy8 loop's list-push pointer (a2p, s4+0x10D4) and the tail walker (a2p2, s4+0x8B4) are ONE variable in the original; splitting them is what inverts the $a2/$a3 seats
- mechanism: global.c `allocno_compare` = floor_log2(n_refs)*n_refs/live_length, descending. Measured from .flow: a2p refs=4 len=13 (pri .615) vs a3p refs=5 len=12 (pri .833), so a3p sorts first and `find_reg` (no REG_ALLOC_ORDER on MIPS ⇒ ascending scan) gives it $6. Merging a2p with the tail walker takes the merged pseudo to ~9 refs over ~25 insns (pri ~1.08), sorting it ahead of a3p.
- probe: reuse the same local for both walkers (`a2p = s4 + 0x8B4;` after `copydone:`), sandbox --disable all
- result: **24 → 4** (272/272). Class B (14 diffs) and Class C (6 diffs) both closed by the single edit; the `-1` holder seat cascaded to $a0 as target has it.
- verdict: CONFIRMED

## [s2] H6 — Class A's a4p +0x14 bias is loop.c strength reduction, and target's loop was never reduced
- mechanism: `.loop` shows three dest-address givs off biv 96 (adds 16/18/20, benefit 2 each). `combine_givs` walks `bl->giv` head-first and `record_giv` prepends, so the base is the LAST-emitted address (+0x14); combining credits it with benefit 6, and `v->lifetime*threshold*benefit < insn_count` (3*threshold*4 vs 28) can never fire, so it is always reduced to `s3 + 0x68 + 0x14 = s3+124` with stores at -4/-2/0. Target's base register is `s3+0x68` (the biv's init) with the last store at +0x14 — impossible for ANY combination base, so target's loop was not reduced at all (benefit ≤ 0 there).
- probe: read .loop dump for the current body + loop.c:3775-3830 / combine_givs / combine_givs_p / mips_address_cost
- result: attribution nailed; two whole families of respellings eliminated on paper — (i) re-basing a4p so a store lands at offset 0 (loop.c does not record a mult-1/add-0 address as a giv, so the remaining two still combine AND the biv then survives as a second register, costing insns), (ii) permuting store order (only moves the base to another nonzero offset).
- verdict: CONFIRMED (mechanism); the two respelling families KILLED without needing a measurement

## Frontier (for s3)

1. **Class A, the only residual (4 diffs).** Target needs a4p to NOT be a
   basic induction variable, so that its three `+0x10/+0x12/+0x14` stores stay
   biv-relative and `addiu $a0,$a0,0x68` is emitted as an ordinary add. In
   loop.c any SECOND set of that pseudo inside the loop body forces
   `reg_iv_type = NOT_BASIC_INDUCT`. Probe order:
   (a) re-derive the case-0 init loop from scratch (m2c + target asm + the
       matched sibling loops in this TU and func_80040B44) — the honest shape
       may simply differ from the s1 spelling (e.g. a struct-typed walker, or
       the counter driving the address);
   (b) check whether writing the three stores through a nested member group /
       a differently-typed pointer changes what `find_mem_givs` records;
   (c) only if (a)+(b) are exhausted, evaluate whether a second in-loop set of
       a4p that flow.c deletes before final is a dead-store-family construct —
       that is `.claude/rules/dead-store-fake-exception.md` territory, needs
       the ladder documented and a FAKE annotation, so it is a LAST resort and
       probably a ruling-request, not a self-approval.
   Do NOT re-measure: re-basing a4p to s3+0x78 (offsets 0/2/4) or s3+0x7C
   (offsets -4/-2/0), or permuting the three stores — all proven dead in H6.
2. Nothing else diverges. Every other insn in the function is byte-identical.

## Judge-facing notes (updated s2)
- Constructs currently in the body needing a self-vet when floor hits 0:
  (i) the prologue `tmp` two-var load (s1); (ii) the `s5` reuse across the
  s4+0x2C role and the Copy8 source pointer (s1); (iii) the `a2p` reuse across
  the Copy8 list-push walker and the s4+0x8B4 tail walker (s2); (iv) the
  `copyloop:`/`copydone:` goto spelling of the Copy8 loop (s1).
  (ii) and (iii) are variable-reuse family and BOTH are evidence-backed as the
  original's own object model (target's $s5 and $a2 each serve exactly the two
  roles the merged C variable serves) — argue object-model reconstruction and
  cite `.claude/rules/staged-value-reused-variable.md` +
  `.claude/rules/defeat-licm-hoist-var-reuse.md` after reading them end-to-end.

## [s2] The Copy8 loop's list-push pointer (a2p, s4+0x10D4) and the later s4+0x8B4 tail walker (a2p2) are ONE variable in the original; declaring them as two separate locals is what inverts the $a2/$a3 seats in the copy loop and the $a2/$a0 seats in the tail walker.
- mechanism: global.c allocno_compare = floor_log2(n_refs)*n_refs/live_length, sorted descending; MIPS defines no REG_ALLOC_ORDER so find_reg scans ascending and the first-allocated of the pair takes $6. Measured from the .flow dump: pseudo 186 (a2p) refs=4 len=13 -> pri 0.615; pseudo 187 (a3p) refs=5 len=12 -> pri 0.833, so a3p sorts first (confirmed verbatim in the .greg 'regs to allocate' order line: 187 is 9th, 186 is 13th). Merging a2p with the tail walker takes the merged pseudo to ~9 refs over a ~25-insn live range (pri ~1.08), sorting it ahead of a3p.
- probe: reuse the same local for both walkers (a2p = s4 + 0x8B4; after copydone:) and re-measure sandbox --disable all
- result: 24 -> 4 (272/272 insns). All 14 Class B diffs and all 6 Class C diffs closed by the single edit; the -1 comparison holder cascaded to $a0 exactly as target has it.
- verdict: CONFIRMED

## [s2] Class A (the case-0 init loop's a4p induction variable biased +0x14) is loop.c strength reduction, and target's loop was never reduced at all -- so no re-basing or store-reordering respelling can reach it.
- mechanism: loop.c: the case-0 init loop has three dest-address givs off biv 96 (adds 16/18/20, benefit 2 each). combine_givs walks bl->giv head-first and record_giv PREPENDS, so the combination base is always the LAST-emitted address (+0x14); combining credits the base with the absorbed benefits (benefit 6). The reduction test v->lifetime*threshold*benefit < insn_count (loop.c:3823) with benefit -= add_cost*bl->biv_count (add_cost measured = 2 from the sibling single-giv loops' 'not worth while, 0 vs 12' lines) and threshold = 2*(3+n_non_fixed_regs) gives 3*threshold*4 vs insn_count 28 -- it can never fire, so the loop is always reduced to s3+0x68+0x14 = s3+124 with stores at -4/-2/0.
- probe: pwsh tools/grinder/dump.ps1 func_80040D48 against the current body; read the .loop giv records for the case-0 loop plus loop.c:3775-3830, combine_givs, combine_givs_p, and mips_address_cost
- result: Attribution nailed. Target's base register holds exactly s3+0x68 (the biv's initial value) with its LAST store at +0x14 -- impossible for any combination base, since the base giv's own store is by construction at offset 0 -- so target's three givs were never combined (benefit <= 0 there). This kills two whole respelling families without needing measurements: (i) re-basing a4p so one store lands at offset 0 (loop.c does not record a mult-1/add-0 address as a giv at all, so the remaining two still combine AND the biv then survives as a second live register, costing insns) and (ii) permuting the three stores (only moves the base to another nonzero offset).
- verdict: CONFIRMED

## [s2] Any ref-count edit inside the Copy8 loop itself could flip the a2p/a3p seats.
- mechanism: raising a2p's reg_n_refs or lowering a3p's within the loop to invert allocno_compare
- probe: enumerate a3p's irreducible refs from the .lreg RTL (def, lw 0x40($a3), the BLKmode struct copy which is ONE insn at flow time, and the increment's set+use) and a2p's (def, list store, increment set+use), then compare against target's own instruction stream
- result: KILLED. a3p's 5 refs and a2p's 4 are both irreducible, and target's a2p likewise has exactly 4 refs in its own bytes, so the flip provably cannot come from inside the loop -- it had to come from outside it (which is what the merge did).
- verdict: KILLED
