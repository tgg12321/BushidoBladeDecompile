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

## [s4] Random whole-function permuter mutation from the floor-4 candidate reaches a sub-20 (sub-floor) geometry
- mechanism: exotic statement geometries outside the manual s2/s3 sweeps could shift the RA census
- probe: campaign c1 (s4-rand-whole), 26.5k iters, fresh-seed window discipline
- result: 2 finds, both score-20 pointer-alias-handle attractor members (the s2-killed class); nothing sub-20
- verdict: KILLED

## [s4] Directed staged-load PERM cross-product (stagings x random) finds a home-preserving conflict injection
- mechanism: v07 proved staging flips ptr a1->a2; cross-product of staged spellings x random mutation might find a placement preserving w-homes
- probe: campaign c2 (s4-directed-staged), PERM_GENERAL staged forms on all three loads + PERM_RANDOMIZE, 31.7k iters
- result: 1 find, the same score-20 alias attractor; no staged variant beat base
- verdict: KILLED

## [s4] The v07-flip basin (ptr in a2, base 50) contains a mutation path to a home-preserving a3 flip
- mechanism: search the neighborhood of the ONLY known form with a flipped ptr-home for refinements restoring w0=v1/w1=a0/w2=a1
- probe: campaign c3 (s4-v07-flip-neighborhood) seeded from the v07 staged form, 15k iters, 12 finds
- result: basin descends 50->40->30->20 in <5 min but the descent RE-ENTERS the ptr=a1 attractor (best find honest-measured sandbox 4, identical residual); late 25; nothing sub-20
- verdict: KILLED

## [s4] The new c3-20 geometry's own neighborhood (idx-presb + double-staged w2) contains a sub-20 mutation
- mechanism: a different point in the score-20 attractor has a different local mutation neighborhood than the candidate
- probe: campaign c4 (s4-c3-20-neighborhood) seeded at the c3 best find, 31k iters
- result: 2 finds, both equal-score attractor members (alias handle + split multiply); the attractor is locally rigid
- verdict: KILLED

## [s4] PERMUTER MODALITY MEASURED NEAR-DEAD: 4 basins, ~104k iterations, zero
sub-20 finds; stochastic whole-function search independently corroborates the
s2/s3 structural closure. Remaining live axes: cc1 forensics on
post-conflict-build deletions (no-op-move coalescence / reg_may_share, reload
inheritance) + cc1psx calibration cross-check.

## [s4] Random whole-function permuter mutation from the floor-4 candidate reaches a sub-20 geometry
- mechanism: exotic statement geometries outside the manual s2/s3 sweeps could shift the RA census
- probe: campaign c1 s4-rand-whole, 26.5k iters, fresh-seed window discipline, harvested+stopped
- result: 2 finds, both score-20 pointer-alias-handle attractor members (the s2-killed class); nothing sub-20
- verdict: KILLED

## [s4] Directed staged-load PERM cross-product finds a home-preserving conflict injection
- mechanism: v07 proved staging flips ptr a1->a2; PERM_GENERAL staged spellings on all three loads x PERM_RANDOMIZE might find a placement preserving w-homes
- probe: campaign c2 s4-directed-staged, 31.7k iters, harvested+stopped
- result: 1 find, the same score-20 alias attractor; no staged variant beat base
- verdict: KILLED

## [s4] The v07-flip basin (ptr in a2, base 50) contains a mutation path to a home-preserving a3 flip
- mechanism: search the neighborhood of the only known ptr-home-flipping form for refinements restoring w0=v1/w1=a0/w2=a1
- probe: campaign c3 s4-v07-flip-neighborhood seeded from the v07 staged form, 15k iters, 12 finds, harvested+stopped
- result: basin descends 50->40->30->20 in under 5 min but re-enters the ptr=a1 attractor: best find (idx-presb + double-staged w2) honest-measured sandbox 4 with the IDENTICAL residual (move a1,a0 + three lw via a1 vs target a3); nothing sub-20
- verdict: KILLED

## [s4] The c3-20 geometry's own mutation neighborhood contains a sub-20 form
- mechanism: a different point in the score-20 attractor has a different local neighborhood than the candidate seed
- probe: campaign c4 s4-c3-20-neighborhood seeded at the c3 best find, 31k iters, harvested+stopped
- result: 2 finds, both equal-score attractor members (alias handle + split multiply); the attractor is locally rigid
- verdict: KILLED

## [s5] The DImode-pair basin (Z1 seed, base 325) contains a mutation path that sheds the pair-half bytes while keeping the pair pseudo alive across ptr
- mechanism: s3 proved a byte-free DImode pseudo would pair-home a1+a2 and push ptr to a3; stochastic search might find a spelling manual sweeps missed
- probe: campaign c5 s5-dimode-basin, 17,078 iters, full fresh-seed window, harvested+stopped
- result: basin descends 325->120->80->50->30->20 but every sub-30 step DEGRADES the s64 (int/long/uchar) into the known attractor; the sole pair-preserving byte-free form (output-30-1, score 30) does it via an upward-exposed UNINIT read (def moved below use) — semantically invalid, and ptr still a1
- verdict: KILLED

## [s5] The walker/flags-pointer basin (L7 seed, base 450) contains a pointer spelling matching target's at-macro addressing while keeping the conflicting pseudo
- mechanism: pseudo 75 genuinely conflicts with ptr; mutation might find an address form whose bytes match target
- probe: campaign c6 s5-walker-basin, 16,887 iters, full fresh-seed window, harvested+stopped
- result: every descent step REMOVES the pointer (folds back to direct global addressing) — score 20 reached exactly at the candidate's own form; no materialized-pointer member below 50
- verdict: KILLED

