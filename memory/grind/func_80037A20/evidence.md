# Evidence bank — func_80037A20

- s1 (recon) GREG-CONFIRMED mechanism (artifact tmp/grind/func_80037A20/s1/dump/pre.i.greg).
  2 regs to allocate; allocation order "75 74". pseudo 74 (POINTER, `la D_80102810`)
  -> s1(17); pseudo 75 (COUNTER) -> s0(16). Target wants pointer->s0, counter->s1
  (SWAPPED). Order 75-then-74 => counter has HIGHER global.c priority
  (n_refs*freq/live_length) and grabs s0 first. pseudo 74 conflicts list INCLUDES
  6,7 (a2/a3): the pointer's symbol-address load is hoisted to the function top by
  GCC (loop-invariant), so it is live across the func_80079A30 arg setup -> long
  live range -> lower priority. Counter is set AFTER the call -> no 6/7 conflict,
  shorter range -> higher priority. BOTH factors favor the counter for s0.

- s1 KILLED: pointer-init-after-call (move `var_s0=&D_80102810;` below the
  func_80079A30 call to trim its live range). sandbox 16 (WORSE). greg IDENTICAL
  (still 75 74, 74 still conflicts 6,7): GCC hoists the address load to the top
  regardless of C statement position, so the pointer live range / a2-a3 conflict
  are NOT reorder-controllable. Reposition only worsened scheduling (+3). This is
  the [[register-alloc-pure-c]] "confirmed limit" priority-wall class.
  rejected/pointer-init-after-call-live-range-trim.c

