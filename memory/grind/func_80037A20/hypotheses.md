# Hypothesis ledger — func_80037A20

Floor = 13 (honest, cheat-free). HEAD "matches" only via 2 register-asm pins +
1 __asm__ opt-barrier (all cheats). candidate.c = pin-free faithful body @ 13.

The 13 diffs = ~12 from the s0<->s1 register SWAP + 1 from the entry-increment
`li s0,1` vs target `addiu s1,s1,1` FOLD. GREG-confirmed (s1): the swap is a
global.c priority wall (counter pseudo-75 out-prioritizes pointer pseudo-74 for
s0; pointer's live range is forced long by GCC hoisting the loop-invariant
`la D_80102810` to the top -> conflicts a2/a3 -> lower priority). The fold is a
separate cse2 const-prop (pre-allocation).

## KILLED (measured)
- H1 pointer-init-after-call (trim pointer live range): sandbox 16, greg
  identical -> address load hoisted regardless of statement order. [s1]
- (WIP) do-while no-entry-increment: wrong count. decl reorder: alloc unchanged.
  nextfile-in-if-condition: 34 insns +nop. firstfile->v0_temp: 13 unchanged.
- [s2] counter-init-before-call (EXTEND counter live range across the call to
  DROP its priority below the ptr, AND put a call boundary before the entry ++):
  sandbox 15. greg: both 74,75 now conflict 6,7 (equal ranges) but order still
  75 74, dispositions still 74->s1/75->s0; fold persists. Equalizing live length
  does NOT flip the ref-count tiebreak. KILLED.
- [s2] do-while0-entry-increment (BB boundary to block cse2 0+1 fold): sandbox 13
  unchanged; do-while(0) collapses; fold+swap persist. KILLED.
- [s2] KEY: target loads the ptr `la $s0` BEFORE func_80079A30 too => target's
  ptr live-range is IDENTICAL to ours yet target allocates ptr->s0 and does NOT
  fold. => both diffs are cc1-internal divergences (global.c allocno-order
  tiebreak + cse2 const-prop) under the MANDATORY target structure, not
  pure-C-structural. Structural axis EXHAUSTED (s1+s2).

## [s3] KILLED (measured, structural)
- return-value-split (drop counter n_refs below ptr via separate `result` local
  for store+return): sandbox 13, greg identical (75 74, ptr->s1/counter->s0),
  `result` copy-propagated away. ROOT: allocno_compare uses floor_log2(n_refs);
  floor_log2(4)==floor_log2(6)==2, so ref-count NEVER discriminates these two —
  the ledger's "counter ~6 vs ptr ~4 ref-count tiebreak" note is mechanically
  inert; the real discriminator is live_length (s1/s2 proved unmovable). KILLED.
- u32-counter type narrowing: sandbox 13/33 insns unchanged; width-invariant RA +
  type-independent cse2 fold. KILLED.
- => structural axis now closed in PRINCIPLE (floor_log2 coarsening + width
  invariance), not merely by trial. Both remaining diffs are cc1-internal.

## Live frontier (untried, mechanism-grounded)
1. [s2 KILLED — structural levers exhausted] Frontier items 1 (raise ptr
   ref-weight / equalize live range) and 2 (structural fold-disruption via BB
   boundary) are both measured dead. Ref-count tiebreak (counter ~6 vs ptr ~4)
   and cse2 fold both persist under every faithful statement rearrangement,
   because byte-match forces target's exact do-while+entry-++ structure. Target
   proves identical live-ranges yet opposite allocation => cc1-internal.
2. decomp-permuter campaign from candidate.c with a clean single-function
   target.o (per difficult-is-not-impossible §3): the sanctioned modality for a
   simultaneous tied register-rename + fold-disrupting structural mutation. Not
   yet run for code6cac_c. Bank the base score, run fresh-seed windows.
   THIS IS THE ONLY REMAINING SANCTIONED AXIS not yet measured dead.
3. cc1-vs-cc1psx calibration check on BOTH the allocno-order tiebreak (swap) and
   the 0+1 cse2 fold: if cc1psx diverges from our decompals port on the SAME C,
   this is an escalation candidate (compiler divergence, not a worker close),
   pending after permuter comes back negative.

## [s1] Moving the pointer init (var_s0=&D_80102810) below the func_80079A30 call trims the pointer pseudo's live range, raising its global.c priority above the counter so it grabs s0 and resolves the s0<->s1 swap.
- mechanism: global.c allocno priority ~ n_refs*freq/live_length; pointer(74) has a long live range because its symbol-address load conflicts with a2/a3 (live across the func_80079A30 arg setup). Shortening it should raise priority.
- probe: Edit src to move the pointer init after the call; sandbox --disable all; re-dump pre.i.greg and check allocation order.
- result: sandbox 16 (WORSE than 13). greg IDENTICAL: order still 75 74, pointer 74 still ->s1(17) and still conflicts 6,7. GCC hoists the loop-invariant la D_80102810 to the function top regardless of C statement position, so the live range/a2-a3 conflict is not reorder-controllable; reposition only worsened scheduling.
- verdict: KILLED

## [s2] Moving `var_s1 = 0;` above the func_80079A30 call extends the counter pseudo's live range across the call to drop its priority below the pointer (fixing the s0<->s1 swap) AND places a call boundary before the entry ++ that blocks cse2 from const-propagating 0+1 (fixing fold diff #13).
- mechanism: global.c allocno priority ~ n_refs*freq/live_length; equalizing the counter's live length to the pointer's should remove the pointer's live-range disadvantage. A call insn between the 0-init and the ++ was expected to break cse2's extended-basic-block const propagation.
- probe: Edit src (var_s1=0 before call); sandbox --disable all; re-dump pre.i.greg (allocation order/dispositions) and out.s (entry increment).
- result: sandbox 15 (WORSE). greg: both pseudo 74(ptr) and 75(counter) now conflict a2/a3 (equal live ranges) but order STILL '75 74', dispositions STILL 74->s1(17)/75->s0(16). Fold persists (li $16,1) across the call boundary; +2 sched damage from a redundant a0 recompute. Equalizing live length does not flip the ref-count tiebreak (counter ~6 refs vs ptr ~4).
- verdict: KILLED

## [s2] Wrapping the entry increment in a sanctioned do{var_s1++;}while(0) introduces a basic-block/loop-note boundary that cse2 will not const-propagate the dominating var_s1=0 across, so the entry ++ emits addiu (reading 0) like the target instead of our folded li 1.
- mechanism: cse2 folds 0+1->1 because var_s1=0 dominates var_s1++ in the same EBB; a real BB boundary would stop the propagation. do-while(0) is a sanctioned codegen-effect technique.
- probe: Edit src (do{var_s1++;}while(0)); sandbox --disable all; re-dump out.s entry increment + greg dispositions.
- result: sandbox 13 UNCHANGED; the do-while(0) COLLAPSED (documented behavior) - out.s entry still li $16,1 (fold persists), dispositions still 74->s1/75->s0 (swap persists). No boundary survived.
- verdict: KILLED

## [s2] The s0/s1 swap and the li/addiu fold are pure-C-structural (reorderable), not compiler-internal divergences.
- mechanism: If the diffs were structural, some faithful statement arrangement would flip the allocation or block the fold.
- probe: Read target asm (asm/funcs/func_80037A20.s) and compare its pointer live-range/init positions and fold behavior to ours under multiple structural rearrangements.
- result: DISPROVEN. Target loads the pointer `la $s0` at the TOP, BEFORE the func_80079A30 jal - IDENTICAL live range to ours - yet target allocates pointer->s0 and emits `addu $s1,$0,$0`+`addiu $s1,$s1,1` (no fold). Given identical C live-ranges/refs our GCC port allocates the opposite way and folds 0+1. Byte-match forces target's exact do-while+entry-++ +post-- structure, so no faithful reshape can change it. Both diffs are cc1-internal (global.c allocno-order tiebreak + cse2 const-prop).
- verdict: CONFIRMED

## [s3] Splitting the counter's store+return refs onto a separate `result` local drops its n_refs (~6 -> ~4) below the pointer's (~4), so the pointer sorts first in global.c allocno order and grabs s0, fixing the s0<->s1 swap.
- mechanism: global.c allocno_compare priority is a function of n_refs; lowering the counter's ref count below the pointer's should reverse the allocation-order tiebreak.
- probe: Added `result = var_s1; D_800A38C8 = result; return result;`; sandbox --disable all; greg dump (tmp/grind/func_80037A20/s3/dump_probeA/pre.i.greg).
- result: sandbox 13 UNCHANGED. greg IDENTICAL: order still '75 74', 74/ptr -> s1(17), 75/counter -> s0(16); `result` copy-propagated away (no new pseudo). ROOT: allocno_compare uses floor_log2(n_refs), and floor_log2(4)==floor_log2(6)==2, so the counter-vs-ptr ref-count gap CANNOT discriminate the two allocnos at all — n_refs is mechanically inert here; live_length is the sole discriminator (proved unmovable in s1/s2).
- verdict: KILLED

