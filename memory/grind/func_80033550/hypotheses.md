# Hypothesis ledger — func_80033550

## s1 (recon)

- H1 "the old form's 4 needed the pin" — **KILLED**: pin-free do-while form
  scores 4; the pin was inert (sandbox had been stripping it anyway).
- H2 "idx spelling (i*3 + s32-array indexing) moves the <<2 or changes RA" —
  **KILLED**: byte-identical output; combine canonicalizes ×12.
- H3 "`register` storage class on the alias biases the pointer home" —
  **KILLED** (no effect, still a1; also stripper-flagged spelling).
- H4 "sched hoist of final sll is a separate honest gap" — **CONFIRMED
  closed**: single-level FAKE-annotated do-while(0) (sanctioned 2026-07-06)
  fences it; floor 5→4.
- H5 (mechanism, CONFIRMED by .greg): a1-vs-a3 is a global.c first-free
  outcome; ptr conflict set = {v0,v1,a0} and w2 shares a1 via death/birth
  adjacency. Flipping to a3 requires making BOTH a1 and a2 conflict with the
  pointer without emitting bytes.

## Live frontier (next sessions)

1. **Permuter campaign** from the floor-4 candidate (directed + random),
   targeting the a1→a3 pointer-home flip. Prior park's un-run avenue; the
   candidate base is better than any prior seed (schedule already matches).
2. **Instrumented cc1 forensics** (BB2_ALLOC_DEBUG / -da on variants): find
   any input shape where local-alloc seats an overlapping pseudo in a1/a2
   (e.g. forms where w2's birth precedes ptr's last use), or a preference
   path to a3. Key question: what deleted-post-RA construct (no-op move
   coalescence) can occupy a1+a2 during ptr's range?
3. **Sanctioned-FAKE family sweep** (last resort, byte-neutrality required):
   staged-value-reused-variable / duplicated-statement-into-arms /
   named-local-fake forms that extend an a1/a2 occupant across the pointer's
   range. Note measured trap: extending i's life across the loads steals v1
   from w0 (target needs w0=v1) — any lever must not disturb w-value homes.

## [s1] The inherited form's floor-4 required the asm("a3") register pin
- mechanism: sandbox strips pins before scoring, so the pin could never have moved the score
- probe: removed the pin, kept the do-while wrap, sandbox --disable all
- result: score 4, cheat_asm_stripped back to file baseline (369) — pin was inert
- verdict: KILLED

## [s1] Respelling idx as i*3 with s32-array indexing changes chain placement or RA
- mechanism: move the <<2 into store-address arithmetic so combine/sched treat the chain differently
- probe: (&D_80107850)[i*3] form, sandbox + objdump diff
- result: byte-identical output to the i*12 form (combine canonicalizes); score 5 without wrap
- verdict: KILLED

## [s1] `register` storage class on a pointer alias biases the pointer home toward a3
- mechanism: REG_USERVAR/register-decl preference in local/global alloc
- probe: register s32 *p = arg0; loads via p; sandbox
- result: still 4 (a1), and the engine stripper counts the register-alias as strippable (369->370) — spelling is also reviewer-hostile
- verdict: KILLED

## [s1] The sched1 hoist of the final sll above the three lw's is honestly closeable
- mechanism: single-level do-while(0) loop notes fence the tail schedule — sanctioned for any codegen effect per the 2026-07-06 owner rewrite of do-while-zero-exception (FAKE-annotated)
- probe: pin-free wrap around {idx = new_var; w0 = arg0[0];}, sandbox
- result: score 4, schedule matches target exactly; residual is purely the 4 pointer-home insns
- verdict: CONFIRMED

## [s2] Pointer-alias second handles create a1/a2 conflicts byte-free
- mechanism: overlapping ptr pseudos from `s32 *p = arg0;` forms
- probe: 7 alias variants (w2-only, w12, all, block, late-init, two-handle, first-two)
- result: every variant score 4 with ptr=a1 (handle ties to a0 no-op, or becomes THE copy); overlapping un-tied pair emits a 2nd move (13)
- verdict: KILLED

## [s2] Staged value through dead new_var injects an a1 conflict and flips ptr
- mechanism: staged load's pseudo + w-home shift put a1 in 72's conflict set
- probe: v07 + 5 placement variants; .greg dumps
- result: CONFIRMED for the flip (a1→a2, conflicts {v0,a0,a1}, first ever movement) but inherently rotates w-homes (staged pseudo takes v1) — best staged score 5, none preserves homes
- verdict: CONFIRMED (mechanism) / KILLED (as a closing lever on its own)

## [s2] Load-order / wrap-content geometry flips ptr while keeping homes
- mechanism: reorder which load is ptr's last use
- probe: w2-in-wrap, w2-before-w1, w0+w1-in-wrap, idx-only wrap, w2-before-wrap, early-stage
- result: all either 4 with ptr=a1 (sched restores order; last-use unchanged at RA) or 5 (ptr-copy deleted, loads via a0)
- verdict: KILLED