## [s5] PERMUTER MODALITY NOW FULLY DEAD: six independent basins (s4 random,
directed-staged, v07-flip, c3-20 + s5 DImode-pair, walker-pointer) across
~138k cumulative iterations all terminate in the score-20 ptr=a1 attractor
with zero sub-20 finds. NEW mechanism fact for the remaining axes: a
dead-def/uninit-use pseudo occupies a register byte-free (c5 output-30-1,
$a3) — the zero-byte-occupant channel EXISTS; frontier-1 cc1 forensics and
frontier-3 FAKE-family sweeps should target how flow/conflict-build treats
upward-exposed uses and whether any VALID construct reaches the same channel.

## [s5] The DImode-pair basin (Z1 seed, base 325) contains a mutation path shedding the pair-half bytes while keeping the pair pseudo alive across ptr
- mechanism: s3 proved a byte-free DImode pseudo would pair-home a1+a2 and push ptr to a3; stochastic search over exotic geometries might find a spelling manual sweeps missed
- probe: campaign c5 s5-dimode-basin via tools/permuter_campaign.py, 17,078 iters, full fresh-seed window, harvested+stopped
- result: descends 325->120->80->50->30->20 but every sub-30 step degrades the s64 into the known attractor; the sole pair-preserving byte-free form (output-30-1, score 30, 0 ins/del) uses an upward-exposed UNINIT read (def below use) — semantically invalid, ptr still a1
- verdict: KILLED

## [s5] The walker/flags-pointer basin (L7 seed, base 450) contains a pointer spelling matching target's at-macro addressing while keeping the conflicting pseudo
- mechanism: the walker pseudo genuinely conflicts with ptr (s3 .greg); mutation might find an address form whose emitted bytes match target
- probe: campaign c6 s5-walker-basin, 16,887 iters, full fresh-seed window, harvested+stopped
- result: every descent step removes the pointer (folds to direct global addressing); score 20 reached exactly at the candidate's own form; no materialized-pointer member below 50
- verdict: KILLED

## [s5] A dead-def pseudo can occupy a hard register with zero emitted bytes in this GCC
- mechanism: upward-exposed uninit use keeps the pseudo in the conflict graph after flow deletes/never-emits the def
- probe: honest asm diff of c5 output-30-1 vs target from a space-free /tmp copy (measure30.sh)
- result: CONFIRMED: 0 ins/del, 6 reg diffs; the DImode t occupies $a3 byte-free (build idx chain reads a3 garbage); ptr did not flip; only reachable spelling found is semantically invalid
- verdict: CONFIRMED

## [s6] The SN cc1psx homes ptr in a3 for this same C (compiler-fork RA divergence)
- mechanism: if the original compiler's global.c/local-alloc differed, no C spelling under our pinned toolchain reproduces the home without a real occupant
- probe: minimal TU of the candidate compiled by both tools/gcc-2.7.2/build/cc1 and tools/cc1psx_wrapper.sh (identical input, identical flags subset); full diff
- result: INSTRUCTION-IDENTICAL output — cc1psx also homes ptr in $5/a1 (move $5,$4 + three lw via $5), same schedule; only label syntax and banner differ
- verdict: KILLED