## [s3] Narrowing the counter's type (u32) alters allocation or disrupts the cse2 0+1 entry-increment fold.
- mechanism: type narrowing is a listed structural lever; a different width could change RA or the const-prop fold.
- probe: Changed `s32 var_s1` -> `u32 var_s1`; sandbox --disable all.
- result: sandbox 13, 33 insns UNCHANGED. Register width identical (one word reg) so allocation unchanged; the 0+1 fold is a type-independent cse2 const-prop. Inert for both diffs.
- verdict: KILLED

## [s4] A decomp-permuter campaign from the pin-free candidate can find a simultaneous tied register-rename (fix the s0<->s1 swap) + fold-disrupting mutation (fix the li s0,1 vs addiu s1,s1,1 diff) that no single hand structural edit achieves.
- mechanism: difficult-is-not-impossible §3 documented modality: the mutation space (random + refer-to-var + reorder passes) is larger than hand structural edits, so a basin the hand search can't reach could yield the tied-rename + fold-disrupt combo.
- probe: Built a faithful offset-0 single-function workspace (33=33 insns, base_score 298) + a structurally-different while-loop chassis (34 insns, base_score 793). Ran two --stop-on-zero campaigns: chassis A 7887 iters, chassis B 3852 iters (~11,700 combined). Harvested + stopped both in-turn.
- result: Chassis A lowest weighted score 98; chassis B lowest 363. Zero (byte match) NEVER approached. Every find was a junk mutation (new_var pointer alias; loop-internal increment reorder) that shaves the weighted diff but is semantically broken / cheat-form. The 13-diff itself (s0<->s1 swap + cse2 0+1 fold) was NEVER perturbed by any mutation.
- verdict: KILLED