- s1 m2c reference shape (tmp/grind/func_80037A20/s1/m2c_target.c): m2c splits the
  counter into two SSA vars — `var_s1=0; if(firstfile){ var_s1_2=1; do{ptr+=0x28;
  var_s1_2+=1;} while(nextfile(ptr)); var_s1=var_s1_2-1;}`. Confirms the faithful
  count++/count-- do-while structure (candidate.c). NOTE m2c renders the entry as
  `var_s1_2 = 1` but the TARGET instruction is `addiu s1,s1,1` (an ADD reading the
  prior 0) — our build folds it to `li s0,1` (diff #13). The fold is a cse2
  (pre-allocation) transform, so it is INDEPENDENT of the final register (not a
  downstream symptom of the swap). Target's cse2 did not const-propagate 0 into
  the entry ++; ours does. Faithful-semantics fold-disruption lever still open.

- s1 near-dup lead obj_InitTaskCamera (0.606) is SPURIOUS — it is only an extern
  decl in this TU; a camera-task init, no firstfile/nextfile file-count loop. Dead.

- WIP rejected_form: do-while no-entry-increment (wrong count, off-by-one)

- WIP rejected_form: nextfile-in-if-condition (34 insns, +nop)

- WIP rejected_form: decl reorder var_s1-first / var_s0-first (allocation unchanged)

- WIP rejected_form: firstfile inlined to v0_val temp (13, unchanged)

- == imported from memory/wip notes.md ==
# func_80037A20 — WIP (memcard file-count via firstfile/nextfile)

## TL;DR
Honest pin-free floor = **13** (sandbox --disable all), identical to HEAD's
honest distance. HEAD "matches" ONLY via two `register asm()` pins
(`var_s0 asm("s0")`, `var_s1 asm("s1")`) + a forbidden opt-barrier
`__asm__("" : "=r"(var_s1) : "0"(var_s1))` that the 2026-04-04 commit message
admits was added to "block constant propagation." All three are cheats; this
function is INCOMPLETE. candidate.c is the faithful pin/barrier-free body
(33=33 insns, 13 diffs).

## The 13 diffs decompose into exactly TWO coupled problems
1. **s0<->s1 register swap (~12 of 13 diffs).** Target: var_s0(ptr)->s0,
   var_s1(counter)->s1. Our pin-free build: ptr->s1, counter->s0. The counter
   out-prioritises the pointer for the preferred callee-saved reg s0 because it
   is incremented in the loop AND is the return value (live to function end),
   while the pointer's address-load lives from the top too. Tied loop-weighted
   refs -> global.c allocno-priority tiebreaker. Decl reorder (var_s1 first /
   var_s0 first) does NOT flip it. This is the [[register-alloc-pure-c]]
   "confirmed limit" class (global.c:624 tiebreaker).
2. **`li s0,1` vs target `addiu s1,s1,1` (1 diff).** The faithful structure
   requires an entry `var_s1++` (counts the firstfile hit = slot 0) SEPARATE
   from the loop's `var_s1++`. Since `var_s1 = 0;` immediately dominates that
   entry `++`, GCC constant-folds `0 + 1` -> `li 1`. The target's real cc1
   emitted `addiu` (did NOT fold). This is the EXACT fold the cheat
   `__asm__("")` barrier was suppressing. No pure-C structural lever found that
   keeps the faithful count semantics AND disrupts the fold (see rejected/).

## rejected_forms (measured, do NOT re-derive)
- do-while w/ s1++ at loop top, no entry-++, no s1-- : WRONG COUNT (drops
  slot-0 count; off by one). v6/v7.
- v3 (nextfile in the if-condition, s1++ before branch): 34 insns (+1 nop),
  fold persists, swap persists.
- decl reorder var_s1-first (v4) / var_s0-first: allocation UNCHANGED.
- inlining firstfile into a v0_val temp (v5): 13, unchanged.

## Avenues for next session (change MODALITY)
- **decomp-permuter** from candidate.c — the documented modality for both a
  tied register rename AND a fold-disrupting structural mutation. Not yet run
  (per-fn permuter setup for code6cac_c is the remaining work).
- **cc1 -da greg dump** to confirm the allocno-priority tiebreaker is the s0/s1
  driver, then a live-range lever to raise the pointer's priority.
- Possible **cc1-vs-cc1psx divergence** on the `li 1` fold — calibration check
  (cc1psx may not fold here). If confirmed-divergent + permuter-negative, this
  is an escalation candidate, not a worker close.

## Floor
- HEAD honest distance: 13 (carries 2 pins + 1 opt-barrier cheat)
- candidate.c honest distance: 13 (zero cheats) — floor NOT lowered below HEAD.


- [s1] canonical: verdict C, pure-C distance 13, 0 rules. sandbox --disable all = 13 (33=33 insns, 34 cheat-asm instances stripped). HEAD 'matches' only via 2 register-asm pins + 1 __asm__ opt-barrier (cheats).

- [s1] greg (pre.i.greg): 2 regs to allocate, order '75 74'. pseudo 74 = POINTER (la D_80102810) -> s1(17); pseudo 75 = COUNTER -> s0(16). Target wants pointer->s0, counter->s1 (SWAPPED). Counter allocated first => higher priority => grabs s0.

- [s1] pseudo 74 conflicts include 6,7 (a2/a3); pseudo 75 does not. Pointer's symbol-address load is hoisted to the top (loop-invariant) so it is live across the func_80079A30 arg setup -> long live range -> lower priority. Counter is set after the call -> shorter range, no 6/7 conflict -> higher priority. Both factors favor counter for s0.

- [s1] Emitted honest asm (out.s): entry increment = 'li $16,1' (folded 0+1) vs target 'addiu $s1,$s1,1' (add reading prior 0). The fold is a cse2 (pre-allocation) transform => INDEPENDENT of the final register, NOT a downstream symptom of the swap.

- [s1] m2c reference (m2c_target.c) confirms the faithful count++/count-- do-while structure (candidate.c); m2c renders the entry as 'var_s1_2=1' but the real target instruction is addiu.

- [s1] Near-dup lead obj_InitTaskCamera (0.606) is SPURIOUS: only an extern decl in this TU, a camera-task init, no firstfile/nextfile loop. Dead lead.

- [s2] TARGET asm read (asm/funcs/func_80037A20.s): the pointer is loaded with
  `lui/addiu $s0, D_80102810` at the function TOP, BEFORE the func_80079A30 jal
  (i.e. target's pointer is ALSO live across that call — IDENTICAL live range to
  ours), yet target assigns pointer->s0, counter->s1. Counter init is
  `addu $s1,$zero,$zero` AFTER the call; entry increment `addiu $s1,$s1,0x1`
  (reads the prior 0, NOT folded). So given IDENTICAL C live-ranges/refs, target
  and our GCC allocate the OPPOSITE way and fold differently. => Both remaining
  diffs are cc1-internal divergences (allocno-order tiebreak in global.c + cse2
  const-prop), NOT reorderable by pure-C statement placement.

- [s2] KILLED (structural) counter-init-before-call: `var_s1=0;` moved above the
  func_80079A30 call. sandbox 15 (WORSE). greg: NOW both pseudo 74(ptr) AND
  75(counter) conflict 6,7 (equal live ranges) but order STILL "75 74",
  dispositions STILL 74->s1(17)/75->s0(16). Equalizing live length does NOT flip
  the tiebreak (counter wins on ref-count ~6 vs ptr ~4). Fold ALSO persists
  (`li $16,1`) across the call boundary + a redundant a0 recompute (+2 sched).
  rejected/counter-init-before-call.c

- [s2] KILLED (structural) do-while0-entry-increment: `do{var_s1++;}while(0)` to
  force a BB/loop-note boundary cse2 won't const-prop 0 across. sandbox 13
  UNCHANGED; the do-while(0) COLLAPSED (out.s entry still `li $16,1`, swap still
  74->s1/75->s0). rejected/do-while0-entry-increment.c

- [s2] STRUCTURAL AXIS EXHAUSTED for both diffs. To byte-match we are FORCED into
  target's exact do-while + entry-`++` + post-`--` structure (any other faithful
  count shape drops/adds instructions vs the 33-insn target). That fixed structure
  deterministically yields, in our GCC port: (a) counter->s0 swap (ref-count
  tiebreak on equal-or-longer-ptr-live-range) and (b) `li 1` fold. Neither is
  movable by statement reorder / init placement / BB-boundary tricks (all
  measured dead across s1+s2). Remaining sanctioned axis NOT yet run:
  decomp-permuter (simultaneous tied-rename + fold-disrupt mutation) — a
  different modality.