## [s6] A byte-free DImode pair-occupant would home a1+a2 and push ptr to a3 (s3 mechanism claim)
- mechanism: multi-reg pseudo with conflicts {v0,v1,a0} takes the first free consecutive pair
- probe: read mips.c mips_hard_regno_mode_ok init (lines 3439-3465)
- result: GP multi-word values require EVEN start regno; a1+a2 ($5,$6) is not a legal DImode placement — only (a2,a3) is available given the conflicts, which BLOCKS a3 and leaves a1 free; exactly matches c5 output-30-1's observed landing
- verdict: KILLED (architecturally impossible, stronger than s3's spelling-level kill)

## [s6] A preference (copy-pref or full-pref) could seat ptr in a3 over a free a1
- mechanism: find_reg's preference override (global.c:1057-1090) replaces the scanned best_reg with a preferred same-class reg
- probe: BB2_FINDREG_DEBUG=72 via instrumented tmp/gccdbg/cc1 (output verified identical to build cc1); read set_preference (global.c:1590-1675)
- result: all preference sets for pseudo 72 are EMPTY; preferences root only in SET insns pairing a hard reg with a pseudo; a call-free leaf has only the incoming a0 copy, so no a1/a2/a3 preference is reachable from any C spelling (original has no calls either)
- verdict: KILLED

## [s6] Some post-conflict-build deletion channel yields a VALID byte-free a1/a2 occupant (frontier-1)
- mechanism: pseudo present at conflict construction, its def deleted before final emission
- probe: source forensics — flow.c:1479 (insn_dead_p deletion, PRE-RA), final.c:1800-1806 (no-op move deletion, requires REGNO(src)==REGNO(dst)), plus the s5 uninit channel
- result: enumeration closes every channel for valid C: dead defs die pre-RA (never reach conflicts); coalesced copies occupy only the SOURCE's home ∈ {v0,v1,a0}; DImode lands a2+a3; a valid upward-exposed use must alias an existing same-reg value (no new conflict) or force a real def (bytes); the sole byte-free spelling is the invalid uninit read
- verdict: KILLED (no valid C with this 34-insn shape can home ptr in a3 — closure theorem, s6 evidence)

## [s6] The original had a wider signature whose unused params pressured RA (re-test of s2 kill with measurement)
- mechanism: unused parm copies or entry liveness of a1-a3 conflict with ptr
- probe: ANSI 4-param, 2-param, K&R-style 4-param variants; .greg census + asm diff each
- result: all three byte-identical with identical conflict sets ({i,2,3,4,29}) and ptr in a1; unused parm copies flow-deleted, zero residual entry liveness
- verdict: KILLED (now measured, was reasoning-only)

## [s6] The SN cc1psx homes ptr in a3 for this same C (compiler-fork RA divergence)
- mechanism: if the original compiler's allocator differed, no C under our pinned toolchain reproduces a3 without a real occupant
- probe: minimal TU of the candidate compiled by both our cc1 and tools/cc1psx_wrapper.sh (identical input); full asm diff
- result: instruction-identical output — cc1psx also homes ptr in $5/a1 (move $5,$4 + three lw via $5), same schedule; only label syntax and banner differ
- verdict: KILLED

## [s6] A byte-free DImode pair-occupant would home a1+a2 and push ptr to a3 (s3 mechanism)
- mechanism: multi-reg pseudo with conflicts {v0,v1,a0} takes the first free consecutive pair
- probe: mips.c mips_hard_regno_mode_ok init (3439-3465)
- result: GP multi-word values require EVEN start regno: a1+a2 ($5,$6) is illegal; only (a2,a3) fits the conflicts, which blocks a3 and leaves a1 free — exactly matches c5 output-30-1's landing
- verdict: KILLED

## [s6] A copy/full preference could seat ptr in a3 over a free a1
- mechanism: find_reg's preference override (global.c:1057-1090) replaces the scanned best_reg
- probe: BB2_FINDREG_DEBUG=72 via instrumented tmp/gccdbg/cc1 (output verified identical to build cc1) + set_preference source read (global.c:1590-1675)
- result: all preference sets for pseudo 72 EMPTY; preferences root only in hard-reg SETs; a call-free leaf has only the incoming a0 copy — channel unreachable from any C spelling (target has no calls either)
- verdict: KILLED

## [s6] Some post-conflict-build deletion channel yields a VALID byte-free a1/a2 occupant (frontier-1)
- mechanism: pseudo present at conflict construction whose def is deleted before final emission
- probe: source forensics: flow.c:1479 (dead-def deletion PRE-RA), final.c:1800-1806 (no-op move deletion requires REGNO(src)==REGNO(dst)), s5 uninit channel semantics
- result: closure: dead defs never reach conflicts; coalesced copies occupy only the source's home ({v0,v1,a0} here); DImode lands a2+a3; a valid upward-exposed use must alias an existing same-reg value (no new conflict) or force a real def (bytes); only the invalid uninit read is byte-free
- verdict: KILLED

## [s6] The original had a wider signature whose unused params pressured RA (s2 kill re-tested with measurement)
- mechanism: unused parm copies or a1-a3 entry liveness conflict with ptr
- probe: ANSI 4-param, 2-param, K&R 4-param variants; .greg census + asm diff each
- result: all three byte-identical, identical conflict sets, ptr in a1; unused parm copies flow-deleted with zero residual entry liveness
- verdict: KILLED

## [s7] Dead-store / named-local FAKE placements supply a1/a2 occupants byte-free (frontier-3, ds/nl half)
- mechanism: a dead-store or named-copy pseudo overlapping ptr's range could enter the conflict graph
- probe: 5 variants (fake=i tail, fake=arg0[0] entry, two dead stores, k0/k1/k2 keep-copies, slot=i live copy), per-variant -da dumps
- result: ALL byte-identical, census-identical (2 allocnos); dead-store pseudo present in .rtl, gone in .jump — jump_optimize pass 1 deletes it pre-cse/pre-RA; named copies coalesce in cse. s6 prediction confirmed (stronger: jump1, not flow)
- verdict: KILLED

## [s7] Duplicated-statement-into-arms placements are RA-inert here (s6 theorem prediction)
- mechanism: s6 predicted duplication only lifts allocation priority, order outcome-irrelevant
- probe: 22 arms variants across 4 rounds (natural preds, invented conds, arm content sweeps), .greg + sdiff each
- result: PREDICTION WRONG — arms duplication reshapes the conflict graph byte-free: duplicated arm pseudos are real at RA, jump2 cross-jump merges the identical arms (.greg 21 -> .jump2 16 insns), jump.c delete_computation deletes cond computes (sltiu in .greg, absent in final). ptr moved off a1 in multiple forms; dupU homed ptr in a3 (first valid-C ever) with w0=v1, w1=a0 correct
- verdict: KILLED as stated (the family is NOT inert) — replaced by the channel-(f) frontier

## [s7] Some arms geometry reaches the full target seating (idx v0, w0 v1, w1 a0, w2 a1, ptr a3) byte-free
- mechanism: merged-away duplicate/cond pseudos fill a1+a2 during ptr's range while locals keep target seats
- probe: 22 variants: arm content {sw3, sw2+sw3, loads, whole tail, shadow copies idx2/w2b}, conds {i<6, i<3, arg0!=0, (u32)arg0<2U, <3U, big-const, reg-reg equality, nested/3-arm}; honest sandbox on dupX1 and dupM
- result: two rigid outcomes only — sw3-arms attractor (idx a1, w2 a2, ptr a0-coalesced; honest 11) and big-const rotation (w0/w1 correct, ptr a2, w2 a3, idx a1, stray li; honest 11). Globalized idx always loses v0; deletable temps seat low; reg-reg conds self-destruct via cse else-arm specialization. No form beat 4
- verdict: KILLED for every spelling tried (family floor 11 so far); residue: temps seated exactly at a2, label-placement merge steering — pending the sanctioning ruling

## [s8] The s7 channel-(f) residue (a2-seated deletable temps, label-steered merges) remains a live sanctioned axis
- mechanism: those spellings all require the manufactured identical-arms branch as the carrier
- probe: 2026-07-21 00:19 Judge ruling on the s7 ruling-request (docs/grind/decisions.md)
- result: FAIL — the invented branch is a cheat-by-spelling outside the duplicated-statement-into-arms sanction; channel (f) via manufactured branches is CLOSED and the residue dies with it; ruling verifies every sanctioned axis is now measured dead
- verdict: KILLED

## [s8] func_80033550 qualifies for canonical-asm under endgame-lock-disposition criterion 1
- mechanism: authorization requires STRONG scan_hand_coded signals (S1/S2/S6 class)
- probe: python3 tools/scan_hand_coded.py --single func_80033550 (tmp/grind/func_80033550/s8/scan_hand_coded.txt)
- result: LOW 0/8, "no strong hand-coded indicators"; corroborated by s6 cc1psx instruction-identity (ordinary GCC 2.7.2 output, RA tiebreak artifact)
- verdict: KILLED (canonical-asm refused; escalation option (a) stated as not supportable)

## [s8] ESCALATION FILED: every sanctioned axis measured dead (s2/s3 structural, s4/s5 permuter 6 basins ~138k iters, s6 closure theorem + cc1psx, s7 FAKE-family sweep + Judge-closed channel (f)); OWNER-ESCALATION filed in docs/grind/decisions.md 2026-07-21 per endgame-lock-disposition; options (a) canonical-asm (not supportable, LOW 0/8) vs (b) INCOMPLETE-owner-accepted with the asm("a3") pin retained to hold the match. Awaiting owner ruling — do not self-resolve, do not re-grind dead axes.

## [s7] The s7 channel-(f) residue (a2-seated deletable temps, label-steered merges) remains a live sanctioned axis
- mechanism: all such spellings require the manufactured identical-arms branch as carrier
- probe: 2026-07-21 00:19 Judge ruling on the s7 ruling-request (docs/grind/decisions.md)
- result: FAIL: the invented branch is a cheat-by-spelling outside the duplicated-statement-into-arms sanction; channel (f) via manufactured branches is closed and the residue dies with it; the ruling itself verifies every sanctioned axis is now measured dead
- verdict: KILLED

## [s7] func_80033550 qualifies for canonical-asm under endgame-lock-disposition criterion 1
- mechanism: authorization requires STRONG scan_hand_coded signals (S1/S2/S6 class); LOW is dispositive refuse
- probe: python3 tools/scan_hand_coded.py --single func_80033550 (tmp/grind/func_80033550/s8/scan_hand_coded.txt)
- result: LOW 0/8, no strong hand-coded indicators; corroborated by s6 cc1psx instruction-identity (ordinary GCC output, RA tiebreak artifact)
- verdict: KILLED

## [s7] The floor and the pin's score-inertness are unchanged at escalation time
- mechanism: sandbox strips the asm("a3") pin before scoring, so pinned and pin-free forms measure identically
- probe: sandbox func_80033550 --disable all on HEAD's pinned form, then on the re-applied pin-free candidate
- result: pinned form 4 / cheat_asm_stripped 371; pin-free candidate 4 / 369 (identical JSON to s2-s7 baselines); candidate left in place in src
- verdict: CONFIRMED

## [s9] The inverse solver's LEVER atom (pseudo 74 acquires an $a3 copy preference) is reachable by an argument-flow / call-return C spelling.
- mechanism: global.c set_preference records a hard-reg copy preference when a
  SET copies between a pseudo and a hard reg; find_reg then tries the preferred
  register first, so pseudo 74 preferring $a3 would home arg0's pointer there.
- probe: (a) read the pre-RA RTL (tmp/inverse_work/ra/code6cac_b.i.lreg) for
  every hard GPR the function's RTL actually contains; (b) brute-force the
  solver's own validated forward model over refs 1..40 x livelen 1..160 for
  every focus pseudo, checking whether ANY flow configuration alone reaches
  74 -> $a3.
- result: FORECLOSED, two independent ways. (a) The ONLY hard GPR anywhere in
  this function's pre-RA RTL is $a0 (regno 4) — the function is call-free (zero
  jal in target) and single-parameter, so $a3 never appears and set_preference
  can never record a preference for it; expand_preferences can only propagate
  preferences that already exist. Creating one would require a 4-argument call
  or a 4th incoming parameter, neither of which the target's 34-insn shape
  admits. (b) The grid sweep returns NO (refs, livelen) configuration for any
  focus pseudo that reaches $a3 — the preference is the model's only route, and
  it is not C-reachable. This is an independent empirical confirmation of the s6
  closure theorem's "set_preference needs a hard-reg SET = calls" clause.
  NB the sweep report's LEVER verdict is an artifact: inverse.py generates a
  PREF_ADD atom for any goal register without checking that the register can
  appear as a hard reg in that function's RTL.