## [s2] Promoting w's to global pseudos (per-load wraps) changes the RA fill
- probe: v21 triple wrap
- result: byte-identical to base
- verdict: KILLED

## [s2] Uncoalesced i-copy (found_idx) or constant-holder (one=1) adds pressure
- probe: v22, v23
- result: both byte-identical (copy ties+deleted; holder folds)
- verdict: KILLED

## [s2] Original signature had extra params occupying a1/a2
- probe: caller asm inspection (only caller: DispPracticeMenuTex_C)
- result: caller sets only a0; unused params are flow-deleted pre-RA anyway
- verdict: KILLED

## [s2] STRUCTURAL CONCLUSION (mechanism, CONFIRMED): numeric alloc order
(no REG_ALLOC_ORDER) + 5-value census ⇒ a3 requires TWO zero-byte a1/a2
occupants conflicting with ptr; the w's are the only candidates and
re-homing them costs ≥2 diffs. The original's RA input had conflict
sources invisible in the bytes — not reachable by tail geometry.

## [s1] a1-vs-a3 is a global.c first-free allocation outcome, not scheduling
- mechanism: ptr pseudo 72 conflicts {v0,v1,a0} only; w2 (local, a1) shares a1 via death/birth adjacency at its own lw; first free = a1. Target's a3 requires BOTH a1 and a2 to conflict with ptr
- probe: cc1 -da full dump; read .greg conflict lists + dispositions
- result: confirmed from probe.i.greg: '72 conflicts: 72 73 2 3 4 29', dispositions 72->a1, 76(w2)->a1 shared; no visible a2 occupant exists in target either — original RA input had byte-invisible pressure
- verdict: CONFIRMED

## [s2] Pointer-alias second handles to arg0 create a1/a2 conflicts byte-free
- mechanism: overlapping ptr pseudos from s32 *p = arg0 forms occupy arg regs during ptr's range
- probe: 7 alias variants (w2-only, w12, all, block-local, late-init, two-handle, first-two), sandbox + normalized diff each
- result: every variant scores 4 with ptr=a1 — the handle either ties to incoming a0 (copy no-op-deleted) or becomes THE single a1 copy; two overlapping un-tied handles emit a second move (score 13)
- verdict: KILLED

## [s2] Staging w2 through dead new_var injects an a1 conflict that flips ptr's home
- mechanism: staged load's global pseudo + shifted w-homes put hard-reg 5 in pseudo 72's conflict set; first-free skips to a2
- probe: v07 + 5 placement variants, sandbox + .greg dumps (72 conflicts: 74 2 4 5 29)
- result: CONFIRMED as mechanism: ptr moved a1->a2, the only movement across all sessions. KILLED as a closing lever alone: staged pseudo grabs v1, local-alloc rotates w-homes to a0/a1/v1 (target v1/a0/a1), best staged score 5, none preserves homes
- verdict: CONFIRMED

## [s2] Load-order / wrap-content geometry can flip ptr while keeping w-homes
- mechanism: reorder which load is ptr's last use so earlier-born w's conflict
- probe: w2-in-wrap, w2-before-w1, w0+w1-in-wrap, idx-only-wrap, w2-before-wrap, early-stage variants
- result: all 4 with ptr=a1 (sched restores RA-time order) or 5 (ptr copy vanishes, loads via a0)
- verdict: KILLED

## [s2] Promoting w's to block-crossing pseudos (per-load do-while wraps) changes the RA fill
- mechanism: global.c priority allocation instead of local-alloc first-free
- probe: v21 triple wrap, sandbox + diff
- result: byte-identical to base
- verdict: KILLED

## [s2] An uncoalesced i-copy (found_idx) or a constant-holder (one=1) adds RA pressure
- mechanism: extra pseudo range overlapping ptr
- probe: v22 found_idx split, v23 one-holder, sandbox + diff
- result: both byte-identical (copy ties to v1 and is no-op-deleted; holder folds to the same li)
- verdict: KILLED

## [s2] The original signature had extra params whose incoming a1/a2 pressured RA
- mechanism: live-in arg regs conflict with ptr at entry
- probe: inspected the single caller's asm (DispPracticeMenuTex_C @19BA4)
- result: caller sets only a0 in the jal delay slot; a1 holds a stale callee return; unused param copies are flow-deleted pre-RA anyway
- verdict: KILLED