- [s2] Baseline candidate.c sandbox --disable all = 13 (33=33 insns, 0 rules); HEAD 'matches' only via 2 register-asm pins + 1 __asm__ opt-barrier (cheats).

- [s2] TARGET (asm/funcs/func_80037A20.s): pointer loaded lui/addiu $s0 BEFORE the func_80079A30 jal (live across the call, same as ours) yet assigned s0; counter addu $s1,$0,$0 after the call; entry increment addiu $s1,$s1,0x1 reads prior 0 (NOT folded).

- [s2] s2 greg (counter-init-before-call variant): both pseudo 74(ptr) and 75(counter) conflict 6,7 (a2/a3) => equal live ranges, but allocation order still '75 74' and dispositions still 74->s1(17)/75->s0(16). Ref-count tiebreak (counter ~6 refs vs ptr ~4) is the swap driver, not live length.

- [s2] Both remaining diffs (~12 swap + 1 fold) persist under every faithful structural rearrangement measured across s1+s2 (pointer-init-after-call, decl reorder, nextfile-in-if, firstfile->temp, no-entry-increment, counter-init-before-call, do-while0-entry-increment). Structural axis EXHAUSTED.

- [s2] The only remaining sanctioned axis not yet measured dead is decomp-permuter (simultaneous tied register-rename + fold-disrupting mutation), a different modality; a cc1-vs-cc1psx calibration check on the tiebreak+fold is the escalation candidate if permuter returns negative.

- [s3] KILLED (structural) return-value-split: separate `result = var_s1;
  D_800A38C8 = result; return result;` to move store+return refs off the counter
  pseudo and drop its n_refs (~6 -> ~4) below the pointer's (~4), hoping the
  pointer sorts first in allocno order. sandbox 13 UNCHANGED. greg IDENTICAL
  (order still "75 74", 74/ptr -> s1(17), 75/counter -> s0(16)); `result`
  copy-propagated away, no new pseudo in dispositions.
  rejected/return-value-split.c artifact tmp/grind/func_80037A20/s3/dump_probeA/.