- verdict: KILLED

## [s10] The 2026-08-19 unpark's F6 lever (empty-if / fabricated redundant condition) injects a byte-free RA conflict here
- mechanism: F6 (.claude/rules/no-new-park-categories.md:371, ESTABLISHED
  2026-08-18) sanctions a fabricated redundant condition / empty-if inserted
  solely for codegen. If such a condition's compare pseudo survived into
  global.c's conflict graph it would be exactly the byte-free conflict fuel
  s1-s7 never found, seating $a1/$a2 and pushing arg0's pointer pseudo 72
  toward the target's $a3.
- probe: 5 empty-if placements (after `found`, after new_var, after the w2
  read, `if (!arg0)`, and the `if (arg0 && arg0)` redundant-condition
  spelling) — tmp/grind/func_80033550/s8/variants/f6_emptyif_*.c; sandbox
  --disable all on each, plus full -da pass dumps for one.
- result: KILLED, all five byte-identical to baseline (score 4, 34 insns) and
  RA-identical. Pass named from the dumps: the compare+branch exists in
  code6cac_b.rtl (35 insns vs baseline 34) and is GONE by code6cac_b.jump
  (24 == baseline 24) — jump_optimize pass 1 (jump.c: jump-to-next-insn
  deletion, then the compare dies as unused). Every dump from .jump onward,
  .lreg/.greg included, is byte-identical to baseline: 2 allocnos (74 72),
  72 conflicts {72 74 2 3 4 29}, 72 in 5 ($a1). The F6 empty-if shape shares
  the fate of the s7 dead-store / named-local family: deleted pre-RA, zero
  conflict-graph effect.
- verdict: KILLED

## [s10] The other F6 shape (cancellation pair `x++; x--;`) injects a byte-free conflict that seats $a1/$a2
- mechanism: same as above; the pair's intermediate pseudo would be a real
  allocno in global.c's conflict graph while emitting no bytes.
