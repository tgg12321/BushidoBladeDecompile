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