- [s3] ROOT-MECHANISM finding (why the ref-count tiebreak the ledger named is
  mechanically INERT): GCC 2.7.2 global.c allocno_compare priority uses
  floor_log2(n_refs), NOT raw n_refs. floor_log2(4) == floor_log2(6) == 2, so the
  counter-vs-pointer ref-count gap (~6 vs ~4) CANNOT discriminate the two allocnos
  at all. The real, sole discriminator is live_length (counter shorter -> higher
  priority -> grabs s0), which s1 (pointer-init-after-call) and s2
  (counter-init-before-call) already proved unmovable by statement placement.
  => the n_refs structural sub-axis is closed in principle, not just by trial.

- [s3] KILLED (structural / type-narrowing) u32-counter: change counter type to
  u32. sandbox 13, 33 insns UNCHANGED. Register width is identical (one word reg)
  so allocation unchanged; the 0+1 fold is a type-independent cse2 const-prop.
  Type narrowing is inert for BOTH diffs. rejected/u32-counter-type-narrow.c.

- [s3] STRUCTURAL AXIS DEFINITIVELY EXHAUSTED across s1+s2+s3. The two named
  structural sub-levers (ref-count re-weighting; type narrowing) are now measured
  dead WITH mechanism (floor_log2 coarsening + width-invariant RA), on top of the
  s1/s2 live-range/init-placement/BB-boundary kills. Every faithful structural
  rearrangement leaves the s0<->s1 swap (live_length tiebreak) and the li/addiu
  fold (cse2 const-prop) intact. Remaining sanctioned axis NOT yet measured dead:
  decomp-permuter (different modality) then cc1psx calibration -> escalation.

- [s3] Baseline candidate.c (pin/barrier-free faithful body) sandbox --disable all = 13 (33=33 insns, 0 rules); HEAD 'matches' only via 2 register-asm pins + 1 __asm__ opt-barrier (cheats).

- [s3] s3 return-value-split KILLED: sandbox 13, greg identical (75 74; ptr->s1/counter->s0); `result` copy-propagated away.

- [s3] ROOT MECHANISM (new this session): GCC 2.7.2 global.c allocno_compare priority uses floor_log2(n_refs), not raw n_refs. floor_log2(4)==floor_log2(6)==2, so the counter(~6)-vs-pointer(~4) ref-count difference the ledger flagged as the swap driver is mechanically INERT — it can never flip the allocno order. The sole real discriminator is live_length (counter shorter -> higher priority -> grabs s0), already proved unmovable by s1 (pointer-init-after-call, sandbox 16) and s2 (counter-init-before-call, sandbox 15).

- [s3] s3 u32-counter type-narrowing KILLED: sandbox 13/33 insns unchanged; width-invariant RA + type-independent cse2 fold.

- [s3] STRUCTURAL AXIS DEFINITIVELY EXHAUSTED (s1+s2+s3): both named structural sub-levers (ref-count re-weighting; type narrowing) now dead WITH mechanism, atop the s1/s2 live-range/init-placement/BB-boundary kills. Target proves identical C live-ranges/refs yet opposite s0/s1 allocation + no fold => both remaining diffs are cc1-internal (global.c allocno-order/live_length tiebreak + cse2 const-prop), not pure-C-structural.

- [s4] s4 built a FAITHFUL offset-0 permuter workspace for func_80037A20 (tools/decomp-permuter/nonmatchings/func_80037A20_s4): single-function base.c, target.o at offset 0 (gp=64 dropped), objdump-verified 33=33 insns with the diff EXACTLY = s0<->s1 swap (ptr la->s1/counter->s0 vs target ptr->s0/counter->s1) + entry increment li s0,1 (folded) vs target addiu s1,s1,1. Base_score 298 is the real weighted diff (the callee-save swap cascades into reorderings), NOT offset noise.