- probe: 16 placements over every local and both orderings —
  tmp/grind/func_80033550/s8/variants2/ (i, idx, new_var, w0, w2, a fresh
  staged local, two-pair combos) and variants3/ (w0/w1 pairs interleaved
  between the three arg0 reads, plus 2-pair combinations); sandbox on each,
  -da dumps for the two representative outcomes.
- result: KILLED by a sharp dichotomy that is itself the finding.
  (i) Pair on a variable whose live range does NOT overlap pointer pseudo 72
  (i, idx, new_var, fresh staged local, w1/w2 after the last arg0 read):
  survives jump/cse/loop/cse2 (24 insns vs baseline 22) and is deleted at
  .flow (21 == baseline 21) by flow.c life-analysis DCE, pre-RA. Byte-free
  (4 / 34 insns) but .greg is byte-identical to baseline — INERT.
  (ii) Pair on a value consumed across 72's live range (w0 before w1, w0
  between w1/w2, w1 before w2, and both 2-pair combinations): the pair
  SURVIVES .flow (23) and .combine (21) into global.c as a real allocno that
  conflicts with 72 AND with hard 2,3,4 — genuine byte-free-looking conflict
  injection, the first ever from a sanctioned family. BUT the same liveness
  extension makes the entry copy `move a1,a0` coalescable: .greg prints
  `72 preferences: 4` (absent in every baseline dump) and 72 LOSES its
  hard-reg-4 conflict, so global.c find_reg takes the PREFERRED $a0 before
  the numeric v0,v1,a0,a1,a2,a3 scan runs at all. Result 33 insns (entry move
  deleted), distance 6 (w0-after-w2, w0-between-w1/w2, w1-before-w2) or 7
  (w0-before-w1 and both 2-pair forms). Never byte-free, always the wrong
  direction ($a0, not $a3).
- verdict: KILLED

## [s10] The F6+F7 seam (fabricated condition carrying duplicated identical arms) is a live path to distance 0
- mechanism: the 2026-08-19 stale-park re-audit unparked this function citing
  an "F6+F7 seam": F6 now sanctions fabricating a redundant condition, F7
  sanctions duplicating common-tail stores into both if/else arms (SOTN ships
  25 fully-identical-arm if/else constructs), and the s7 channel-(f) forms
  are exactly that composition — an invented identical-arms branch merged
  away by jump2 cross_jump, the only construct in 10 sessions that ever
  homed 72 in $a3.
- probe: (a) re-measure both banked seam forms on the post-migration chassis
  (rejected/dup-arms-bigconst-cond-ptr-a2-w2-a3-stray-li-11.c and
  rejected/dup-arms-sw3-attractor-idx-a1-w2-a2-ptr-coalesced-11.c); (b) test
  F7's own precondition ("the stores' values real and required" in both arms)
  against this function's control flow.
- result: KILLED, and the seam is MOOT rather than merely unruled.
  (a) Both banked seam forms are chassis-invariant at honest 11 (36 and 33
  insns) versus the floor-4 candidate — measured this session, same numbers
  as s7. Even a hypothetical owner ruling that blessed the composition could
  not close the function: no seam form has ever scored below 11, and the s7
  22-variant sweep floors the sanctioned (natural-arms) form at 11 too.
  (b) F7 is inapplicable on its own terms: the only real branches are the
  search loop's two exits and `if (i == 6) return;`, whose arms merge before
  the tail; the tail is straight-line, and duplicating the six tail stores
  into the i==6 arm would write the table when it is full — a semantic
  change, so the "values real and required" prerequisite fails. F7 has no
  pre-existing if/else in this function to duplicate into.
  Therefore no ruling-request is warranted: the seam question cannot change
  the disposition, because the construct it would authorize is measured 7
  points WORSE than the honest floor.
- verdict: KILLED

## [s8] The 2026-08-19 unpark's F6 empty-if / fabricated-redundant-condition shape injects a byte-free RA conflict that seats $a1/$a2 and pushes arg0's pointer pseudo 72 toward the target's $a3.
- mechanism: F6 (.claude/rules/no-new-park-categories.md:371) sanctions a fabricated redundant condition / empty-if inserted solely for codegen; if its compare pseudo survived into global.c's conflict graph it would be the byte-free conflict fuel s1-s7 never found.
- probe: 5 placements (if(!i){} after the `found` label, after new_var = i*12, if(!w2){} after the w2 read, if(!arg0){}, if(arg0 && arg0){}) in tmp/grind/func_80033550/s8/variants/; sandbox --disable all on each plus full -da pass dumps on one.
- result: All five score 4 / 34 insns, byte-identical to baseline. Pass named from the dumps: the compare+branch is present in code6cac_b.rtl (35 insns vs baseline 34) and gone by code6cac_b.jump (24 == baseline 24) - jump_optimize pass 1 (jump.c) deletes the jump-to-next-insn and the compare dies with it, before any allocation pass. Every dump from .jump onward including .lreg/.greg is byte-identical to baseline: 2 allocnos (74 72), 72 conflicts {72 74 2 3 4 29}, dispositions 72 in 5 ($a1). Zero conflict-graph effect - the same fate as the s7 dead-store / named-local family.
- verdict: KILLED