## [s5] A THIRD structurally-distinct permuter chassis (for(;;)/break AST, distinct from goto[A]/while[B]) reaches a different attractor basin that flips the s0<->s1 swap or disrupts the cse2 0+1 fold where chassis A/B could not.
- mechanism: fresh-seed discipline — a distinct CFG/AST chassis can seed a different mutation trajectory; if the permuter axis were merely under-sampled rather than mechanically closed, a new chassis might reach the tied-rename + fold-disrupt combo.
- probe: Built faithful offset-0 chassis C (33=33 insns, base_score 298, same 13-diff signature). Launched fresh-seed --stop-on-zero campaign (label s5C_for_break), waited in-turn ~9546 iters, harvest --stop.
- result: KILLED. Lowest weighted 193 (WORSE than chassis A's 98); zero never approached; novel-find gaps widened (basin quiet). All finds re-find the SAME 98/193/203/278/288/293/298 attractor classes; best (output-193-1) is a dead-store junk mutation (`var_s1=1; var_s1=0;`) that never touches the swap or fold. A 3rd distinct chassis lands in the identical basin => the permuter cannot reach the byte-match; both diffs are cc1-internal as the s1-s3 greg diagnosis predicted.
- verdict: KILLED

## Axis status after s5
Structural (s1-s3) AND permuter (s4-s5, 3 chassis / ~21k iters) axes are ALL
measured dead with mechanism. The only remaining move is the frontier's cc1psx
calibration self-disproof (compile base_single.c via tools/cc1psx_wrapper.sh;
diff s0/s1 allocation + entry-increment fold vs our fork vs target). Per
no-compiler-divergence.md + difficult-is-not-impossible §3 (cc1psx 0/282 wins),
this is EXPECTED to DISPROVE divergence (cc1psx also swaps/folds) and re-confirm
the C is the variable. That is a forensics/escalation modality, NOT permuter.
Only if cc1psx unexpectedly matches target where our fork does not is this an
owner escalation (compiler divergence). No escalation claim is warranted until
that self-disproof runs.

## [s5] A third structurally-distinct permuter chassis (for(;;)/break AST, distinct from goto[A]/while[B]) reaches a different attractor basin that flips the s0<->s1 swap or disrupts the cse2 0+1 fold where chassis A/B could not.
- mechanism: Fresh-seed discipline: a distinct CFG/AST chassis seeds a different mutation trajectory; if the permuter axis were merely under-sampled rather than mechanically closed, a new chassis might reach the tied-rename + fold-disrupt combo the s4 goto/while chassis missed.
- probe: Built faithful offset-0 chassis C (33=33 insns, base_score 298, same 13-diff signature: ptr->s1/counter->s0 swap + entry li s0,1 fold vs target addiu s1,s1,1). Launched fresh-seed --stop-on-zero campaign (label s5C_for_break), waited in-turn ~9546 iters across 6 windows, harvest --stop.
- result: Lowest weighted score 193 (WORSE than chassis A's 98); zero never approached; novel-find gaps widened 15s->15s->120s->60s->60s (basin quiet). All 6 novel finds (193/298/293/288/298/203) re-find the SAME 98/193/203/278/288/293/298 attractor classes s4 characterized; best (output-193-1) is a dead-store junk mutation (var_s1=1; var_s1=0;) that shaves scheduling weight but never touches the swap or the fold. A 3rd distinct chassis lands in the identical basin.
- verdict: KILLED

## [s6] cc1psx (original PsyQ compiler) diverges from our decompals fork on candidate.c: if cc1psx allocates ptr->s0 + emits addiu (no fold) it is compiler divergence (owner escalation), else the C is the variable.
- mechanism: no-compiler-divergence.md + difficult-is-not-impossible §3 record cc1psx at 0/282 wins over our fork; the documented self-disproof before any compiler-divergence escalation.
- probe: forensics — ran tools/cc1psx_wrapper.sh on the s4 cpp.i; diffed cc1psx func region vs our fork cc1 output vs target on the s0/s1 allocation + entry increment.
- result: DISPROVEN. cc1psx output is BYTE-IDENTICAL to our fork (only $L vs .L label cosmetic differs): cc1psx ALSO swaps (ptr->s1/counter->s0) AND folds (li $16,1). cc1psx does NOT match target. Compiler is deterministic+identical; the matching pure-C provably exists as a DIFFERENT source. Compiler-divergence escalation FORECLOSED.
- verdict: KILLED

## [s6] The fold is a cse2 (post-loop) transform; a live-range dataflow lever can lengthen the counter past the pointer to demote it and flip the swap without scheduling damage (live-frontier #2).
- mechanism: prior ledger attributed the fold to cse2 and posited the counter loses s0 because it is SHORT-lived; a dataflow lever lengthening its range should raise the pointer's relative priority.
- probe: fresh cc1 -da dumps on the s4 cpp.i; traced the entry increment across jump/cse/cse2/combine, and read the greg allocno conflicts + RTL live ranges for pseudo 74(ptr)/75(counter).
- result: BOTH premises refuted. (a) The fold appears already in base.i.cse (FIRST cse pass) with a REG_WAS_0 29 note — it is the FIRST cse, not cse2. (b) The counter allocno is live insn 29..75 while the pointer is live 13..51: the counter is ALREADY the LONGER-lived allocno yet STILL out-ranks the pointer (allocated first: header "75 74"). So lengthening the counter's live range is the WRONG direction and cannot demote it; its priority win is the n_refs*freq product, which the byte-forced loop structure fixes. n_refs already inert (s3 floor_log2). No mechanism-plausible pure-C lever remains.
- verdict: KILLED

## Axis status after s6
Compiler-divergence axis now MEASURED DEAD (cc1psx == fork, byte-identical),
alongside structural (s1-s3) and permuter (s4-s5). The live-frontier dataflow
lever (#2) is refuted at the mechanism level by the greg live-range reading (the
counter is already longer-lived and still wins). Exact-pass attribution: SWAP =
global.c allocno-order priority (counter allocno sorts first, takes s0); FOLD =
cse.c FIRST pass REG_WAS_0 const-prop of the dominating s1=0. Both are
cc1-internal under the mandatory 33-insn target structure and identical between
cc1psx and our fork. No sanctioned pure-C axis with a plausible mechanism
remains open. Next disposition is owner escalation (endgame-lock-disposition:
RA+cse-fold internal lock a couple insns short under a byte-forced structure) —
to be filed by an escalation-modality session, NOT this forensics session.

## [s6] cc1psx (original PsyQ GCC 2.7.2.SN.1) diverges from our decompals fork on candidate.c — allocating ptr->s0 and emitting addiu (no fold) like target — making the two diffs a fork-vs-cc1psx compiler divergence (owner escalation).
- mechanism: no-compiler-divergence.md + difficult-is-not-impossible §3 record cc1psx at 0/282 wins; documented self-disproof before any compiler-divergence escalation.
- probe: Ran tools/cc1psx_wrapper.sh on the exact s4 cpp.i; diffed cc1psx func region vs our fork cc1 -da output vs target on the s0/s1 allocation + entry increment.
- result: DISPROVEN. cc1psx output BYTE-IDENTICAL to our fork (only $L vs .L label prefix cosmetic): cc1psx ALSO allocates ptr->s1/counter->s0 (swap) AND folds the entry ++ to li $16,1 (fold). cc1psx does NOT match target. Compiler is deterministic+identical; the matching pure-C provably exists as a DIFFERENT source. Compiler-divergence owner-escalation FORECLOSED.
- verdict: KILLED

## [s6] The fold is a cse2 (post-loop) transform and the counter loses s0 because it is SHORT-lived, so a dataflow lever lengthening the counter's live range past the pointer's would demote it and flip the swap without scheduling damage (live-frontier #2).
- mechanism: Prior ledger attributed the fold to cse2 and the swap to a live_length tiebreak where the counter's shorter range wins; a range-lengthening dataflow shape should raise the pointer's relative priority.
- probe: Fresh cc1 -da dumps on the s4 cpp.i; traced the entry increment across jump/cse/cse2/combine and read the greg allocno conflicts + RTL live ranges of pseudo 74(ptr)/75(counter).
- result: BOTH premises refuted. (a) The fold is already present in base.i.cse (FIRST cse pass) as (set reg (const_int 1)) with a REG_WAS_0 29 note — it is the FIRST cse, not cse2. (b) From the RTL the counter allocno is live insn 29..75 while the pointer is live 13..51: the counter is ALREADY the LONGER-lived allocno yet STILL out-ranks the pointer (greg header 'allocate: 75 74'). Lengthening the counter's range is the WRONG direction and cannot demote it; its priority win is the n_refs*freq product, fixed by the byte-forced loop structure. n_refs already inert (s3 floor_log2). No mechanism-plausible pure-C lever remains.
- verdict: KILLED

## [s7] FORENSICS — first-hand ALLOCDBG corrects the swap mechanism; fold<->swap coupling
- claim: instrumented cc1 (BB2_ALLOC_DEBUG) gives the exact global.c allocno
  priorities and settles whether n_refs / live_length is the swap driver and whether
  defeating the cse fold demotes the counter.
- probe: ran tmp/gccdbg/cc1 -O2 with BB2_ALLOC_DEBUG on the pin-free body; then a
  forensic-only opt-barrier variant (fold blocked) to test fold<->swap coupling.
- result: counter(75) pri=17142 (nrefs=8, livelen=14) beats pointer(74) pri=5882
  (nrefs=5, livelen=17) by ~2.9x. CORRECTS s3 (real refs 8 vs 5, NOT 4 vs 6 -> n_refs
  IS a discriminator) and s6 (counter livelen 14 < pointer 17 -> counter is SHORTER-
  lived, not longer). Barrier probe: blocking the fold RAISES counter pri to 22000,
  swap does NOT flip -> unfolding adds a counter ref, entrenching s0. The two diffs
  are coupled in the OPPOSITE direction from the original frontier lever.
- verdict: CONFIRMED (swap is a decisive, fold-robust priority win; endgame-lock
  strengthened). The quantified escape (counter allocno refs <=4 at fixed bytes) is
  the sole remaining structural target and is what a future escalation session weighs.

## Axis status after s7
Mechanism now correctly quantified from instrumented data (not hand-counted RTL).
Compiler-divergence (s6), structural (s1-s3), permuter (s4-s5) axes dead; s7 shows
the swap is robust even to fold-defeat, and quantifies the only theoretical escape
(reduce counter global.c allocno refs 8->4 at fixed 33-insn bytes — s3's return-
value-split already copy-propagated away). Disposition unchanged: RA+cse-fold
endgame-lock, owner escalation is the correct next disposition — to be FILED by an
escalation-modality session. This forensics session names/quantifies; does not self-file.

## [s7] Instrumented cc1 (BB2_ALLOC_DEBUG) will give the exact global.c allocno priorities and settle whether n_refs or live_length drives the s0<->s1 swap, testing the s3 'n_refs inert' and s6 'counter longer-lived' claims.
- mechanism: global.c allocno_compare pri = floor_log2(nrefs)*nrefs/live_length*10000*size; the higher-pri allocno sorts first and claims the first free callee-saved reg (s0).
- probe: Ran tmp/gccdbg/cc1 -O2 -G0 with BB2_ALLOC_DEBUG=1 on the pin-free faithful cpp.i; read the ALLOCDBG per-allocno print.
- result: counter(pseudo75) nrefs=8 livelen=14 pri=17142 -> s0; pointer(pseudo74) nrefs=5 livelen=17 pri=5882 -> s1. Counter wins by ~2.9x via BOTH more refs (8 vs 5, crossing floor_log2 3 vs 2 -> numerator 24 vs 10) AND shorter live range (14 vs 17). REFUTES s3 (actual refs 8/5, not 4/6, so n_refs IS a discriminator) and s6 (counter is SHORTER-lived, 14<17, not longer).
- verdict: CONFIRMED

## [s7] Defeating the cse REG_WAS_0 fold of the entry increment will lengthen the counter's live range and demote it below the pointer, flipping s0 to the pointer and resolving both diffs at once (the original frontier lever).
- mechanism: If folding shortens the counter's live range (fold makes insn44 write-only), then unfolding should extend it and lower pri = floor_log2(nrefs)*nrefs/live_length.
- probe: Forensic-only opt-barrier __asm__("":"=r"(s1):"0"(s1)) before the entry ++ (a CHEAT, never a candidate) to block the fold; re-ran BB2_ALLOC_DEBUG.
- result: Swap did NOT flip. Counter pri ROSE to 22000 (nrefs 8->11, livelen 14->15); pointer 5555; still 75->s0/74->s1. Unfolding the entry ++ ADDS a read-ref to the counter -> fold and swap are coupled in the OPPOSITE direction: defeating the fold ENTRENCHES the counter's s0 win. Explains why every single-lever attempt s1-s6 failed.
- verdict: KILLED

## [s8] The sanctioned duplicated-statement-into-arms byte-free ref-lift can raise the pointer allocno's global.c refs above the counter's (pri 17142) to flip s0 and drop the floor.
- mechanism: duplicated-statement-into-arms lifts a pseudo's reg_n_refs via byte-neutral duplication into 2+ control-flow arms (cross-jump re-merges to identical bytes), raising its allocno_compare priority.
- probe: Structural evaluation of the function's control-flow surface vs the s7-quantified flip target (pointer needs ~13+ refs; has 5).
- result: Structurally unavailable: the function has only one if-block and one loop, so there is no byte-neutral way to duplicate a pointer statement into the ~8 arms needed to reach 13+ refs. Pointer refs are byte-forced at 5.
- verdict: KILLED

## [s8] A faithful count/pointer decomposition can present <=4 counter allocno refs at the fixed 33 bytes AND the entry ++ at a cse-block boundary, flipping s0 to the pointer.
- mechanism: Lowering the counter's global.c refs below the pointer's 5882 pri, or defeating the REG_WAS_0 fold at a cse boundary, would redistribute allocation without changing emitted bytes.
- probe: Cross-check of the s7 coupling result against the s3 return-value-split kill.
- result: Self-contradicting: unfolding the entry ++ ADDS a counter read-ref (s7 barrier probe raised counter pri 17142->22000, entrenching the swap); s3's return-value-split already tried the ref reduction and `result` copy-propagated away. The two requirements pull against each other.
- verdict: KILLED

## [s9] SOLVER MODALITY — the owner's 2026-08-24 directive, executed. FLOOR 13 -> 8.

The owner's queue-item directive ("solver modality (ra_solver/sched_solver)
recommended before deep re-grind of RA/scheduler-tiebreak residuals") had never
been executed on this function: s1-s8 predate it. Running it did NOT confirm the
endgame lock — it broke it.

## [s9] The ra_solver inverse model says the s0<->s1 swap is reachable only by lowering the COUNTER allocno's global.c refs to <=4 (at live_length 14) or raising the POINTER's to >=10; at target's fixed byte stream neither is spellable, because refs and live_length are positively coupled in this function.
- mechanism: global.c allocno_compare pri = floor_log2(nrefs)*nrefs*size/live_length*10000;
  order is pri DESC (tie -> lower pseudo = birth order) and the FIRST allocno takes
  the first free callee-saved register ($s0). The pointer allocno is byte-forced at
  nrefs=5 / live_length=17 -> pri 5882 (the la is at insn 5 of target's stream and the
  last use at insn 20, with no dead gap), so the only freedom is on the counter side.
- probe: tools/ra_solver/extract.py + inverse.py global --goal '{"74":16,"75":17}'
  (tmp/grind/func_80037A20/s9/inverse_global.txt), then an enumeration of every legal
  contiguous partition of the counter's nine byte-forced references across multiple C
  locals (tmp/grind/func_80037A20/s9/partition_foreclosure.py), then a MEASURED
  instance of the most promising partition.
- result: the enumeration predicted every partition leaves a counter allocno above the
  pointer, minimum max-pri 12500 vs the pointer's 5882. MEASURED CONFIRMATION: the
  "n = var_s1 + 1" split (rejected/split-counter-partition-p2.c) is byte-neutral
  (13 / 33 insns) and the split SURVIVES to global alloc, but ALLOCDBG gives
  p79(n) nrefs=4 len=7 pri=11428 and p75(count) nrefs=4 len=8 pri=10000 — both still
  above the pointer's 5882, exactly as predicted (the model called 4/7/11428 to the
  digit). Splitting the COUNTER is mechanically dead: every cut that lowers a pseudo's
  ref count shortens its live range by the same span, so the quotient never falls.
- verdict: CONFIRMED (counter-side levers foreclosed, with a measured instance)

## [s9] KEY — splitting the POINTER (base + walking pointer) creates a short, reference-dense loop-carried allocno that out-ranks the counter, allocating the pointer family FIRST and reproducing target's $s0/$s1 assignment.
- mechanism: the same allocno_compare quotient read in the other direction. The
  counter's priority cannot be lowered, but the POINTER's can be RAISED — not by
  adding references (s8 correctly killed that: the function has one if and one loop,
  so there is nowhere byte-neutral to duplicate a pointer statement) but by
  SHORTENING the live range of the allocno that carries them. A base pointer that
  dies at the loop preheader plus a walking pointer born there splits the pointer's
  17-insn range into 12 + 6 while the loop-carried half keeps the dense references.
  The two halves do not conflict (the base dies at the copy), so they share $s0 and
  the copy is a no-op move deleted by final.c — the insn count is unchanged.
- probe: `p = var_s0;` in the loop preheader, loop advances p; measured across five
  statement-order/loop-shape spellings (tmp/grind/func_80037A20/s9/sweep.ps1,
  sweep2.ps1) with ALLOCDBG read for each.
- result: with the goto loop the split alone gives p pri 13333 vs counter 15000 —
  pointer sorts SECOND, sandbox 16. With a do/while loop the loop-carried pointer
  gains references and tightens: p75 nrefs=7 len=6 pri=23333 -> $s0; counter p76
  nrefs=10 len=16 pri=18750 -> $s1; base p74 nrefs=3 len=12 pri=2500 -> $s0.
  That is TARGET's assignment exactly. **sandbox --disable all = 8** (was 13),
  33/33 insns; objdump confirms $s0 = pointer, $s1 = counter throughout.
  Four spellings of the family (C_dowhile, W1 copy-outside-if, W3 copy-before-inc,
  W4 firstfile-uses-p) all land on 8; adding the counter split on top regresses to
  18/35 (the split counter stops conflicting with the walking pointer, so it re-takes
  $s0 and the base pointer is pushed to $s2, costing a third save/restore pair).
- verdict: CONFIRMED — the s1-s8 "cc1-internal RA lock, no pure-C lever remains"
  conclusion is REFUTED. The lock was a search boundary, not a mechanism.

## Axis status after s9 — the endgame-lock claim is WITHDRAWN
The 2026-07-24 OWNER-ESCALATION and the 2026-07-27 refusal ruling rest on
"every sanctioned pure-C axis is measured dead". That premise is now false: the
solver modality (never run before s9) produced an ordinary-C form that drops the
honest floor 13 -> 8 with no coercion construct of any kind. func_80037A20 is
plainly grindable and must stay ACTIVE.

Remaining 8 diffs, both PRE-RA (register allocation is SOLVED):
1. `la D_80102810` placement — target emits the lui/addiu pair in the ENTRY basic
   block, before the sprintf jal; ours emits it in the block after the jal, which
   re-schedules the whole prologue (the sw $s0 / sw $s1 / sw $ra distribution and
   which save lands in the jal delay slot). With one pointer local (s1..s8) the la
   WAS at the top — the loop use kept it there — so the split traded the la
   position for the allocation. Both are needed. NOT yet pass-attributed: read
   tmp/grind/func_80037A20/dumps/*.cse / *.loop / *.combine before hypothesising.
2. the entry increment — ours `li $s1,1`, target `addiu $s1,$s1,1`; already
   attributed in s6 to the FIRST cse.c pass (REG_WAS_0 const-prop of the
   dominating `var_s1 = 0`). s7 showed defeating it ADDS a counter reference; under
   the NEW allocation that is harmless (the counter is no longer competing for
   $s0), so the s7 coupling objection no longer applies and this lever is worth
   re-opening.

## [s9] The s0<->s1 residual is reachable by perturbing the COUNTER allocno's global.c inputs (ra_solver inverse: counter refs 8->4 at live_length 14, or pointer refs 5->10).
- mechanism: global.c allocno_compare pri = floor_log2(nrefs)*nrefs*size/live_length*10000; order is pri DESC (tie -> lower pseudo number = birth order) and the first allocno takes the first free callee-saved register ($s0).
- probe: tools/ra_solver/extract.py + inverse.py global --goal {74:16, 75:17}; then an enumeration of every legal contiguous partition of the counter's byte-forced references across multiple C locals (partition_foreclosure.py); then a MEASURED instance of the most promising partition (n = var_s1 + 1).
- result: Foreclosed on the counter side. The split is byte-neutral (13 / 33 insns) and DOES survive to global alloc, but ALLOCDBG gives n(p79) nrefs=4 len=7 pri=11428 and count(p75) nrefs=4 len=8 pri=10000 - both still above the pointer's byte-forced 5882, exactly as the enumeration predicted (it called 4/7/11428 to the digit). nrefs and live_length are positively coupled here, so every cut lowers both and the quotient is unmoved.
- verdict: CONFIRMED

## [s9] Splitting the POINTER into a base pointer (passed to firstfile) and a walking pointer the loop advances creates a short, reference-dense loop-carried allocno that out-ranks the counter, allocating the pointer family first and reproducing target's $s0/$s1 assignment.
- mechanism: The same allocno_compare quotient read in the other direction: the counter's priority cannot be lowered, but the pointer's can be RAISED by shortening the live range of the allocno that carries its references - splitting the pointer's 17-insn range into 12 + 6 and leaving the dense references on the short half. The base dies at the copy so it does not conflict with the walking pointer; both take $s0 and the copy is a no-op move deleted by final.c, so the insn count stays 33.
- probe: p = var_s0 in the loop preheader with the loop advancing p, measured across five statement-order / loop-shape spellings (tmp/grind/func_80037A20/s9/sweep.ps1, sweep2.ps1) with ALLOCDBG read for each, plus objdump of the emitted object.
- result: With a do/while loop: p(p75) nrefs=7 len=6 pri=23333 -> $s0; counter(p76) nrefs=10 len=16 pri=18750 -> $s1; base(p74) nrefs=3 len=12 pri=2500 -> $s0 - target's assignment exactly. sandbox --disable all = 8 (was 13), 33/33 insns, 0 rules, no cheat construct; objdump confirms $s0 = pointer, $s1 = counter throughout. Four spellings of the family (C_dowhile, W1 copy-outside-if, W3 copy-before-inc, W4 firstfile-uses-p) all measure 8; the goto-loop spelling measures 16 (p pri 13333 < counter 15000).
- verdict: CONFIRMED

## [s9] Stacking the counter split on top of the pointer split lowers the floor further.
- mechanism: Both splits independently reduce allocno priorities, so combining them should further separate the pointer family from the counter family.
- probe: W2_counter_split (pointer split + n = var_s1 + 1 + do/while); sandbox + ALLOCDBG.
- result: REGRESSES to 18 / 35 insns. The split counter is dead during the loop, so it no longer CONFLICTS with the walking pointer, re-takes $s0, and the base pointer is pushed to $s2 - costing a third callee-saved save/restore pair. The counter must stay UNSPLIT precisely so that it conflicts with the walking pointer and is forced to $s1.
- verdict: KILLED

## [s9] func_80037A20 is an endgame lock with every sanctioned pure-C axis measured dead (the premise of the 2026-07-24 OWNER-ESCALATION and the 2026-07-27 refusal ruling).
- mechanism: s1-s8 measured the structural, permuter (3 chassis, ~21k iters), compiler-divergence and ALLOCDBG-forensics axes dead at a flat floor of 13.
- probe: Executed the solver modality that the owner's 2026-08-24 queue directive recommended for exactly this class of residual and that no prior session had run.
- result: DISPROVEN. An ordinary-C form with no coercion construct of any kind drops the honest floor to 8. The four dead modalities were all attacking the same side of a two-sided quotient; the solver named the quotient and made the other side visible in one session. Escalation withdrawn in docs/grind/decisions.md (2026-08-26 entry); the function stays ACTIVE and grindable.
- verdict: KILLED

## [s10] REDERIVE MODALITY — FLOOR 8 -> 6. The s9 pointer split is REPLACED by an ordinary single-pointer body.

Re-derivation from the target asm (not a tweak of the s9 body) plus first-hand
pass attribution from `tmp/grind/func_80037A20/dumps/` and a read of
`tools/gcc-2.7.2/sched.c`. Two mechanisms were named for the first time; the
s9 candidate's structural regression was explained and undone.

## [s10] The `la D_80102810` sinks past the sprintf jal in the s9 split body because sched.c's adjust_priority()/birthing_insn_p() boosts any insn whose destination pseudo has reg_n_sets == 1, and sched1 schedules BACKWARD so the boost emits the insn LATE.
- mechanism: `tools/gcc-2.7.2/sched.c` adjust_priority(): with zero REG_DEAD notes
  (and the file's own comment says those notes are always gone by then, so the
  n_deaths==0 arm always runs) it calls birthing_insn_p(), which returns
  `reg_n_sets[REGNO(dest)] == 1`. If true the insn's INSN_PRIORITY is raised to
  max_priority. schedule_block() runs backward, so a high priority means "picked
  early in the backward walk" = "emitted late" — the pass's register-lifetime
  shortening heuristic. The s9 split gives the BASE pointer exactly one set, so
  the la is boosted and lands adjacent to its only consumer, after the jal.
- probe: pwsh tools/grinder/dump.ps1 func_80037A20 on both bodies; read the
  `;; Function func_80037A20` block-0 ready lists in code6cac_c.sched.
  SPLIT body: `ready list at T-4: 32 (1) 29 (1) 13 (7f000001), now 13 32 29`
  ONE-POINTER body: `ready list at T-4: 32 (1) 29 (1) 13 (1), now 32 29 13`
  and insn 13 then drifts to T-11 (emitted first).
  The la is still ahead of the call in .cse, .loop, .combine and .flow — the move
  happens in sched1 and nowhere else (the .lreg chain shows 18,20,22,24,call26,
  29,32,13,34,36,40).
- result: CONFIRMED, and it is a hard gate: every attempt to give the base pointer
  a second set byte-free failed because cse1 deletes any set whose value is a
  constant/constant-equivalent (rejected/s10-fmtstring-var-reuse-la-still-sinks.c,
  s10-sp10-buffer-var-reuse-la-still-sinks.c). A single walking pointer is set
  twice by construction (la + `+= 0x28`), so the ordinary body never triggers the
  boost. THE s9 POINTER SPLIT IS THEREFORE STRUCTURALLY SELF-DEFEATING: it buys
  the allocation and pays for it with target's prologue schedule.
- verdict: CONFIRMED

## [s10] The s0<->s1 allocation is reachable in the ORDINARY one-pointer body — it is an allocno_compare TIE broken by pseudo number, and hoisting `var_s1 = 0;` above the sprintf call produces the tie.
- mechanism: global.c pri = floor_log2(nrefs)*nrefs*size/live_length*10000, order
  pri DESC, tie -> lower pseudo number, and the first allocno takes $s0. reg_n_refs
  is LOOP-DEPTH WEIGHTED (flow.c adds loop_depth per reference), so a loop-body
  reference counts twice — this is why the ledger's earlier hand counts were low.
  Moving the zero-init above the call simultaneously LENGTHENS the counter's live
  range and SHORTENS the pointer's, and the two quotients meet.
- probe: tools/ra_solver/extract.py on each body (models in
  tmp/grind/func_80037A20/s10/model_onept.json, model_P1.json, model_Q.json),
  plus objdump of the sandbox object.
- result: MEASURED
    do/while, init AFTER the call : ptr(74) 8 refs/17 len = 14117
                                    counter(75) 10/14 = 21428  -> counter first,
                                    counter takes $s0. sandbox 13.
    do/while, init BEFORE the call: ptr(74) 8/16 = 15000
                                    counter(75) 10/20 = 15000  ** EXACT TIE **
                                    -> pseudo 74 (declared first) allocated first,
                                    takes $s0; counter forced to $s1 = TARGET.
                                    **sandbox --disable all = 6**, 33/33 insns,
                                    0 rules, no coercion construct.
  The tie is delicate and order-sensitive: initialising the POINTER before the
  counter instead (rejected/s10-zero-init-before-pointer-init-flips-tie.c) breaks
  it the other way (sandbox 15), and the goto loop chassis loses it too
  (rejected/s10-goto-chassis-on-hoisted-init.c, 14). The DECLARATION order
  `s32 *var_s0;` before `s32 var_s1;` is load-bearing — it is the tie-break.
- verdict: CONFIRMED

## [s10] s9's "counter-side levers are foreclosed" result is CHASSIS-STALE and its numbers do not carry.
- mechanism: s9 computed the foreclosure against a pointer priority of 5882 (the
  s1..s8 goto chassis). On the do/while chassis the pointer is at 14117, so the
  bar the counter family must clear is less than half as high.
- probe: re-measured s9's own p2 partition (`n = var_s1 + 1`) on the do/while
  one-pointer chassis with ALLOCDBG.
- result: still loses, but for a newly-named reason: the loop-carried counter
  pseudo cannot fall below 6 refs at live_length 7 (1 set + 2 loop refs counted
  twice by loop-depth weighting + 1 tail read) = pri 17142, and 17142 > 14117.
  It would need live_length >= 9, which the byte stream does not provide.
  Counter-side splitting stays dead, but the CORRECT reason is the loop-depth
  ref weighting, not the s9 "refs and live_length are positively coupled" claim.
- verdict: CONFIRMED (dead, re-derived reason)

## [s10] The last 6 diffs are one sched1 decision plus the known cse1 fold.
- mechanism: `move s1,zero` has NO consumer inside the entry basic block (its
  consumer is the entry increment in the next block), so in the backward pass it
  is ready from the first cycle and rank_for_schedule falls through to
  `INSN_LUID (tmp) - INSN_LUID (tmp2)` — a DESCENDING LUID sort, so the largest
  LUID is picked first in the backward walk and emitted LAST. Hoisting the
  statement gave the insn a low LUID, so it is emitted FIRST (index 4); target
  emits it LAST in the block (index 12, right after the jal). The three register
  saves and the second `addiu a0,sp,0x10` redistribute around it, which also
  changes which insn reorg puts in the sprintf jal delay slot.
- probe: side-by-side of the emitted block against asm/funcs/func_80037A20.s
  (recorded in candidate.c's header) + the sched/sched2 ready-list dumps.
- result: OPEN. The requirement is contradictory on its face — the insn needs a
  LOW luid (early source position) for the live range that wins the allocation
  tie, and a HIGH luid (late source position) for target's emission slot. Two
  outs exist and neither is measured: (a) give the insn the birthing boost so
  sched1 sinks it regardless of luid — it needs reg_n_sets == 1 on its
  destination, and the naive spelling of that (a separate single-set local copied
  into the counter) costs the tie
  (rejected/s10-zero-init-split-single-set-pseudo.c, 13); (b) find a different
  byte-free way to lengthen the counter's live range / shorten the pointer's that
  does not depend on the zero-init's source position at all.
- verdict: OPEN — this is the whole remaining frontier.

## [s10] The `la D_80102810` sinks past the sprintf jal in the s9 split body because of a named sched1 heuristic, not because of cse/combine/LICM.
- mechanism: tools/gcc-2.7.2/sched.c adjust_priority() raises an insn to max_priority (0x7f000001) when birthing_insn_p() is true, i.e. when reg_n_sets[REGNO(dest)] == 1. schedule_block() runs BACKWARD, so the boost emits the insn LATE (the register-lifetime shortening heuristic). The s9 split gives the base pointer exactly one set; a walking pointer has two (the la and the += 0x28).
- probe: pwsh tools/grinder/dump.ps1 func_80037A20 on both bodies; read the block-0 ready lists in tmp/grind/func_80037A20/dumps/code6cac_c.sched and the insn chain in .cse/.loop/.combine/.flow/.lreg.
- result: The la is still ahead of the call in .cse, .loop, .combine and .flow, and only moves by .lreg (post-sched1). sched1 dumps: split body `T-4: 32 (1) 29 (1) 13 (7f000001), now 13 32 29`; one-pointer body `T-4: 32 (1) 29 (1) 13 (1), now 32 29 13` with insn 13 then drifting to T-11 (emitted first).
- verdict: CONFIRMED

## [s10] The base pointer can be given a second set byte-free so that birthing_insn_p stops boosting its la, keeping the s9 allocation AND target's schedule.
- mechanism: reg_n_sets >= 2 disables the boost; a first assignment whose value is consumed before the la would supply the second set at no byte cost.
- probe: Two spellings measured: the pointer local first holding the format-string address, and first holding the stack buffer address (tmp/grind/func_80037A20/s10/v_A_fmtreuse.c, v_D_bufreuse.c), plus a walking-pointer-declared-at-top variant (v_K_ptop.c).
- result: Both address-reuse spellings stay at sandbox 8 with the la still after the jal: cse1 deletes any set whose source is a constant or constant-equivalent (symbol_ref, frame-pointer + offset), so reg_n_sets falls back to 1. The declared-at-top variant collapses to the one-pointer body (13). Byte-free second sets of an address pseudo are not spellable.
- verdict: KILLED

## [s10] Target's $s0/$s1 allocation is reachable in the ORDINARY one-pointer body by moving `var_s1 = 0;` above the sprintf call, which makes the two allocnos tie on global.c priority so the tie-break by pseudo number hands $s0 to the pointer.
- mechanism: global.c pri = floor_log2(nrefs)*nrefs*size/live_length*10000, ordered pri DESC with ties broken by lower pseudo number, and the first allocno takes $s0. reg_n_refs is LOOP-DEPTH WEIGHTED (flow.c adds loop_depth per reference, so loop-body references count twice). Hoisting the zero-init lengthens the counter's live range and shortens the pointer's at the same time, and the two quotients meet.
- probe: tools/ra_solver/extract.py on each body (model_onept.json, model_P1.json, model_Q.json) plus sandbox --disable all and objdump of the sandbox object.
- result: init AFTER the call: ptr(74) 8 refs/17 len = 14117 -> $s1, counter(75) 10/14 = 21428 -> $s0, sandbox 13. init BEFORE the call: ptr(74) 8/16 = 15000 and counter(75) 10/20 = 15000 - an EXACT TIE - so pseudo 74 (var_s0, declared first) is allocated first and takes $s0, counter forced to $s1. That is target's assignment; sandbox --disable all = 6, 33/33 insns, 0 rules, objdump confirms.
- verdict: CONFIRMED

## [s10] s9's counter-side foreclosure (every counter partition leaves a counter allocno above the pointer) still holds on the new do/while chassis.
- mechanism: s9 computed the bar against a pointer priority of 5882 from the s1-s8 goto chassis; the do/while chassis puts the pointer at 14117, less than half as high a bar, so the conclusion had to be re-measured rather than inherited.
- probe: Re-measured s9's own partition (`n = var_s1 + 1`) on the do/while one-pointer chassis with ALLOCDBG (model_P1.json), plus a tail-split variant (v_P6.c) and an intermediate-copy variant (v_P5.c).
- result: Still dead, but for a newly-named reason: loop-body references are loop-depth weighted, so the loop-carried counter pseudo floors at 6 refs / live_length 7 = 17142 (1 set + 2 loop refs counted twice + 1 tail read), and 17142 > 14117. It would need live_length >= 9, which the fixed byte stream does not provide. The tail-split variant costs an insn (34, sandbox 17) because jump2 does not cross-jump the duplicated gp store + return move.
- verdict: CONFIRMED

## [s10] With the allocation solved, the entry-increment fold and the residual entry-block schedule can be closed by re-trying the s2/s6 fold defeats on the new chassis (s9 held that every prior kill was chassis-stale).
- mechanism: s6 pinned the fold to the FIRST cse.c pass (REG_WAS_0 note on the dominating zero-init); s9 argued the s7 coupling objection no longer applied under the new allocation.
- probe: do-while(0) wrap of the entry increment, the `var_s1 = var_s1 + 1` spelling, and a single-set zero-init local, all measured on the s10 chassis.
- result: All fail. do-while(0) measures 14 (it does not defeat the cse1 REG_WAS_0 fold AND it costs the allocation tie); `var_s1 = var_s1 + 1` is byte-identical to `var_s1++` at 6; the single-set zero-init local measures 13 (the extra pseudo perturbs both live lengths and loses the tie). On this chassis the fold is not independently attackable - it is coupled to the same live-range balance that wins the allocation.
- verdict: KILLED

## [s11] The disposition of $s0/$s1 is decided ENTIRELY by global.c allocno order (no hard-reg preference path exists on this function), so target's register assignment is a closed-form inequality on ref counts and live lengths.
- mechanism: `tools/gcc-2.7.2/config/mips/mips.h` defines NO `REG_ALLOC_ORDER`, so
  global.c `find_reg` walks hard regs 0..N and takes the first available in the
  allocno's class; the first allocno in the priority sort therefore takes $16 and
  the second takes $17. The preference machinery (`hard_reg_preferences`,
  `hard_reg_copy_preferences`, `hard_reg_full_preferences`, `regs_someone_prefers`)
  is INERT here: `prune_preferences` clears every preference because the only
  hard-reg copies either allocno participates in are to CALL-CLOBBERED regs
  ($v0 for the counter, $a0/$a1 for the pointer) and both allocnos cross calls.
- probe: `tools/ra_solver/extract.py func_80037A20 code6cac_c` on four bodies;
  read `prefs` / `copy_prefs` / `full_prefs` / `hard_conflicts` out of the model
  JSONs (tmp/grind/func_80037A20/s11/model_{base,v7,v9}.json).
- result: prefs == {} and copy_prefs == {"74": [], "75": []} on EVERY body measured.
  hard_conflicts for both allocnos are only {2,4,5,(6,7),29} — never 16 or 17.
  There is therefore no pure-C lever that can hand $s1 to an allocno that sorts
  first: the ONLY lever is the priority sort itself,
  `pri = floor_log2(nrefs)*nrefs*size/live_length*10000`.
- verdict: CONFIRMED

## [s11] KILLED — no assignment of source statement positions produces BOTH target's `la $s0` position AND target's `move $s1,$zero` position, because the two requirements pull the allocation inequality in opposite directions.
- mechanism: block 0 is emitted in RTL order (see the sched1 hypothesis below), so
  the `la` lands before the sprintf jal iff the pointer-init statement is before the
  sprintf call, and `move $s1,$zero` lands after the jal iff the counter-init
  statement is after the sprintf call. But the pointer's live length Lp is measured
  from the `la` and the counter's Lc from its zero-init, so "la early" maximises Lp
  and "zero-init late" minimises Lc. The pointer only wins the allocation when
  floor_log2(8)*8/Lp >= floor_log2(10)*10/Lc, i.e. **Lc >= 1.25 * Lp**.
  Rp = 8 and Rc = 10 are STRUCTURALLY FIXED by target's own 33-insn stream
  (loop-depth-weighted: pointer = la 1 + loop `addiu` 2*2 + loop `move a0` 1*2 +
  `move a1` 1 = 8; counter = init 1 + peel 1 + loop `addiu` 2*2 + `addiu -1` 2 +
  `sw` 1 + `move v0` 1 = 10), so the inequality cannot be re-balanced by refs.
- probe: the full 2x2 statement-position matrix, each measured with
  `sandbox --disable all` AND `tools/ra_solver/extract.py` (ALLOCDBG ground truth):
    ptr BEFORE call, zero BEFORE call (s10 candidate / base):
        ptr(74) 8 refs / livelen 16 = 15000 ; counter(75) 10 / 20 = 15000
        EXACT TIE -> pseudo 74 first -> $s0 = pointer (TARGET's allocation)
        la BEFORE jal (target) ; `move s1,zero` FIRST in block (NOT target)
        **sandbox 6**
    ptr AFTER  call, zero BEFORE call (s11/v1_ptr_after_call.c):
        **sandbox 7** ; la sinks past the jal (NOT target)
    ptr AFTER  call, zero AFTER  call (s11/v7_both_after_call.c):
        ptr 8/11 = 21818 ; counter 10/15 = 20000 -> ptr wins STRICTLY -> $s0 = ptr
        `move s1,zero` lands immediately AFTER the jal (TARGET's slot!) and
        `sw $s1,0x34($sp)` lands in the jal delay slot (TARGET) — but the `la`
        sinks past the jal (NOT target).  **sandbox 8**
    ptr BEFORE call, zero AFTER  call (= target's apparent statement order,
        s11/v9_ptrbefore_zeroafter.c):
        ptr 8/17 = 14117 ; counter 10/14 = 21428 -> COUNTER sorts first and takes
        $s0.  Allocation is target-INVERTED.  **sandbox 13**
  Target needs Lp≈17 and Lc≈14 simultaneously, i.e. Lc/Lp = 0.82, against a
  requirement of >= 1.25.  Foreclosed by a factor of 1.5.
- verdict: KILLED (the s10 frontier item "sweep loop-shape/tail-shape variants with
  the zero-init AFTER the call" is closed: the obstruction is not the loop shape,
  it is the ref/live-length inequality, and no loop or tail shape changes Rp/Rc.)

## [s11] sched1 does NOT reorder func_80037A20's entry block at all — every insn ties at INSN_PRIORITY 1 and rank_for_schedule's descending-INSN_LUID tie-break reproduces the RTL order exactly. The s10 "birthing boost sinks the zero-init" frontier is therefore misframed.
- mechanism: `tools/gcc-2.7.2/sched.c` rank_for_schedule sorts descending
  INSN_PRIORITY, then by class relative to last_scheduled_insn, then descending
  INSN_LUID; schedule_block walks BACKWARD, so a descending-LUID ready list is
  emitted in ascending-LUID (= original) order. adjust_priority()'s birthing boost
  (`birthing_insn_p` -> `reg_n_sets[dest] == 1`) can only raise an insn to
  max_priority = 0x7f000001, which is still BELOW the block-terminating jump's
  0x7fffffae, so the very best a boosted insn can do is be emitted second-to-last
  in the block — one slot before the jal, never after it.
- probe: `pwsh tools/grinder/dump.ps1 func_80037A20`, then the
  `;; Function func_80037A20` segment of tmp/grind/func_80037A20/dumps/code6cac_c.sched.
- result: block 0 spans insns 4..40 and contains BOTH calls (29 = sprintf,
  36 = firstfile) — calls do NOT split scheduling blocks here.
  All of 13,16,21,23,25,27,32,34,36 have `priority = 1`; only insn 40 (the block's
  jump) has 0x7fffffae and insns 25/27 carry the birthing boost 0x7f000001.
  The emitted sequence is T-11..T-1 = 13,16,21,23,25,27,29,32,34,36,40 — exactly
  ascending insn number.  Identification of the insns:
    4  reg72 = $a0      6  reg73 = $a1     13 reg75 = 0 (the counter zero-init)
    16 reg74 = &D_80102810 (the `la`)      21 $a0 = sp+16   23 $a1 = fmt
    25 $a2 = reg72 (deleted: reg72 -> $a2) 27 $a3 = reg73 (deleted)
    29 call sprintf     32 $a0 = sp+16     34 $a1 = reg74   36 call firstfile
    40 beqz
  Additionally reg_n_sets[counter] == 4 (init, peel, loop +1, tail -1), so the
  boost can never apply to insn 13 in any case.
- verdict: CONFIRMED (kills s10 frontier item 1 as stated)

## [s11] The callee-saved register SAVES are NOT RTL insns — mips.c emits `sw $sN,off($sp)` as text immediately before the first insn that defines/uses $sN — so target's `sw $s1,0x34($sp)` in the sprintf delay slot is a CONSEQUENCE of `move $s1,$zero` sitting after the jal, not an independent scheduling fact.
- mechanism: the sched1 dump's block-0 insn list contains no store-to-stack insns,
  yet raw cc1 output (tmp/grind/func_80037A20/s11/tu.s) interleaves
  `sw $17,52($sp)` / `sw $16,48($sp)` / `sw $31,56($sp)` between scheduled insns —
  each one immediately preceding the first appearance of its register.
- probe: raw cc1 .s (tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -mcpu=3000
  -mips1 -mno-abicalls -mel) vs the .sched insn inventory above.
- result: base body prints `sw $17,52` right before insn 13 (`move $17,$0`),
  `sw $16,48` right before insn 16 (`la $16`), `sw $31,56` right before the jal.
  Target prints `sw $16,48` before its `la $16` (idx 3/4) and `sw $17,52` in the
  jal delay slot immediately before its `move $17,$0` (idx 11/12).  Same rule,
  different `move $17,$0` position.  Corollary: chasing the `sw $s1` position
  independently is wasted work — it is fully determined.
- verdict: CONFIRMED

## [s11] Defeating the `li $s1,1` -> `addiu $s1,$s1,1` fold is QUANTITATIVELY self-defeating on the base chassis: unfolding raises the counter's weighted ref count 10 -> 11, which raises the allocation bar from Lc >= 1.25*Lp to Lc >= 1.375*Lp, and the base body sits exactly at 1.25.
- mechanism: target's `addiu $s1,$s1,1` mentions $s1 twice where our folded
  `li $s1,1` mentions it once, so Rc becomes 11 and floor_log2(11)*11 = 33.
  On the base body (Lp = 16, Lc = 20) that gives counter 33/20 = 16500 against
  pointer 24/16 = 15000 — the counter would sort FIRST and take $s0.
- probe: arithmetic on the measured ALLOCDBG numbers for the base body
  (tmp/grind/func_80037A20/s11/model_base.json: 74 -> 8/16/15000,
  75 -> 10/20/15000), applying the same
  pri = floor_log2(nrefs)*nrefs*size/live_length*10000 formula the tool reproduces
  exactly on all four measured bodies.
- result: the fold and the allocation are not two levers, they are ONE constraint.
  Any body that shows target's `addiu $s1,$s1,1` must ALSO satisfy
  Lc >= 1.375*Lp, which no measured statement ordering reaches (best is 1.25).
  s10 recorded this coupling qualitatively; this is the number.
- verdict: CONFIRMED (this is why every s10 fold-defeat attempt cost the tie)

## [s11] Structural re-derivations of the loop/exit shape all cost insns; the entry-block residual is not a loop-shape problem.
- mechanism: rederive modality — four structurally distinct bodies, all measured
  with sandbox --disable all against the 33-insn target.
- probe: tmp/grind/func_80037A20/s11/{v3_branch_target_peel,v4_while_break,
  v8_early_exit_zero,v15_no_temp,v16_u8buf}.c
- result: branch-target peel with duplicated tail (`goto found;` so the peeled
  increment is the branch TARGET rather than the fall-through — an attempt to put
  the increment outside cse1's extended-basic-block path and so defeat the fold):
  **34 insns, sandbox 17** — jump2 does not cross-jump the duplicated
  `sw`+`move v0`+return.  `while (1) { ...; if (!nextfile) break; ... }`:
  **34 insns, sandbox 10**.  Early-exit `if (!firstfile) { D = 0; return 0; }`:
  **34 insns, sandbox 17**.  Inlining the nextfile result into the do/while
  condition instead of a named temp: **35 insns, sandbox 9**.  Declaring the
  sprintf buffer as `u8 sp10[32]` instead of `s32 sp10[8]`: **33 insns, sandbox 6**
  — byte-identical to base, buffer type is inert.
- verdict: KILLED (all four)

## [s11] OPEN — the whole 6-diff residual is reproduced exactly if reorg leaves the sprintf jal's delay slot EMPTY on the base body; reorg currently fills it with insn 21 (`addiu $a0,$sp,0x10`), the nearest length-4 eligible insn.
- mechanism: on the base body the pre-reorg block-0 stream is
  13 (`move $17,$0`), 16 (`la $16`, length 8), 21 (`addiu $4,$sp,16`, length 4),
  23 (`la $5`, length 8), 29 (call).  reorg's fill_simple_delay_slots scans
  backward from the call; both `la`s are two-instruction macros and are ineligible
  for a delay slot, so the first eligible candidate is insn 21, which reorg moves
  down into the slot.  If instead NO candidate were eligible, the slot would be
  left to the assembler, and — by the save-emission rule confirmed above —
  mips.c's lazy `sw $17,52($sp)` (printed immediately before the first def of $17)
  plus `move $17,$0` would follow the jal, with the assembler pulling the `sw` into
  the slot.  The resulting entry block is
  `sw $16 / la $16 / addiu $a0,$sp,16 / la $a1 / sw $ra / jal / sw $s1 (delay) /
   move $s1,$zero / addiu $a0,$sp,16 / jal / move $a1,$s0` — bit-for-bit target's,
  with the 15000/15000 allocation tie completely untouched (the RTL ORDER does not
  change, so Lp and Lc do not change).  This is the only reading found so far that
  reconciles target's schedule with target's allocation; the statement-position
  route is proven impossible above.
- probe (for the next session): (a) confirm the delay-slot choice first-hand in
  tmp/grind/func_80037A20/dumps/code6cac_c.dbr (the reorg dump was generated this
  session but not read) — identify which insn reorg picks and WHY insn 21 is
  eligible; (b) read `fill_simple_delay_slots` in tools/gcc-2.7.2/reorg.c to
  enumerate the byte-free conditions under which insn 21 becomes ineligible or is
  not reached by the backward scan (resource conflict with an intervening insn,
  `eligible_for_delay` length test, basic-block boundary, or the scan's step
  limit); (c) then look for a pure-C spelling that satisfies one of those
  conditions WITHOUT changing the RTL order of insns 13/16 — candidates: change
  which insn is nearest-before-the-call by altering the sprintf ARGUMENT
  expressions (target emits $a0 before $a1, same as ours, so the lever is what
  sits between $a1's `la` and the call), or make the $a0 setup itself a two-insn
  sequence.  Note the constraint: any spelling that ADDS an insn is out (33/33
  already).
- verdict: OPEN — this is the whole remaining frontier.

## [s11] The $s0/$s1 disposition in func_80037A20 is decided entirely by global.c allocno sort order, with no hard-register preference path available to pure C.
- mechanism: GCC 2.7.2's tools/gcc-2.7.2/config/mips/mips.h defines no REG_ALLOC_ORDER, so global.c find_reg walks hard regs 0..N and the first allocno in the priority sort takes $16 ($s0) while the second takes $17 ($s1). The preference machinery (hard_reg_preferences / hard_reg_copy_preferences / hard_reg_full_preferences / regs_someone_prefers) is inert because prune_preferences clears every preference: the only hard-reg copies either allocno takes part in are to call-clobbered registers ($v0 for the counter, $a0/$a1 for the pointer) and both allocnos cross calls.
- probe: tools/ra_solver/extract.py func_80037A20 code6cac_c on four distinct bodies; read prefs / copy_prefs / full_prefs / hard_conflicts out of tmp/grind/func_80037A20/s11/model_{base,v7,v9}.json.
- result: prefs == {} and copy_prefs == {"74": [], "75": []} on every body measured; hard_conflicts for both allocnos are only {2,4,5,(6,7),29}, never 16 or 17. The only lever on the disposition is the priority formula pri = floor_log2(nrefs)*nrefs*size/live_length*10000.
- verdict: CONFIRMED

## [s11] No assignment of source statement positions produces BOTH target's `la $s0` position (before the sprintf jal) and target's `move $s1,$zero` position (after the sprintf jal), because the two requirements pull the allocation inequality in opposite directions.
- mechanism: Entry-block emission order equals RTL order equals source statement order (sched1 does no reordering here, separately confirmed). So the `la` precedes the jal iff the pointer-init statement precedes the sprintf call, and `move $s1,$zero` follows the jal iff the counter-init statement follows the sprintf call. The pointer's live length Lp starts at the `la` and the counter's Lc starts at its zero-init, so `la early' maximises Lp while `zero-init late' minimises Lc. The pointer wins the $s0 seat only when floor_log2(8)*8/Lp >= floor_log2(10)*10/Lc, i.e. Lc >= 1.25*Lp. Rp = 8 and Rc = 10 are structurally fixed by target's own 33-instruction stream under flow.c's loop-depth ref weighting, so the inequality cannot be re-balanced from the ref side.
- probe: Full 2x2 statement-position matrix, each cell measured with `sandbox func_80037A20 --disable all` AND tools/ra_solver/extract.py ALLOCDBG ground truth.
- result: ptr before / zero before (the s10 candidate): ptr(74) 8 refs/livelen 16 = 15000, counter(75) 10/20 = 15000, EXACT TIE -> pseudo 74 first -> $s0 = pointer, sandbox 6. ptr after / zero before: sandbox 7. ptr after / zero after: ptr 8/11 = 21818, counter 10/15 = 20000, ptr wins strictly, `move s1,zero` lands in TARGET's slot right after the jal with `sw $s1,0x34($sp)` in the delay slot, but the `la` sinks past the jal, sandbox 8. ptr before / zero after (= target's apparent statement order): ptr 8/17 = 14117, counter 10/14 = 21428, COUNTER sorts first and takes $s0 (target-inverted), sandbox 13. Target's own layout demands Lp ~ 17 with Lc ~ 14, ratio 0.82, against a requirement of 1.25 - foreclosed by a factor of 1.5.
- verdict: KILLED

## [s11] sched1 sinks or could sink `move $s1,$zero` to the end of the entry block via adjust_priority's birthing boost (the s10 frontier item 1).
- mechanism: sched.c rank_for_schedule sorts descending INSN_PRIORITY, then by dependence class relative to last_scheduled_insn, then descending INSN_LUID; schedule_block walks backward, so a descending-LUID ready list is emitted in ascending-LUID order. adjust_priority raises an insn to max_priority = 0x7f000001 when birthing_insn_p (reg_n_sets[dest] == 1) holds.
- probe: pwsh tools/grinder/dump.ps1 func_80037A20, then the `;; Function func_80037A20' segment of tmp/grind/func_80037A20/dumps/code6cac_c.sched, with insn identities cross-read from the RTL in the same dump.
- result: Block 0 spans insns 4..40 and contains BOTH calls (29 = sprintf, 36 = firstfile) - calls do not split scheduling blocks here. Every insn ties at priority 1 except insn 40 (the terminating jump, 0x7fffffae) and insns 25/27 (argument moves carrying the birthing boost 0x7f000001, both deleted by coalescing). The emitted sequence T-11..T-1 is 13,16,21,23,25,27,29,32,34,36,40 - exactly ascending insn number, i.e. sched1 performs NO reordering in this function. The hypothesis is dead twice over: reg_n_sets[counter] == 4 (init, peel, loop +1, tail -1) so the boost can never apply to insn 13, and even a boosted insn caps at 0x7f000001 < the jump's 0x7fffffae, so it could at best reach the second-to-last slot of the block, never past the jal.
- verdict: KILLED

## [s11] The callee-saved register saves (`sw $s0/$s1/$ra`) are schedulable RTL insns whose position can be attacked independently.
- mechanism: If they were RTL insns they would appear in the sched1 block inventory and could be moved by sched1/reorg.
- probe: Compared the sched1 block-0 insn inventory (which contains no store-to-stack insns) against raw cc1 output for the same body (tmp/grind/func_80037A20/s11/tu.s, produced with tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel).
- result: The saves are TEXT emitted by mips.c immediately before the first insn that touches the corresponding register: `sw $17,52($sp)` precedes `move $17,$0`, `sw $16,48($sp)` precedes `la $16`, `sw $31,56($sp)` precedes the jal. Target obeys the identical rule (its `sw $16,48` precedes its `la $16`; its `sw $17,52` sits in the jal delay slot immediately before its `move $17,$0`). The `sw $s1` position is therefore a dependent variable of the `move $s1,$zero` position and must never be chased on its own.
- verdict: KILLED

## [s11] The `li $s1,1` vs target `addiu $s1,$s1,1` fold can be attacked independently of the allocation on the s10 chassis.
- mechanism: Target's `addiu $s1,$s1,1` mentions $s1 twice where our folded `li $s1,1` mentions it once, so unfolding raises the counter's loop-depth-weighted ref count from 10 to 11 and floor_log2(11)*11 = 33, which raises the allocation bar from Lc >= 1.25*Lp to Lc >= 1.375*Lp.
- probe: Arithmetic on the measured ALLOCDBG numbers for the s10/base body (model_base.json: pseudo 74 -> 8 refs / livelen 16 / pri 15000; pseudo 75 -> 10 / 20 / 15000), using the same priority formula that the tool reproduces exactly on all four bodies measured this session.
- result: On the base body (Lp = 16, Lc = 20, ratio exactly 1.25) an unfolded peel would give the counter 33/20 = 16500 against the pointer's 24/16 = 15000, so the counter would sort first and take $s0 - the tie is lost. The fold and the allocation are ONE constraint, not two levers; this is the quantitative reason every s10 fold-defeat attempt cost the tie.
- verdict: CONFIRMED

## [s11] A structurally different loop/exit shape (rederive modality) reaches target's entry block or defeats the cse1 fold.
- mechanism: Four distinct rewrites of the counting loop and the early-exit path, including one specifically designed to put the peeled increment on the branch-TARGET side rather than the fall-through side so it would fall outside cse1's extended-basic-block path and escape the constant fold.
- probe: sandbox func_80037A20 --disable all on tmp/grind/func_80037A20/s11/{v3_branch_target_peel,v4_while_break,v8_early_exit_zero,v15_no_temp,v16_u8buf}.c
- result: branch-target peel with duplicated tail: 34 insns, sandbox 17 (jump2 does not cross-jump the duplicated gp store + return move, matching the s10 tail-split finding). while(1){...;break}: 34 insns, sandbox 10. early-exit `if (!firstfile) { D_800A38C8 = 0; return 0; }`: 34 insns, sandbox 17. nextfile inlined into the do/while condition instead of a named temp: 35 insns, sandbox 9. `u8 sp10[32]` instead of `s32 sp10[8]`: 33 insns, sandbox 6 - byte-identical, buffer C type is inert. The entry-block residual is not a loop-shape problem.
- verdict: KILLED
