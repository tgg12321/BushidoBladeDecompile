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