- [s4] TOOLING BUG FOUND (why the prior s?-era workspace was worthless): the old multi-function base.c + piped compile.sh emitted a cc1 .file directive with the ABSOLUTE repo path, which contains spaces ('Bushido Blade 2 Decompile'); maspsx splits .file on whitespace expecting 3 tokens -> 'too many values to unpack (expected 3)' -> every candidate failed to compile, so base_score 298 there was garbage and the campaign searched nothing. Fix: single-function base.c + file-based compile.sh + sed neutralizing the .file path to "base.c" before maspsx.

- [s4] Chassis A (goto, 33 insns): 7887 iterations, lowest weighted score 98. Window went quiet (>120s no novel find) after ~6000 iters; all novel finds after the first ~30s were re-finds of the 98/193/203/278/298 attractor classes.

- [s4] Chassis B (while-loop, 34 insns, base 793): 3852 iterations, lowest 363 -- a strictly worse basin that never approached chassis A, let alone 0.

- [s4] The permuter's best form (score 98) moves var_s1++ inside the loop before nextfile -- a double-increment that is semantically WRONG (wrong file count) and still 98 != 0. It is permuter noise, not a closing form; no cheat-vetting required (not a match).

- [s4] The permuter never once flipped the s0/s1 allocation or disrupted the 0+1 fold, empirically confirming the s1-s3 greg diagnosis: both diffs are cc1-internal (global.c live_length allocno tiebreak + cse2 const-prop) and unreachable by pure-C statement mutation.

- [s4] candidate.c unchanged (pin-free faithful body, floor 13). src/code6cac_c.c reverted to HEAD. Structural axis (s1-s3) + permuter axis (s4) are now BOTH measured dead.

- [s5] Built a THIRD structurally-distinct faithful chassis C (for(;;) with in-loop
  `if(!v0){var_s1--; break;}`, distinct AST from goto[A]/while[B]) in
  tools/decomp-permuter/nonmatchings/func_80037A20_s5C. Compiles 33=33 insns,
  offset-0, base_score 298 (IDENTICAL basin depth to chassis A), and the
  insn-level diff is the SAME 13-diff signature (ptr->s1/counter->s0 swap +
  entry `li s0,1` fold vs target `addiu s1,s1,1`). artifact
  tmp/grind/func_80037A20/s5/base_insns.txt vs tgt_insns.txt.