## [s3] A loop-region respelling (same loop bytes, different pseudo census) supplies the invisible a1/a2 occupants
- mechanism: any pseudo overlapping ptr's whole-function range counts; loop spellings might create extra pseudos that fold to identical bytes
- probe: 6 spellings (for, do-while, while, goto+flag-temp s32/u8, per-iteration address temp), sandbox + per-variant .greg
- result: ALL score 4 with byte-for-byte identical .greg (2 allocnos, 72 conflicts {i,v0,v1,a0}) — cse/flow canonicalize every spelling to the same pre-RA RTL
- verdict: KILLED

## [s3] A live flags/walker pointer in the loop injects an a1/a2 conflict cheaply
- mechanism: base-pointer pseudo overlaps ptr and takes an arg reg
- probe: flags[] loop+store, loop-only, walker+index hybrid, store-via-addr-temp
- result: pseudo 75 does conflict with ptr, but scores 19/10/9/11 — target's at-macro address forms make any materialized pointer diverge
- verdict: KILLED

## [s3] Const-address (REG_EQUIV-class) pointer pseudos survive to conflict construction and shape the scan byte-free
- mechanism: frontier-2 conjecture — conflict lists built before final deletions could include folded pseudos
- probe: t0/t1/t2 = &D_8010785x table pointers, plain (Z2) and inside a do-while(0) fake loop (Z4); .greg allocno census
- result: both byte-identical (4) with only 2 allocnos — the sets are cse-folded + flow-deleted PRE-RA; global.c never sees them; loop notes don't preserve them
- verdict: KILLED (as a C-spelling lever; the pre-RA-deletion argument is now empirical)

## [s3] A DImode temp pair-homes into a1+a2 as a single zero-byte double occupant
- mechanism: multi-reg pseudo needs 2 consecutive hard regs; conflicts {v0,v1,a0} force exactly a1+a2, pushing ptr to a3
- probe: s64 t=i (Z1) and u64 t=(u32)i (Z3, zero high half = cheapest conceivable), sandbox + .greg
- result: mechanism CONFIRMED by .greg (DImode pseudo conflicts exactly {v0,v1,a0}+ptr) but both score 22 — GCC 2.7.2 always emits both half-sets; no byte-free DImode spelling exists
- verdict: KILLED (spelling) / CONFIRMED (mechanism — a pair occupant is the single-construct answer if one could ever be byte-free)

## [s3] STRUCTURAL MODALITY EXHAUSTED (s2 tail geometry + s3 loop census +
REG_EQUIV + DImode). Remaining live axes are permuter whole-function search
and cc1 forensics: post-conflict-build deletions (no-op-move coalescence /
reg_may_share, reload inheritance) + a cc1psx calibration cross-check
(does the SN cc1psx home ptr in a3 for this same C?).

## [s3] A loop-region respelling (same loop bytes, different pseudo census) supplies the invisible a1/a2 occupants
- mechanism: any pseudo overlapping ptr's whole-function range counts; loop spellings might create extra pseudos that fold to identical bytes
- probe: 6 spellings (for, do-while, while, goto+flag-temp s32/u8, per-iteration address temp), sandbox + per-variant .greg extraction
- result: ALL score 4 with byte-for-byte identical .greg (2 allocnos, 72 conflicts {i,v0,v1,a0}); cse/flow canonicalize every spelling to the same pre-RA RTL
- verdict: KILLED

## [s3] A live flags/walker pointer in the loop injects an a1/a2 conflict cheaply
- mechanism: base-pointer pseudo overlaps ptr's range and takes an arg reg
- probe: flags[] loop+store (L6), loop-only (L7), walker+index hybrid (L8), store-via-addr-temp (L10)
- result: pseudo 75 does conflict with ptr but scores 19/10/9/11 — target's at-macro address forms make any materialized pointer diverge
- verdict: KILLED

## [s3] Const-address (REG_EQUIV-class) pointer pseudos survive to conflict construction and shape the first-free scan byte-free
- mechanism: conflict lists are built before final deletions, so folded pseudos could still have shaped the scan
- probe: t0/t1/t2 = &D_8010785x table pointers, plain (Z2) and inside a do-while(0) fake loop (Z4); .greg allocno census
- result: both byte-identical (score 4) with only 2 allocnos — sets are cse-folded + flow-deleted PRE-RA; global.c never sees them; loop notes do not preserve them
- verdict: KILLED

## [s3] A DImode temp pair-homes into a1+a2 as a single zero-byte double occupant, pushing ptr to a3
- mechanism: multi-reg pseudo needs 2 consecutive hard regs; with conflicts {v0,v1,a0} the pair lands exactly a1+a2
- probe: s64 t=i (Z1) and u64 t=(u32)i (Z3, zero high half = cheapest conceivable), sandbox + .greg
- result: mechanism CONFIRMED by .greg (DImode pseudo conflicts exactly {v0,v1,a0}+ptr) but both score 22 — GCC 2.7.2 always emits both half-sets; no byte-free DImode spelling exists
- verdict: KILLED