## [s8] The other F6 shape, the cancellation pair `x++; x--;`, injects a byte-free conflict that seats $a1/$a2.
- mechanism: the pair's intermediate pseudo would be a real allocno in global.c's conflict graph while emitting no bytes.
- probe: 16 placements over every local and both orderings: tmp/grind/func_80033550/s8/variants2/ (i, idx, new_var, w0, w2, a fresh staged local, two-pair combos) and variants3/ (w0/w1 pairs interleaved between the three arg0 reads); sandbox on each, -da dumps for the two representative outcomes.
- result: Sharp dichotomy, both halves dead. (i) Pair NOT overlapping pointer pseudo 72's live range: survives jump/cse/loop/cse2 (24 insns vs baseline 22) but is deleted at .flow (21 == baseline 21) by flow.c life-analysis DCE, pre-RA; .greg byte-identical to baseline - byte-free but INERT. Special case `w2++; w2--;` right after the last arg0 read IS byte-free and does promote a new allocno (`3 regs to allocate: 77 74 72`, 77 conflicts {77 2 3 4 29}), but 77 cannot conflict with 72 (72 is already dead there), so both sit in $a1 and nothing moves. (ii) Pair on a value consumed across 72's range (w0 before w1, w0 between w1/w2, w1 before w2, and both two-pair combos): survives .flow (23) and .combine (21) into RA as a real allocno conflicting with 72 and with hard 2,3,4 - but the same liveness extension makes the entry copy `move a1,a0` coalescable, .greg gains `72 preferences: 4` (never present in baseline) and 72 LOSES its hard-reg-4 conflict, so global.c find_reg takes the preferred $a0 before the numeric v0,v1,a0,a1,a2,a3 scan runs at all. 33 insns (entry move deleted), distance 6 or 7. Measured: c5 6, d1 7, d2 6, d3 6, d4 7, d5 7, d6 6, d8 7.
- verdict: KILLED

## [s8] The F6+F7 seam (a fabricated F6 condition carrying F7-duplicated identical arms) is a live path to distance 0 and needs an owner ruling.
- mechanism: F6 now sanctions fabricating a redundant condition and F7 sanctions duplicating common-tail stores into both if/else arms (SOTN ships 25 fully-identical-arm if/else constructs); the s7 channel-(f) forms are exactly that composition - an invented identical-arms branch merged away by jump2 cross_jump, the only construct in 10 sessions that ever homed pseudo 72 in $a3.
- probe: (a) re-measure both banked seam forms on the post-migration chassis; (b) test F7's own prerequisite (the duplicated stores' values must be real and required in both arms) against this function's actual control flow.
- result: MOOT, not merely unruled. (a) rejected/dup-arms-bigconst-cond-ptr-a2-w2-a3-stray-li-11.c = 11 (36 insns) and rejected/dup-arms-sw3-attractor-idx-a1-w2-a2-ptr-coalesced-11.c = 11 (33 insns), identical to their s7 numbers - chassis-invariant and 7 points WORSE than the floor-4 candidate, so a ruling authorizing the composition could not close the function. (b) F7 is inapplicable on its own terms: the only real branches are the search loop's two exits and `if (i == 6) return;`, all merging above the tail; the tail is straight-line, and duplicating the tail stores into the i==6 arm would write the table when it is full (a semantic change), so the 'real and required' prerequisite fails and there is no pre-existing if/else to duplicate into. No ruling-request filed; the standing Judge constraint on manufactured branches is not re-tested.
- verdict: KILLED

## [s9] 2026-08-20 — escalation modality (disposition session)

- H-s9.1 "The honest floor has moved on the post-migration chassis."
  KILLED. sandbox --disable all with candidate.c applied = 4 (34/34 insns,
  0 rules). Identical to s1-s8. Chassis-invariant.

- H-s9.2 "The phantom-frame-slot volatile pad family (2026-08-18) applies —
  frontier item 2." KILLED ON PREREQUISITE. Target has no stack frame
  whatsoever (no sp adjustment, no callee-save, `jr $ra`+`nop` epilogue across
  all 34 insns), so there is no untouched slot to reserve and a pad local can
  only ADD bytes. Do not re-open.

- H-s9.3 "A rederive-modality reconstruction could reach the target seating —
  frontier item 3." NOT INDEPENDENTLY MEASURED, but now SUBSUMED by an
  explicit numeric requirement rather than left open-ended: whatever shape a
  rederive produces, seating the pointer in $a3 requires pseudo-72's
  hard-conflict set to contain {2,3,4,5,6} with an empty preference set. The
  target body carries only FOUR register-carrying values across the pointer's
  range (const-6/idx in $v0, i-then-w0 in $v1, w1 in $a0, w2 in $a1), so the
  FIFTH occupant ($a2) must be byte-free — the same closed channel s5/s7/s8
  measured (uninit-read occupant is UB; F6 pairs are jump1/flow.c-deleted or
  flip the copy to coalescable; manufactured arms are Judge-FAILed and cost
  11). A rederive that changes the instruction count pays that change as
  distance. Recorded so a future session can attack the arithmetic directly
  instead of re-searching shapes.

- H-s9.4 "An endgame-lock AND-gate passes." KILLED (both). Gate #1
  scan_hand_coded = LOW 0/8, all of S1-S8 negative. Gate #2 SOTN-master census
  negative for a byte-free register occupant; `pad_dummy_local` is a frame-slot
  family with an unmet prerequisite here. -> standing 2026-07-27 ruling applied,
  disposition entry filed at docs/grind/decisions.md:8216, result=owner-gated.

## [s9] The honest floor has moved on the post-asm-until-matched chassis.
- mechanism: chassis re-measurement; the ledger's floor conclusions are chassis-relative and must be re-verified before being spent.
- probe: HEAD (INCLUDE_ASM) sandbox --disable all = 34 (no C body); candidate.c applied verbatim to src/code6cac_b.c (CRLF normalised to LF) = score 4, target_insns 34, build_insns 34, rules_dropped 0, zero cheat-asm.
- result: floor 4, unchanged from s1-s8; chassis-invariant.
- verdict: KILLED

## [s9] The phantom-frame-slot volatile pad family (2026-08-18, no-new-park-categories.md) applies here (live frontier item 2).
- mechanism: an unused volatile pad local reserving target's untouched stack bytes, changing frame size and allocation pressure.
- probe: direct inspection of all 34 instructions of asm/funcs/func_80033550.s for a frame: no addiu $sp,$sp,-N, no callee-save store, epilogue is jr $ra + nop.
- result: target func_80033550 has NO stack frame at all, so there is no untouched target slot to reserve and any pad local can only ADD prologue/epilogue bytes. The family's prerequisite is unmet by direct measurement.
- verdict: KILLED