- [s5] KILLED (permuter, fresh seed) chassis C campaign: ~9546 iters, novel finds
  193/298/293/288/298/203, lowest weighted 193 (WORSE than chassis A's 98), zero
  NEVER approached. Novel-find gaps widened 15s->15s->120s->60s->60s (basin went
  quiet). All finds are attractor-class re-finds of the SAME 98/193/203/278/288/
  293/298 classes s4 characterized. Best find output-193-1 is a DEAD-STORE junk
  mutation (`var_s1=1; var_s1=0;`) that shaves scheduling weight but is not a
  match and never touches the s0<->s1 swap or the cse2 0+1 fold. No cheat-vetting
  (not a match). rejected/permuter-chassis-c-for-break.c ; artifact
  tmp/grind/func_80037A20/s5/permuter_summary.txt + campaign telemetry in
  metrics/events.jsonl (permuter-launch/harvest, label s5C_for_break).

- [s5] PERMUTER AXIS DEFINITIVELY EXHAUSTED across s4+s5 (3 structurally-distinct
  chassis: goto, while, for/break; ~21,000 combined iters). A third distinct AST
  lands in the exact same attractor basin and NEVER perturbs the 13-diff,
  empirically confirming the s1-s3 greg diagnosis: both diffs are cc1-internal
  (global.c live_length allocno tiebreak + cse2 const-prop), unreachable by
  pure-C statement mutation. Structural (s1-s3) + permuter (s4-s5) axes are ALL
  now measured dead. Only remaining move is the cc1psx calibration self-disproof
  (forensics/escalation modality, NOT permuter) before any compiler-divergence
  escalation. candidate.c unchanged (floor 13); src reverted to HEAD.

- [s5] s5 built a THIRD structurally-distinct faithful chassis C (for(;;) with in-loop if(!v0){var_s1--;break;}) at tools/decomp-permuter/nonmatchings/func_80037A20_s5C: 33=33 insns, offset-0, base_score 298 (identical basin depth to chassis A), same 13-diff signature (ptr->s1/counter->s0 swap + li s0,1 fold vs addiu s1,s1,1).

- [s5] s5 permuter campaign (label s5C_for_break): ~9546 iters, novel finds 193/298/293/288/298/203, lowest weighted 193, zero NEVER approached, the s0<->s1 swap and cse2 0+1 fold NEVER perturbed. Best find output-193-1 is a dead-store junk mutation (var_s1=1; var_s1=0;), not a match.

- [s5] PERMUTER AXIS DEFINITIVELY EXHAUSTED across s4+s5: 3 structurally-distinct chassis (goto, while, for/break), ~21,000 combined iters, all landing in the same attractor basin, none touching the 13-diff. Confirms the s1-s3 greg diagnosis that both diffs are cc1-internal (global.c live_length allocno tiebreak + cse2 const-prop), unreachable by pure-C statement mutation.

- [s5] Structural (s1-s3) + permuter (s4-s5) axes are ALL now measured dead with mechanism. candidate.c unchanged (pin-free faithful body, floor 13); src/code6cac_c.c untouched (HEAD-clean).

- [s5] Campaign was harvested and --stopped in-turn (no orphan process); telemetry recorded to metrics/events.jsonl (permuter-launch/harvest, label s5C_for_break).

- [s6] SELF-DISPROOF MEASURED (forensics): ran original PsyQ cc1psx.exe
  (GCC 2.7.2.SN.1) on the exact s4 cpp.i (pin-free faithful body) and diffed
  its func region vs our decompals fork's cc1 output. BYTE-IDENTICAL except the
  assembler label prefix ($L vs .L — a cosmetic, not codegen). cc1psx ALSO
  allocates ptr->s1/counter->s0 (the swap) AND folds the entry ++ to `li $16,1`
  (the fold). => cc1psx does NOT match target on candidate.c; the
  fork-vs-cc1psx divergence hypothesis (live-frontier #1) is DISPROVEN. Compiler
  is deterministic+identical; the C is the variable (the matching C provably
  exists — target itself was cc1psx-built). Compiler-divergence owner-escalation
  FORECLOSED. artifacts s6/cc1psx.s, s6/cc1psx.err, s6/dump/base.s.

- [s6] FOLD PASS PINNED (forensics, -da dumps): the entry increment is a live
  `(plus reg 1)` addsi3 in base.i.jump (PRE-cse) and is already `(set reg
  (const_int 1))` with an `(insn_list:REG_WAS_0 29 ...)` note in base.i.cse
  (FIRST cse pass) — stays folded thereafter. PASS = cse.c FIRST CSE pass (NOT
  cse2 as the prior ledger stated); DECISION = REG_WAS_0 const-prop of the
  dominating `s1=0` (insn 29) into `s1+1` -> const 1. Any faithful re-spelling
  of this count keeps `s1=0` dominating -> cse always folds.

- [s6] SWAP DECISION from fresh greg: header `2 regs to allocate: 75 74` =>
  counter(pseudo 75) allocated FIRST, grabs s0(hard 16); pointer(pseudo 74)
  second, gets s1(hard 17). 74 conflicts 6,7(a2/a3) (hoisted symbol_ref load
  insn 13 live across the call); 75 does not. PASS = global.c allocno-order
  priority. NEW refinement that KILLS live-frontier #2's premise: from the RTL
  the COUNTER allocno is live insn 29..75 while the POINTER is live 13..51 — the
  counter is ALREADY the LONGER-lived allocno and STILL out-ranks the pointer.
  So the frontier idea "lengthen the counter's live range to demote it" is
  aimed the WRONG way; the counter's priority win is the n_refs*freq product
  (loop-structure-fixed, byte-count-forced), not a short-live-range effect.
  n_refs already inert (s3 floor_log2). No mechanism-plausible pure-C lever
  remains open.

- [s6] AXIS SUMMARY after s6: compiler-divergence axis now MEASURED DEAD (not
  merely expected); structural (s1-s3) + permuter (s4-s5) already dead; the
  live-frontier dataflow lever (#2) refuted at mechanism level by the greg
  live-range reading. candidate.c unchanged (floor 13); src/code6cac_c.c
  HEAD-clean (no edits this session). Function is an RA+cse-fold internal lock a
  couple insns short under the byte-forced structure => endgame-lock-disposition
  / owner-escalation territory for a future escalation-modality session, not a
  worker close.

- [s6] cc1psx.exe (original PsyQ compiler) on the pin-free candidate cpp.i is BYTE-IDENTICAL to our decompals fork's cc1 output except the $L/.L assembler label prefix — both swap (ptr->s1/counter->s0) and both fold the entry increment to li $16,1. The compiler is not the variable; the matching pure-C source is a DIFFERENT (still-unknown) faithful form.

- [s6] FOLD pass PINNED: base.i.jump (pre-cse) has the entry increment as a live (plus reg 1) addsi3; base.i.cse (FIRST cse pass) already has it as (set reg (const_int 1)) with an (insn_list:REG_WAS_0 29) note. Pass = cse.c FIRST CSE (ledger's 'cse2' was imprecise); decision = REG_WAS_0 const-prop of the dominating s1=0 (insn 29) into s1+1.

- [s6] SWAP decision from fresh greg: header '2 regs to allocate: 75 74' => counter(pseudo 75) sorts first in global.c allocno_order and claims s0(hard 16); pointer(pseudo 74) gets s1(hard 17). 74 conflicts a2/a3 (6,7) because its symbol_ref load (insn 13) is hoisted above the func_80079A30 call; 75 does not.

- [s6] NEW greg refinement: the counter allocno is live insn 29..75 while the pointer is live 13..51 — the counter is ALREADY the LONGER-lived allocno and STILL out-prioritizes the pointer. The frontier's 'lengthen counter live range to demote it' lever is aimed the wrong way; the win is the n_refs*freq product, not a short-range effect.

- [s6] Axis status after s6: compiler-divergence axis now MEASURED DEAD (not merely expected); structural (s1-s3) + permuter (s4-s5, 3 chassis / ~21k iters) already dead; live-frontier dataflow lever refuted at mechanism level. candidate.c unchanged (floor 13); src/code6cac_c.c HEAD-clean (no edits this session).

- [s7] FIRST-HAND ALLOCDBG (instrumented tmp/gccdbg/cc1, BB2_ALLOC_DEBUG=1) on the
  pin-free faithful body — exact global.c allocno_compare priorities:
  counter(pseudo75) nrefs=8 livelen=14 pri=17142 -> s0(16);
  pointer(pseudo74) nrefs=5 livelen=17 pri=5882  -> s1(17).
  The counter wins s0 by a DECISIVE ~2.9x margin, driven by BOTH more refs
  (8 vs 5, crossing the floor_log2 step: 3 vs 2 => numerator 24 vs 10) AND a
  SHORTER live range (14 vs 17). pri = floor_log2(nrefs)*nrefs/livelen*10000*size.

- [s7] CORRECTS s3: the "n_refs INERT (floor_log2(4)==floor_log2(6)==2)" claim used
  ref counts that do not exist in the RTL. Actual refs are 8 (counter) and 5
  (pointer); floor_log2(8)=3 != floor_log2(5)=2, so n_refs IS a live discriminator.

- [s7] CORRECTS s6: the "counter is ALREADY the LONGER-lived allocno (29..75 vs ptr
  13..51)" claim is contradicted by instrumented livelen — counter=14 < pointer=17.
  The counter is SHORTER-lived. s6's hand-counted cuid live-range reading was wrong;
  its conclusion "lengthen-to-demote is the wrong direction" does not follow.

- [s7] cse FOLD mechanism pinned to the basic-block level (independent -da regen):
  entry ++ (insn 44) folds because it shares a cse basic block with the zero-init
  (insn 29) — the beqz (insn 40) fall-through arm has no CODE_LABEL/barrier before
  it and the firstfile call (insn 36) clobbers no pseudo, so the value table still
  carries reg75==0. CONTROL: the loop-body ++ (insn 58) is past code_label 45
  ("loop", a back-edge target NUSES>1) which starts a fresh cse block -> NOT folded
  (stays addsi3). A matching source must place the entry ++ at a cse-block boundary.

- [s7] COUPLING PROBE (forensic-only opt-barrier `__asm__("":"=r"(s1):"0"(s1))`,
  a CHEAT never proposed as candidate): blocking the fold does NOT flip the swap —
  counter pri RISES to 22000 (nrefs 8->11, livelen 14->15), pointer 5555, still
  74->s1/75->s0. Unfolding the entry ++ ADDS a read-ref to the counter, so fold and
  swap are coupled in the OPPOSITE direction from the original frontier lever:
  defeating the fold ENTRENCHES the counter's s0 win. This mechanism-level result
  explains why every single-lever attempt s1-s6 failed and STRENGTHENS the
  endgame-lock disposition. artifact tmp/grind/func_80037A20/s7/dump/barrier_ad.txt.

- [s7] Quantified flip target: under the byte-forced 33-insn stream the two callee-
  saved pseudos' refs/livelens are FIXED, so global.c is deterministic (counter wins
  s0). Pointer would need pri>17142 (~13+ refs; it has 5) or the counter demoted
  below 5882 (nrefs<=4 at len14, or livelen>40) — both unreachable without changing
  the emitted bytes. s3's return-value-split targeted the ref reduction but `result`
  copy-propagated away. Escape = a faithful count/pointer decomposition presenting
  <=4 counter allocno refs at fixed bytes AND entry ++ at a cse boundary (the two
  pull against each other: unfolding adds a ref). Floor unchanged 13; src HEAD-clean.

- [s7] First-hand ALLOCDBG: counter(75) pri=17142 (nrefs=8, livelen=14) -> s0(16); pointer(74) pri=5882 (nrefs=5, livelen=17) -> s1(17). Decisive ~2.9x swap, not a floor_log2 tie.

- [s7] CORRECTS s3: n_refs are 8 (counter) and 5 (pointer), NOT 4 and 6; floor_log2(8)=3 != floor_log2(5)=2, so n_refs is a live discriminator, not inert.

- [s7] CORRECTS s6: instrumented livelen is 14 (counter) < 17 (pointer); the counter is SHORTER-lived, contradicting s6's hand-counted 29..75 vs 13..51 'counter longer-lived' reading.

- [s7] cse FOLD pinned to basic-block level: entry ++ (insn 44) folds because it shares a cse basic block with the zero-init (insn 29) across the beqz fall-through (insn 40) and the pseudo-preserving firstfile call (insn 36); CONTROL: loop-body ++ (insn 58) past code_label 45 (NUSES>1) is NOT folded. A match needs the entry ++ at a cse-block boundary.

- [s7] Coupling probe: blocking the fold (opt-barrier) raises counter pri to 22000 and does NOT flip the swap; unfolding adds a counter ref. Fold-defeat entrenches, not demotes.

- [s7] Quantified flip target: under the fixed 33-insn bytes both pseudos' refs/livelens are fixed and global.c is deterministic; pointer would need ~13+ refs (has 5) or the counter demoted to nrefs<=4 at len14 (pri 5714<5882) or livelen>40 -- all unreachable without changing emitted bytes. s3's return-value-split targeted the ref reduction but 'result' copy-propagated away.

- [s7] cc1psx==fork already measured (s6, byte-identical); compiler-divergence foreclosed. Floor unchanged 13 (sandbox --disable all = 13, 33=33 insns, 0 rules); src/code6cac_c.c HEAD-clean (no edits).
