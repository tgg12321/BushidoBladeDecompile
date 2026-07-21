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