## [s9] AND-gate #1 (canonical-asm, hand-written-asm evidence) passes for func_80033550.
- mechanism: tools/scan_hand_coded.py STRONG signals S1/S2/S6 are the owner's criterion for a canonical-asm grant.
- probe: python3 tools/scan_hand_coded.py --single func_80033550 re-run this session on the current chassis.
- result: tier=LOW score=0/8; S1-S8 all negative (0 multu/mflo pairs, no empty-body branch, S3/S4 N/A at 34<40 insns, no sibling cluster, no BIOS jumptable, no unsaved $sN, no redundant mask). Corroborated by s6's cc1psx instruction-identical result: the divergence is ordinary GCC RA, not hand-written asm.
- verdict: KILLED

## [s9] AND-gate #2 (an in-hand SOTN-master precedent for the closing construct) passes.
- mechanism: the closing construct here would be a byte-free REGISTER occupant seated in $a1 and $a2 across the pointer's live range; the frozen family list is owner-only to extend and needs a cited PSX-master precedent.
- probe: searched docs/reference/sotn-construct-index.md (1,365 PSX-master entries) for uninitialised/garbage/occupant/pressure/dummy-local constructs.
- result: no entry for a byte-free register occupant. The only adjacent family is pad_dummy_local (index line 29, 816 entries) which is a FRAME-SLOT family whose prerequisite fails here (no stack frame in target). The one construct that ever homed the pointer in $a3 - an invented identical-arms branch cross-jump-merged by jump2 - was Judge-FAILed 2026-07-21 as a cheat-by-spelling and is moot anyway (both banked seam forms re-measure at distance 11 vs the floor of 4).
- verdict: KILLED

## [s10] 2026-08-25 — escalation modality (owner directive 2026-08-24: F1 chain-extender)

- H-s10.1 "The honest floor has moved on the current chassis."
  KILLED. `memory/grind/func_80033550/candidate.c` spliced over
  `INCLUDE_ASM("asm/funcs", func_80033550);` at src/code6cac_b.c:1855 ->
  `sandbox func_80033550 --disable all` = **score 4, target_insns 34,
  build_insns 34, rules_dropped 0**. Chassis-invariant across s1-s10.

- H-s10.2 "The sanctioned F1 combine-foldable chain-extender
  (.claude/rules/dead-store-fake-exception.md:32-46) can move pseudo 72 (the
  arg0 pointer) out of $a1 — the owner's 2026-08-24 named directive."
  KILLED, three measured modes + a solver verdict. The family's ONLY named
  mechanism is the extra `reg_n_refs` flow.c records before combine folds the
  detour. Measured here:
    (A) plain alias `s32 *p = arg0;` — the copy dies in cse/jump BEFORE flow,
        so refs are never bumped: the extracted model is byte-identical to
        baseline (nrefs 5, livelen 16, pri 6250, hard conflicts {2,3,4,29},
        seat $a1). Fully INERT. sandbox 4, 34/34.
        rejected/s10-f1-chain-plain-alias-inert-4.c
    (B) folding detour `s32 *p = arg0 + 1;` feeding p[-1]/p[0]/p[1] — combine
        rebases all three lw offsets and the detour REPLACES the entry copy.
        Insn count holds at 34 but the emitted instruction changes from
        `addu a1,a0,zero` to `addiu a1,a0,4`, so it is count-neutral, not
        byte-neutral. Model moves (livelen 16->17, pri 6250->5882) but nrefs
        stays 5 and the hard-conflict set stays {2,3,4,29}; find_reg's
        ascending scan still returns $a1. sandbox 4, 34/34.
        rejected/s10-f1-chain-folding-detour-slot-substitutes-move-4.c
    (C) partial detour (arg0 feeds loads 0,1; `s32 *p = arg0 + 2;` feeds load
        2) — the ONLY spelling in 10 sessions that splits the pointer into TWO
        surviving global allocnos at 34 insns: 74=$v1(i), 72=$a0(arg0, nrefs 5,
        livelen 16), 79=$a1(detour, nrefs 2, livelen 17). arg0's hard-conflict
        set LOSES 4, so the pointer keeps $a0 and the seat moves the WRONG way;
        the second occupant is NOT byte-free — it consumed the entry-copy slot
        (`addiu a1,a0,8` in place of `addu a3,a0,zero`). sandbox 4, 34/34.
        rejected/s10-f1-chain-partial-detour-eats-entry-copy-slot-4.c
  Structural reason, and it generalizes (same law CD_sync s105 derived): the
  three loads ALREADY use the direct base+offset addressing form and arg0 is a
  runtime parameter, so every algebraically-neutral detour on it is either
  folded before flow.c counts it (mode A, inert) or survives to RA only by
  taking over an existing instruction slot (modes B/C, byte-changing). There is
  no third mode. The chain-extender axis of the 2026-08-24 directive is spent.

- H-s10.3 "Some perturbation of the modelled RA inputs reaches the target seat
  ($a3) for the pointer." KILLED by the ra_solver inverse backend — the first
  typed verdict this function has ever had.
  `python3 tools/ra_solver/extract.py func_80033550 code6cac_b` +
  `python3 tools/ra_solver/inverse.py global tmp/ra_solver_work/func_80033550.model.json
   --goal '{"72": 7}' --depth 2` and `--depth 3`:
    * baseline reproduced exactly: {72: '$a1', 74: '$v1'};
    * **NEGATIVE RESULT** at depth 2 AND depth 3 over 48 single perturbations in
      5 classes (refs +12/-6, live length +/-2,4,8, birth order, pseudo-pseudo
      conflict adds, calls-crossed, preference add/reroute/clear);
    * **FORECLOSED** on the preference route with a named mechanism: "$a3 never
      appears as a hard reg in this function's pre-RA RTL, so global.c
      set_preference can never record a preference for it."
  Scope note recorded honestly for the next session: `inverse.py`'s
  CONFLICT_ADD atoms are pseudo-pseudo only (over the focus set {72,74}); there
  is no HARD-conflict-add atom, so the solver does not search "make $a1 and $a2
  hard-conflict with 72". That single unsearched axis is exactly the closed
  channel s5/s7/s8/s9 already measured (it requires two byte-free register
  occupants alive across the pointer, and mode C above shows the first such
  occupant costs the entry-copy slot).
  Artifacts: tmp/grind/func_80033550/s10/inverse_d2.txt, inverse_d3.txt.

- H-s10.4 "An endgame-lock AND-gate passes." KILLED (both, re-measured).
  Gate #1 `tools/scan_hand_coded.py --single func_80033550` = **tier=LOW
  score=0/8**, S1-S8 all negative (tmp/grind/func_80033550/s10/scan_hand_coded.txt).
  Gate #2 no in-hand SOTN-master precedent: the closing construct would still be
  a byte-free REGISTER occupant, and the s9 census of
  docs/reference/sotn-construct-index.md (1,365 PSX entries) returned nothing;
  mode C above independently shows the construct is not byte-free here.

## [s10] The honest floor has moved on the current chassis.
- mechanism: chassis re-measurement — every banked spelling conclusion is chassis-relative and must be re-verified before it is spent.
- probe: memory/grind/func_80033550/candidate.c spliced over INCLUDE_ASM("asm/funcs", func_80033550); at src/code6cac_b.c:1855 (CRLF normalised to LF), then `sandbox func_80033550 --disable all`.
- result: score 4, target_insns 34, build_insns 34, rules_dropped 0. Identical to s1-s9.
- verdict: KILLED

## [s10] The sanctioned F1 combine-foldable chain-extender (.claude/rules/dead-store-fake-exception.md:32-46) can move pseudo 72 (the arg0 pointer) out of $a1 — the owner's named 2026-08-24 directive.
- mechanism: the family's ONLY named mechanism is the extra reg_n_refs flow.c records before combine folds the algebraically-neutral detour back to the direct form with zero emitted bytes; that lifts the allocno's global.c priority.
- probe: three spellings compiled and measured, each re-extracted through tools/ra_solver/extract.py: (A) plain alias `s32 *p = arg0;`; (B) folding detour `s32 *p = arg0 + 1;` feeding p[-1]/p[0]/p[1]; (C) partial detour with arg0 feeding loads 0,1 and `s32 *p = arg0 + 2;` feeding load 2.
- result: (A) INERT — the copy dies in cse/jump before flow.c counts it; the model is byte-identical to baseline (nrefs 5, livelen 16, pri 6250, hard conflicts {2,3,4,29}, seat $a1); sandbox 4, 34/34. (B) count-neutral but NOT byte-neutral — combine rebases all three lw offsets and the detour survives only by replacing the entry copy, emitting `addiu a1,a0,4` where target has `addu a3,a0,zero`; model moves (livelen 16->17, pri 6250->5882) but nrefs and the hard-conflict set do not, so find_reg still returns $a1; sandbox 4, 34/34. (C) splits the pointer into TWO surviving global allocnos at 34 insns for the first time in ten sessions (74=$v1 i; 72=$a0 arg0; 79=$a1 detour) — arg0's hard-conflict set loses 4 and the seat moves the WRONG way, and the second occupant again consumes the entry-copy slot (`addiu a1,a0,8`); sandbox 4, 34/34. Structural law: the three loads are already in direct base+offset form and arg0 is a runtime parameter, so a detour is either folded pre-flow (inert) or slot-substituting (byte-changing) — there is no third mode.
- verdict: KILLED

## [s10] Some perturbation of the modelled register-allocation inputs reaches the target seat ($a3) for the arg0 pointer.
- mechanism: global.c allocno_compare priority + find_reg's ascending scan; the solver searches refs, live length, birth order, pseudo-pseudo conflicts, calls-crossed and preferences and maps each surviving vector to a C lever.
- probe: tools/ra_solver/extract.py func_80033550 code6cac_b, then tools/ra_solver/inverse.py global tmp/ra_solver_work/func_80033550.model.json --goal '{"72": 7}' at --depth 2 and --depth 3.
- result: NEGATIVE RESULT at both depths over 48 single perturbations in 5 classes (refs +12/-6, live length +/-2/4/8, birth order, conflict adds, calls-crossed, preference add/reroute/clear), plus an explicit FORECLOSED verdict on the preference route with its mechanism named: '$a3 never appears as a hard reg in this function's pre-RA RTL, so global.c set_preference can never record a preference for it.' Honest scope limit recorded: inverse.py's CONFLICT_ADD atoms are pseudo-pseudo only, so 'make $a1 and $a2 HARD-conflict with 72' is not searched — and that single unsearched axis is exactly the byte-free-occupant channel s5/s7/s8/s9 already measured closed, which mode (C) above independently prices at one instruction slot.
- verdict: KILLED

## [s10] An endgame-lock AND-gate passes for func_80033550.
- mechanism: gate #1 = STRONG scan_hand_coded signals (S1/S2/S6) authorise a canonical-asm grant; gate #2 = an in-hand SOTN-master precedent authorises a coercion/spelling family.
- probe: python3 tools/scan_hand_coded.py --single func_80033550 re-run this session; s9's census of docs/reference/sotn-construct-index.md (1,365 PSX-master entries) re-affirmed against this session's measurement.
- result: Gate #1 tier=LOW score=0/8, S1-S8 all negative (0 multu/mflo pairs, no empty-body branch, S3/S4 N/A at 34 < 40 insns, no sibling cluster, no BIOS jumptable, no unsaved $sN, no redundant mask). Gate #2 no precedent for a byte-free register occupant; the adjacent pad_dummy_local family is frame-slot-based and target func_80033550 has no stack frame at all — and mode (C) measures that a surviving second occupant is not byte-free in this 34-insn shape.
- verdict: KILLED
