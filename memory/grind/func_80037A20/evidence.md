> **2026-08-24 MIGRATION NOTE:** HEAD is now `INCLUDE_ASM` (migrated in
> a7892ba2 (2026-08-24 sweep 2)); rules retired, in-source cheat-asm removed. "HEAD"
> claims below describe the pre-migration tree (`retired-chassis-2026-08/body.c`).

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

- [s8] s8 fresh floor: sandbox --disable all = 13 (33/33 insns, 0 regfix/asmfix rules, 34 cheat-asm instances stripped). HEAD byte-matches ONLY via 2 register-asm pins (s0/s1) + 1 __asm__ opt-barrier; zero rules.

- [s8] scan_hand_coded --single func_80037A20 = tier LOW, score 0/8 (no strong hand-coded indicators; too short for S3/S4, no multu pacing/empty-branch/cluster/BIOS pattern) -> canonical-asm route CLOSED.

- [s8] AND-gate #1 (canonical-asm) satisfied: LOW 0/8 -> not COMPLETED-INLINE-ASM-CANONICAL-eligible; the matching pure-C provably exists (s6: cc1psx byte-identical to our fork on candidate.c, compiler-divergence foreclosed).

- [s8] AND-gate #2 (coercion) satisfied: only forms reaching the byte-match are register-asm pins + opt-barrier, both with zero SOTN precedent (2026-07-01 census did NOT relax pins/barriers); the one theoretical pure-C escape is self-contradicting; duplicated-statement-into-arms structurally unavailable.

- [s8] SWAP mechanism (s7 ALLOCDBG, first-hand): counter(pseudo75) nrefs=8 livelen=14 pri=17142 -> s0; pointer(pseudo74) nrefs=5 livelen=17 pri=5882 -> s1; decisive ~2.9x, byte-forced under the 33-insn stream. PASS = global.c allocno_compare order.

- [s8] FOLD mechanism (s6/s7 -da): entry ++ (insn44) folds via cse.c FIRST pass REG_WAS_0 const-prop of the dominating var_s1=0 (insn29) sharing its cse basic block; loop-body ++ (past code_label 45) does NOT fold. Fold-defeat entrenches the swap.

- [s8] Axes measured dead across s1-s7: structural (s1-s3), permuter (s4-s5, 3 chassis / ~21k iters), compiler-divergence (s6, cc1psx==fork), ALLOCDBG dataflow lever (s7). No un-tried sanctioned axis remains.

- [s8] OWNER-ESCALATION entry filed at docs/grind/decisions.md (## 2026-07-24 — func_80037A20 (src/code6cac_c.c) — OWNER-ESCALATION), naming func_80037A20 directly, with both mutually-exclusive owner options (a) sanction coercion / (b) refuse+accept INCOMPLETE.

## [s9] solver modality — floor 13 -> 8; the endgame-lock claim is withdrawn

- The owner's 2026-08-24 queue directive (run the solver before any deep re-grind
  of an RA/scheduler-tiebreak residual) had never been executed here. s1-s8 all
  predate it. Executing it broke the eight-session plateau in one session.
- HEAD chassis re-measured at session start with the s1-s8 candidate pasted over
  the INCLUDE_ASM line: sandbox --disable all = 13, 33/33 insns, 0 rules. Chassis
  unchanged from the ledger.
- global.c law (confirmed against instrumented ALLOCDBG output on this function):
  pri = floor_log2(nrefs) * nrefs * size / live_length * 10000; allocnos sort by
  pri DESC (tie -> lower pseudo number = birth order); the first allocno takes the
  first free callee-saved register, i.e. $s0.
- The POINTER allocno is byte-forced at nrefs=5, live_length=17, pri=5882 when the
  function uses ONE pointer local: target's own stream puts the la at insn 5 and
  the last pointer use at insn 20 with no dead gap. That is the lowest priority any
  allocno in this function can have — which is why s1-s8 could never win $s0.
- Counter-side levers are foreclosed, now with a measured instance rather than an
  argument: the "n = var_s1 + 1" partition is byte-neutral (13 / 33 insns) and the
  split survives to global alloc, but ALLOCDBG gives n(p79) nrefs=4 len=7 pri=11428
  and count(p75) nrefs=4 len=8 pri=10000 — both still above 5882. Reason: nrefs and
  live_length are positively coupled, so every cut lowers both and the quotient is
  unmoved. Enumeration of all legal partitions:
  tmp/grind/func_80037A20/s9/partition_foreclosure.txt (min max-pri 12500 vs 5882).
  Banked at rejected/split-counter-partition-p2.c.
- THE LEVER: split the POINTER, not the counter. A base pointer (handed to
  firstfile) plus a walking pointer the loop advances — the natural spelling for
  walking a DIRENTRY array — cuts the pointer's 17-insn live range into 12 + 6 and
  leaves the dense references on the short half. Priority rises instead of falling.
  With a do/while loop:
      p    (walking) p75 nrefs=7  live_length=6  pri=23333 -> $s0   [= target]
      ctr            p76 nrefs=10 live_length=16 pri=18750 -> $s1   [= target]
      base           p74 nrefs=3  live_length=12 pri= 2500 -> $s0   [= target]
  base dies at the copy so it does not conflict with p; both take $s0 and the copy
  is a no-op move deleted by final.c, so the insn count stays 33.
  **sandbox --disable all = 8** — objdump confirms $s0 = pointer, $s1 = counter.
  The entire s0<->s1 rename (12 of the 13 diffs) is gone.
- Family stability: C_dowhile, W1 (copy outside the if), W3 (copy before the entry
  increment) and W4 (firstfile takes p) all measure 8 / 33. The goto-loop spelling
  measures 16 (p pri 13333 < counter 15000 — pointer sorts second). Stacking the
  counter split on top regresses to 18 / 35: the split counter no longer conflicts
  with the walking pointer, re-takes $s0, and the base pointer is pushed to $s2,
  costing a third save/restore pair.
- No coercion construct is involved anywhere in the new form: two pointer locals
  and a do/while loop, every value real and consumed. No FAKE annotation, no
  sanctioned-exception family claimed, nothing from the forbidden catalog.
- CONSEQUENCE FOR THE DISPOSITION: the 2026-07-24 OWNER-ESCALATION
  (docs/grind/decisions.md:1675) and the 2026-07-27 refusal (…:1778) both rest on
  "every sanctioned pure-C axis is measured dead". That premise is false. The
  function is grindable and stays ACTIVE. Recorded in decisions.md as a
  s9 correction entry.
- Remaining 8 diffs are both PRE-RA — see hypotheses.md [s9] frontier: (1) the
  `la D_80102810` lands in the block after the sprintf jal instead of the entry
  block, re-scheduling the prologue saves; (2) the entry increment folds to
  `li $s1,1` (cse.c first pass, REG_WAS_0) instead of `addiu $s1,$s1,1`. The s7
  objection to defeating the fold (it adds a counter reference and entrenches the
  counter's $s0 win) NO LONGER APPLIES, because the counter no longer competes for
  $s0 — that lever is worth re-opening first.

- [s9] Chassis re-measured at session start with the s1-s8 candidate pasted over HEAD's INCLUDE_ASM: sandbox --disable all = 13, 33/33 insns, 0 rules - matching the ledger, so every banked spelling conclusion was chassis-valid.

- [s9] global.c law confirmed against instrumented ALLOCDBG output on this function: pri = floor_log2(nrefs)*nrefs*size/live_length*10000; allocnos sort pri DESC (tie -> lower pseudo = birth order); the first allocno takes the first free callee-saved register, i.e. $s0.

- [s9] With ONE pointer local the pointer allocno is byte-forced at nrefs=5 / live_length=17 -> pri 5882, the lowest priority any allocno in this function can hold (target's own stream puts the la at insn 5 and the last pointer use at insn 20 with no dead gap). That, not a tiebreak, is why s1-s8 could never win $s0.

- [s9] Counter-side foreclosure now has a measured instance rather than an argument: the n = var_s1 + 1 partition is byte-neutral (13 / 33 insns) and survives to global alloc, yet both halves (pri 11428 and 10000) still out-rank 5882. Enumeration of all legal partitions in tmp/grind/func_80037A20/s9/partition_foreclosure.txt gives min max-pri 12500 vs 5882.

- [s9] The winning form: base pointer var_s0 handed to firstfile + walking pointer p advanced by a do/while loop. ALLOCDBG: p 7 refs / len 6 / pri 23333 -> $s0; counter 10 refs / len 16 / pri 18750 -> $s1; base 3 refs / len 12 / pri 2500 -> $s0. sandbox 8, 33/33 insns, 0 rules.

- [s9] The base-to-walking copy is a no-op move once both land in $s0 and is deleted by final.c, so the pointer split costs zero instructions.

- [s9] Loop shape is load-bearing: the goto-loop spelling gives p pri 13333 < counter 15000 (sandbox 16); the do/while spelling gives p 23333 > counter 18750 (sandbox 8).

- [s9] The counter must stay UNSPLIT: splitting it removes its conflict with the walking pointer, it re-takes $s0 and the base is pushed to $s2 (+2 insns, sandbox 18).

- [s9] No coercion construct appears anywhere in the new body: two pointer locals and a do/while loop, every value real and consumed, nothing dead, no register/asm/volatile/pad/alias. No FAKE annotation is needed or present and no sanctioned-exception family is claimed.

- [s9] Endgame-lock gates re-run for the record and both still FAIL (and are now moot): scan_hand_coded --single func_80037A20 = tier LOW, score 0/8; no SOTN-master precedent is cited or needed because the closing form requires no construct that would need one.

- [s9] Remaining 8 diffs are both PRE-RA - register allocation is SOLVED. (1) la D_80102810 is emitted in the basic block AFTER the sprintf jal; target emits the lui/addiu pair in the ENTRY block, which changes how the three register saves are distributed and which one fills the jal delay slot. (2) The entry increment folds to li $s1,1 (cse.c FIRST pass, REG_WAS_0 const-prop of the dominating var_s1 = 0) where target has addiu $s1,$s1,1.

- [s9] s7's objection to defeating the fold - that unfolding adds a counter read-reference and entrenches the counter's $s0 win - NO LONGER APPLIES under the s9 allocation, because the counter no longer competes for $s0. That lever is re-opened.

- [s9] ra_solver inverse_compose classify reports FIRST DIVERGENCE: PRE-RA on the honest-vs-target text streams (instruction multiset differs by exactly the folded entry increment), consistent with the post-s9 picture that only pre-RA residuals remain.

## [s10] rederive modality — floor 8 -> 6

- [s10] CHASSIS: the s9 candidate (base + walking pointer split) re-measures at
  `sandbox func_80037A20 --disable all` = 8, 33/33 insns, 0 rules — the ledger
  floor is confirmed against HEAD before anything else was spent.
- [s10] The s9 split's 8 diffs are ENTIRELY entry-block schedule: the `lui/addiu`
  pair for D_80102810 is emitted after the sprintf jal, where target emits it
  before, and the three callee-saved stores + the `addiu a0,sp,0x10` redistribute
  around it. Register allocation in the s9 body is already target's.
- [s10] The ordinary ONE-POINTER do/while body (identical to the s1..s8 body but
  with a do/while loop instead of goto) has the OPPOSITE profile: its entry block
  matches target instruction-for-instruction, and all 13 diffs are the s0<->s1
  rename plus the entry-increment fold.
- [s10] PASS ATTRIBUTION (first-hand, dumps read, not inferred): the `la` moves in
  **sched1** and nowhere earlier — it is still ahead of the sprintf call in
  code6cac_c.cse, .loop, .combine and .flow, and appears between insn 32 and insn
  34 in .lreg. The sched1 dump names the reason:
    split body:       `;; ready list at T-4: 32 (1) 29 (1) 13 (7f000001), now 13 32 29`
    one-pointer body: `;; ready list at T-4: 32 (1) 29 (1) 13 (1), now 32 29 13`
  0x7f000001 is max_priority, applied by adjust_priority() in
  tools/gcc-2.7.2/sched.c when birthing_insn_p() is true, i.e. when
  `reg_n_sets[REGNO(dest)] == 1`.
- [s10] MECHANISM: sched1 schedules BACKWARD, so a max_priority boost emits the
  insn LATE (the pass's own register-lifetime shortening heuristic). Any pointer
  spelling that leaves the address-holding pseudo with exactly one set gets its
  `la` sunk to sit adjacent to its only consumer. A walking pointer has two sets
  (the la and the `+= 0x28`) and is never boosted.
- [s10] cse1 DELETES any set whose source is a constant or constant-equivalent
  (a symbol_ref, or frame-pointer + offset), so "give the base pointer a second
  set" cannot be spelled byte-free: both attempts (format-string address first,
  stack-buffer address first) had their first set propagated away and reverted to
  reg_n_sets == 1 with the la still sunk (sandbox 8 in both cases).
- [s10] reg_n_refs is LOOP-DEPTH WEIGHTED — flow.c adds loop_depth per reference,
  so a reference inside the loop body counts TWICE. This reconciles the ALLOCDBG
  numbers with hand counts and is why every earlier hand estimate in this ledger
  was low.
- [s10] MEASURED allocno inputs (tools/ra_solver/extract.py; models saved under
  tmp/grind/func_80037A20/s10/):
    one-pointer do/while, zero-init AFTER the call:
      ptr(74)     nrefs=8  livelen=17  pri=14117   -> $s1   [wrong]
      counter(75) nrefs=10 livelen=14  pri=21428   -> $s0   [wrong]   sandbox 13
    same body + s9's counter partition `n = var_s1 + 1`:
      n(79)  6/7  = 17142 ; ptr(74) 8/17 = 14117 ; var_s1(75) 4/8 = 10000
      -> counter family still first, sandbox 13
    one-pointer do/while, zero-init BEFORE the call  [the s10 candidate]:
      ptr(74)     nrefs=8  livelen=16  pri=15000   -> $s0   [TARGET]
      counter(75) nrefs=10 livelen=20  pri=15000   -> $s1   [TARGET]
      EXACT TIE; allocno_compare falls through to pseudo number, and var_s0 is
      declared before var_s1, so 74 is allocated first and takes $s0.
      **sandbox --disable all = 6**, 33/33 insns, 0 rules, 0 coercion constructs.
      objdump confirms $s0 = pointer and $s1 = counter throughout.
- [s10] The tie is order-sensitive, which is itself evidence the model is right:
  initialising the pointer before the counter measures 15; the goto loop chassis
  with the same hoisted init measures 14; splitting the zero-init into its own
  local measures 13.
- [s10] s9's counter-side foreclosure is chassis-stale: it was computed against a
  pointer priority of 5882 (the goto chassis), and the do/while chassis puts the
  pointer at 14117. The counter family is still foreclosed, but the binding
  reason is the loop-depth ref weighting (the loop-carried counter pseudo floors
  at 6 refs / livelen 7 = 17142), not s9's coupling argument.
- [s10] REMAINING 6 diffs, all one scheduling decision plus the known fold:
    ours   [sp] a2 a3 | sw s1 | s1=0 | sw s0 | la s0 | la a1 | sw ra | jal | (delay) addiu a0
    target [sp] a2 a3 | sw s0 | la s0 | addiu a0 | la a1 | sw ra | jal | (delay) sw s1 | s1=0
  `move s1,zero` has no consumer in the entry block, so rank_for_schedule falls
  through to a DESCENDING INSN_LUID sort; the hoisted statement has a low LUID and
  is therefore emitted first, where target emits it last. Plus the long-known
  `li $s1,1` vs `addiu $s1,$s1,1` cse1 REG_WAS_0 fold.
- [s10] Artifacts: tmp/grind/func_80037A20/s10/{apply.py,dis.sh,v_*.c,
  model_onept.json,model_P1.json,model_Q.json,cse_region.txt,loop_region.txt,
  combine_region.txt,flow_region.txt,lreg_region.txt}; dumps regenerated into
  tmp/grind/func_80037A20/dumps/.

- [s10] s9 candidate re-verified against HEAD this session before anything was spent: sandbox func_80037A20 --disable all = 8, 33/33 insns, 0 rules.

- [s10] The s9 base+walking pointer split is structurally self-defeating: it buys target's register allocation and pays for it with target's prologue schedule, because a single-set address pseudo triggers sched.c birthing_insn_p()/adjust_priority() and its la is emitted late.

- [s10] PASS ATTRIBUTION (first-hand, dumps read): the la placement divergence happens in sched1 and nowhere earlier - the insn is still ahead of the sprintf call in code6cac_c.cse, .loop, .combine and .flow, and has moved by .lreg.

- [s10] The sched1 ready-list dump names the boost directly: 0x7f000001 = max_priority on insn 13 in the split body, plain 1 in the one-pointer body.

- [s10] cse1 deletes any set whose source is constant or constant-equivalent (symbol_ref, frame-pointer + offset), which forecloses every byte-free way to give an address pseudo a second set.

- [s10] reg_n_refs is LOOP-DEPTH WEIGHTED (flow.c adds loop_depth per reference) - loop-body references count twice. Every earlier hand-count of allocno refs in this ledger was low for this reason.

- [s10] MEASURED: the one-pointer do/while body with the zero-init before the sprintf call gives ptr(74) 8 refs / 16 len / pri 15000 and counter(75) 10 / 20 / 15000 - an exact allocno_compare tie, broken by pseudo number in favour of var_s0 (declared first), which takes $s0. Target's assignment; objdump confirms $s0 = pointer and $s1 = counter throughout.

- [s10] NEW FLOOR: sandbox func_80037A20 --disable all = 6, 33/33 insns, 0 rules, no coercion construct, no FAKE annotation, nothing from any sanctioned-exception family. Down from 8 (s9) and 13 (s1-s8).

- [s10] The tie is order-sensitive and that sensitivity is itself confirmation of the model: pointer-init-before-counter measures 15, the goto loop chassis with the same hoisted init measures 14, a split zero-init measures 13, and a pointer-init-after-the-call variant measures 7.

- [s10] The remaining 6 diffs are entirely one entry-block scheduling decision plus the known cse1 fold. Ours: [sp] a2 a3 | sw s1 | s1=0 | sw s0 | la s0 | la a1 | sw ra | jal | (delay) addiu a0. Target: [sp] a2 a3 | sw s0 | la s0 | addiu a0 | la a1 | sw ra | jal | (delay) sw s1 | s1=0. `move s1,zero` has no consumer in the entry block, so rank_for_schedule falls through to a DESCENDING INSN_LUID sort; the hoisted statement has a low LUID and is emitted first where target emits it last.

- [s10] The candidate body contains no construct from any forbidden or sanctioned-exception family: it is a single walking pointer, a counter, a stack buffer that is actually written by sprintf, and a do/while loop. The only unusual thing about it is the ORDER of two ordinary initialisations.

## [s11] rederive modality — floor stays 6; the allocation/schedule conflict is now a closed-form inequality

- [s11] Chassis re-verified at session start: the s10 candidate applied to
  src/code6cac_c.c measures `sandbox func_80037A20 --disable all` = **6**,
  33/33 insns, 0 rules, 4 cheat-asm stripped (unrelated neighbours in the TU).
  The ledger floor of 6 is current; nothing drifted.

- [s11] The exact 6-diff residual, read off objdump of the sandbox object vs
  asm/funcs/func_80037A20.s.  Entry-block emission, ours then target:
    ours   : addiu sp,-64 | move a2,a0 | move a3,a1 | sw s1,52 | move s1,zero |
             sw s0,48 | la s0 | la a1 | sw ra,56 | jal sprintf | (delay) addiu a0,sp,16 |
             addiu a0,sp,16 | jal firstfile | (delay) move a1,s0
    target : addiu sp,-64 | move a2,a0 | move a3,a1 | sw s0,48 | la s0 |
             addiu a0,sp,16 | la a1 | sw ra,56 | jal sprintf | (delay) sw s1,52 |
             move s1,zero | addiu a0,sp,16 | jal firstfile | (delay) move a1,s0
  Everything from the beqz onward is identical except the peeled increment
  (`li $s1,1` vs `addiu $s1,$s1,1`).  Same insn count, same registers.

- [s11] MIPS has no `REG_ALLOC_ORDER` in GCC 2.7.2 (`tools/gcc-2.7.2/config/mips/mips.h`
  defines none), and every hard-reg preference for both allocnos is pruned
  (measured: `prefs == {}`, `copy_prefs == {"74": [], "75": []}` on all bodies).
  Therefore the first allocno in global.c's priority sort takes $s0 and the second
  takes $s1, unconditionally.  The only lever on the disposition is the priority
  `floor_log2(nrefs)*nrefs*size/live_length*10000`, and its only pure-C inputs are
  the weighted ref count and the live length.

- [s11] Weighted ref counts are structurally fixed by target's own byte stream:
  pointer Rp = 8, counter Rc = 10 with our folded peel (11 with target's
  `addiu $s1,$s1,1`).  Both counts are forced by the 33 instructions target emits,
  so the inequality cannot be re-balanced from the ref side.  Consequently the
  pointer wins the $s0 seat iff **Lc >= 1.25 * Lp** (or 1.375 with the unfolded
  peel).

- [s11] Full 2x2 statement-position matrix, sandbox + ALLOCDBG (models in
  tmp/grind/func_80037A20/s11/model_{base,v7,v9}.json):
      ptr before / zero before : 8/16=15000 vs 10/20=15000  TIE -> ptr $s0 : **6**
      ptr after  / zero before :                                        : **7**
      ptr after  / zero after  : 8/11=21818 vs 10/15=20000  ptr $s0      : **8**
      ptr before / zero after  : 8/17=14117 vs 10/14=21428  counter $s0  : **13**
  Target's own layout (la before the jal AND zero-init after the jal) demands
  Lp ~= 17 with Lc ~= 14, a ratio of 0.82 against a requirement of 1.25.  The
  statement-position route to target is FORECLOSED by a factor of 1.5.

- [s11] The `ptr after / zero after` body (sandbox 8) is the first body in eleven
  sessions to place `move $s1,$zero` in target's slot (immediately after the
  sprintf jal) with `sw $s1,0x34($sp)` in the delay slot AND keep target's $s0/$s1
  assignment.  Its only entry-block defect is the `la $s0` sinking past the jal.
  Saved as memory/grind/func_80037A20/rejected/s11-both-inits-after-call-la-sinks.c
  — worth re-reading, it is the closest thing to a second chassis.

- [s11] First-hand sched1 attribution (dumps/code6cac_c.sched, this session):
  block 0 = insns 4..40, containing BOTH calls; every insn ties at INSN_PRIORITY 1
  except the terminating jump (0x7fffffae) and two argument moves that carry the
  birthing boost (0x7f000001) but are deleted by coalescing.  The emitted order is
  exactly ascending insn number.  **sched1 performs no reordering in this function**,
  so entry-block emission order == RTL order == source statement order.  The s10
  frontier item that hoped to sink the zero-init via the birthing boost is dead
  twice over: reg_n_sets[counter] == 4 (no boost possible), and even a boosted insn
  caps at 0x7f000001 < the jump's 0x7fffffae, so it could only reach the
  second-to-last slot of the block — never past the jal.

- [s11] The callee-saved saves are NOT RTL insns.  mips.c prints `sw $sN,off($sp)`
  as text immediately before the first insn that touches $sN (verified on raw cc1
  output, tmp/grind/func_80037A20/s11/tu.s: `sw $17,52` precedes `move $17,$0`,
  `sw $16,48` precedes `la $16`, `sw $31,56` precedes the jal).  Target obeys the
  same rule.  So the `sw $s1` position is a dependent variable, never a target in
  its own right.

- [s11] The remaining reconciliation, and the frontier: if reorg left the sprintf
  jal's delay slot EMPTY on the base body, the lazy `sw $s1` save plus
  `move $s1,$zero` would fall after the jal (assembler pulls the save into the
  slot) and `addiu $a0,$sp,0x10` would stay at its RTL position before `la $a1` —
  reproducing target's entry block exactly, with the 15000/15000 tie untouched
  because the RTL order does not move.  reorg currently fills the slot with insn 21
  (`addiu $a0,$sp,0x10`): both `la`s are length-8 macros and ineligible, so insn 21
  is the nearest eligible candidate in the backward scan.  Unread this session:
  tmp/grind/func_80037A20/dumps/code6cac_c.dbr (generated) and
  tools/gcc-2.7.2/reorg.c `fill_simple_delay_slots`.

- [s11] Rederived loop/exit shapes, all measured, all worse: branch-target peel with
  duplicated tail 34 insns / 17; `while(1){...break}` 34 / 10; early-exit
  `if (!firstfile) { D = 0; return 0; }` 34 / 17; nextfile inlined into the
  do/while condition 35 / 9.  `u8 sp10[32]` instead of `s32 sp10[8]` is byte-
  identical (33 / 6) — the buffer's C type is inert.

- [s11] Chassis re-verified: the s10 candidate applied to src/code6cac_c.c measures sandbox --disable all = 6, 33/33 insns, 0 rules. The ledger floor of 6 is current; nothing drifted this session.

- [s11] The exact residual, objdump of the sandbox object vs asm/funcs/func_80037A20.s: ours = [addiu sp,-64 | move a2,a0 | move a3,a1 | sw s1,52 | move s1,zero | sw s0,48 | la s0 | la a1 | sw ra,56 | jal sprintf | (delay) addiu a0,sp,16 | addiu a0,sp,16 | jal firstfile | (delay) move a1,s0]; target = [addiu sp,-64 | move a2,a0 | move a3,a1 | sw s0,48 | la s0 | addiu a0,sp,16 | la a1 | sw ra,56 | jal sprintf | (delay) sw s1,52 | move s1,zero | addiu a0,sp,16 | jal firstfile | (delay) move a1,s0]. Everything from the beqz onward matches except the peeled increment (li $s1,1 vs addiu $s1,$s1,1).

- [s11] MIPS has no REG_ALLOC_ORDER in GCC 2.7.2 and every hard-reg preference for both allocnos is pruned (prefs == {} measured on all bodies), so the first allocno in global.c's sort takes $s0 unconditionally and the only pure-C lever is the priority formula's two inputs (weighted refs, live length).

- [s11] Weighted ref counts are fixed by target's own byte stream: pointer Rp = 8 (la 1 + loop addiu 2*2 + loop move a0 1*2 + move a1 1) and counter Rc = 10 folded / 11 unfolded (init 1 + peel 1 or 2 + loop addiu 2*2 + tail addiu -1 2 + sw 1 + move v0 1). Hence the pointer wins the $s0 seat iff Lc >= 1.25*Lp (1.375 unfolded).

- [s11] 2x2 statement-position matrix with ALLOCDBG ground truth: (ptr before, zero before) 15000/15000 tie -> ptr $s0, sandbox 6; (ptr after, zero before) sandbox 7; (ptr after, zero after) 21818/20000 -> ptr $s0, sandbox 8; (ptr before, zero after) 14117/21428 -> counter $s0, sandbox 13. Target needs Lp ~ 17 with Lc ~ 14 simultaneously (ratio 0.82) against a bar of 1.25.

- [s11] The (ptr after, zero after) body at sandbox 8 is the first body in eleven sessions to place move $s1,$zero in target's slot immediately after the sprintf jal AND put sw $s1,0x34($sp) in the delay slot AND keep target's $s0/$s1 assignment; its only entry-block defect is the la $s0 sinking past the jal. Banked as memory/grind/func_80037A20/rejected/s11-both-inits-after-call-la-sinks.c - it is effectively a second chassis, not just a dead form.

- [s11] First-hand sched1 attribution: block 0 = insns 4..40 containing both calls; all insns tie at INSN_PRIORITY 1 except the terminating jump (0x7fffffae) and two coalesced-away argument moves (0x7f000001); the emitted sequence is exactly ascending insn number, so sched1 performs no reordering in this function and entry-block emission order == source statement order.

- [s11] Insn identities in block 0 for future sessions: 4 = reg72 <- $a0; 6 = reg73 <- $a1; 13 = reg75 <- 0 (the counter zero-init); 16 = reg74 <- &D_80102810 (the la); 21 = $a0 <- sp+16; 23 = $a1 <- fmt; 25/27 = $a2/$a3 <- reg72/reg73 (deleted by coalescing); 29 = call sprintf; 32 = $a0 <- sp+16; 34 = $a1 <- reg74; 36 = call firstfile; 40 = beqz.

- [s11] The callee-saved saves are not RTL insns: mips.c emits sw $sN,off($sp) as text immediately before the first insn touching $sN. Verified on raw cc1 output (tmp/grind/func_80037A20/s11/tu.s) and consistent with target. The sw $s1 position is therefore fully determined by the move $s1,$zero position.

- [s11] Reconciliation candidate (the new frontier): reorg fills the sprintf jal delay slot with insn 21 (addiu $a0,$sp,0x10) because both la insns are length-8 macros and hence ineligible, leaving insn 21 as the nearest eligible candidate in fill_simple_delay_slots' backward scan. If that slot were instead left empty, the lazy sw $s1 save plus move $s1,$zero would fall after the jal (the assembler pulls the save into the slot) and addiu $a0,$sp,0x10 would stay at its RTL position before la $a1 - reproducing target's entry block bit-for-bit with the 15000/15000 allocation tie untouched, because the RTL order does not change.

- [s11] Rederived shapes measured and dead: branch-target peel with duplicated tail 34 insns/17; while(1){...break} 34/10; early-exit return-zero 34/17; nextfile inlined into the loop condition 35/9. u8 sp10[32] vs s32 sp10[8] is byte-identical at 33/6.

- [s11] src/code6cac_c.c was reverted to HEAD (INCLUDE_ASM) at end of session; the working body lives in memory/grind/func_80037A20/candidate.c and is unchanged from s10.

## [s12] structural — three-pseudo chassis, gate decomposition

- Target's own 33 insns FIX Rp = 8 and Rc = 11 (10 folded) for ANY two-pseudo
  decomposition; with the la before the sprintf jal and the zero-init after it,
  Lp = Lc + 3, so allocno_compare can never sort the pointer first.  A two-pseudo
  body is arithmetically incapable of matching this function.  The match needs a
  THIRD pseudo whose copy is deleted as a no-op move.
- flow.c:2087 confirms "reg_n_refs[regno] += loop_depth" on every set (and the
  mirror on every use), i.e. each (use, set) occurrence is weighted 1 outside the
  loop and 2 inside.  global.c allocno_compare truncates pri to int and tie-breaks
  on allocno index (ascending pseudo number); find_reg's pass-1 loop takes the
  lowest-numbered non-conflicting hard reg, so the first-sorted allocno gets $s0.
  (pass 0 only considers regs already in regs_used_so_far, which for a
  call-crossing allocno always fails, so pass 1 decides.)
- NEW CHASSIS vJ (memory/grind/func_80037A20/chassis_s12_vJ_basewalk.c), sandbox 8,
  33/33 insns, ordinary C: base pseudo (la) + walking pointer + counter.  The
  "p = var_s0" copy is byte-free (deleted as a no-op move); hard_reg_copy_preferences
  DOES bind the two pointer pseudos to the same hard reg, so the s11 observation
  that prefs/copy_prefs are always empty does not generalise past two-pseudo bodies.
- vJ ALLOCDBG (sched1 on): walking 7 refs/len 6 = 23333 -> $s0, counter 10/16 = 18750
  -> $s1, base 3/12 = 2500 -> $s0.  Target's assignment with a STRICT margin (the
  s10/s11 floor-6 body only ever reached it as a 15000/15000 tie).
- vJ built with -fno-schedule-insns (forensic only; NOT a build path) is TARGET LINE
  FOR LINE, delay slots included, with exactly ONE divergence: peel li $17,1 vs
  target addiu $s1,$s1,1.  Artifact: tmp/grind/func_80037A20/s12/code6cac_c.nosched.s
- GATE 1 = sched.c:2504 birthing_insn_p (reg_n_sets[i] == 1) + sched.c:2584
  adjust_priority's max_priority boost, which sinks the base's la past the sprintf
  jal (sched1 schedules backward, so a boosted insn is emitted late, landing just
  before its only in-block consumer addu $a1,$s0,$zero).  n_deaths is always 0
  (REG_DEAD notes are stripped before adjust_priority runs — its own comment), so
  reg_n_sets >= 2 on the base pseudo is the ONLY disqualifier.
- GATE 2 = the cse1 REG_WAS_0 fold of the peel.  On vJ it is DECOUPLED from the
  allocation for the first time: unfolding (Rc 10 -> 11) leaves the counter at
  20625 vs the walking pointer's 23333 (sched1 on) and 17368 vs 17500 (sched1 off).
  Every s2/s6/s10 fold-defeat kill was measured on two-pseudo chassis where
  unfolding cost the allocation tie; those kills are chassis-stale for vJ.
- Statement-position facts re-measured this session: one-pointer body with target's
  statement order = 13; base+walk with target's order (vA) = 8; base+walk with the
  zero-init before the sprintf call (vB) = 8; base+walk with the walk-init below the
  peel (vJ) = 8.  vA's allocnos tie at 20000/20000; vJ converts that to a strict win.
- The session's best form is unchanged at floor 6 (the s10/s11 body), re-verified
  this session at sandbox --disable all = 6, 33/33 insns, 0 rules.

- [s12] Re-verified this session: the s10/s11 body in src/code6cac_c.c measures sandbox --disable all = 6, 33/33 insns, 0 rules, 4 cheat-asm stripped (chassis-check number, HEAD was reported unavailable at dispatch).

- [s12] flow.c:2087 - reg_n_refs[regno] += loop_depth on every set, with the mirror increment on every use; loop-body references therefore count double. Confirms the (use,set)-weighted model used for all ref arithmetic in this ledger.

- [s12] global.c allocno_compare: pri = floor_log2(nrefs)*nrefs/live_length*10000*size, truncated to int, tie-broken by `*v1 - *v2` (allocno index = ascending pseudo number). find_reg pass 0 only considers hard regs already in regs_used_so_far (always fails for a call-crossing allocno), so pass 1 decides and takes the lowest-numbered non-conflicting reg: the first-sorted allocno gets $s0.

- [s12] Target's 33 insns fix Rp = 8 and Rc = 11 (10 folded) for any two-pseudo decomposition, and target's own emission order forces Lp = Lc + 3, against a requirement of Lc > 1.375*Lp. A two-pseudo body is arithmetically incapable of matching this function.

- [s12] NEW three-pseudo chassis vJ (memory/grind/func_80037A20/chassis_s12_vJ_basewalk.c), ordinary C, 33/33 insns, sandbox 8. ALLOCDBG (sched1 on): 75 walking 7 refs/len 6 = 23333 -> $s0; 76 counter 10/16 = 18750 -> $s1; 74 base 3/12 = 2500 -> $s0. Target's assignment with a STRICT margin (the floor-6 body only ever reached it as a 15000/15000 tie).

- [s12] The `p = var_s0` copy in vJ is byte-free: base and walking pointer do not conflict and hard_reg_copy_preferences binds them to the same hard reg, so the copy is deleted as a no-op move. This disproves the s11 generalisation that prefs/copy_prefs are always empty on this function.

- [s12] vJ compiled with -fno-schedule-insns (forensic only; not a build path) is target LINE FOR LINE including both delay slots, with exactly one divergence: peel li $17,1 vs target addiu $s1,$s1,1. Artifact tmp/grind/func_80037A20/s12/code6cac_c.nosched.s.

- [s12] GATE 1 (the only reason vJ is 8): sched.c:2504 birthing_insn_p (reg_n_sets[i] == 1) + sched.c:2584 adjust_priority's max_priority boost sink the base's la past the sprintf jal, because sched1 schedules backward. n_deaths is always 0 (REG_DEAD notes stripped before adjust_priority runs, per its own comment), so reg_n_sets >= 2 on the base pseudo is the ONLY disqualifier.

- [s12] GATE 2 on vJ is decoupled from the allocation: unfolded, the counter is 33/16 = 20625 vs the walking pointer's 23333 (sched1 on) and 33/19 = 17368 vs 17500 (sched1 off). Every earlier fold-defeat kill was measured on a two-pseudo chassis where unfolding cost the allocation, so those kills are chassis-stale for vJ.

- [s12] Statement-position re-measurements this session: one-pointer body in target's statement order = 13 (ptr 8/17 = 14117, counter 10/14 = 21428); base+walk in target's order (vA) = 8 (20000/20000 tie); base+walk with the zero-init before the sprintf call (vB) = 8; vJ = 8.

- [s12] Gate-1 spellings measured and killed: g1a (no base variable, address expression passed directly) - cse1 makes one shared address pseudo, still 1 set, sandbox 8; g1d (var_s0 = p after the loop) - dead store, DCE'd before flow, sandbox 8; vK (p = var_s0 hoisted above the if) - cse copy-propagates the split away, only two allocnos remain, sandbox 8.

- [s12] s11's open frontier item (empty sprintf delay slot collapses the residual on the floor-6 body) is KILLED by reading the cc1 -da .s: cc1 emits `move $17,$0` BEFORE the jal on that body, so no delay-slot decision can relocate it.

## [s13] MATCHED � honest pure-C byte match, full-build SHA1 verified

- [s13] `src/code6cac_c.c` func_80037A20 written as the plain one-pointer PsyQ
  file-count idiom (zero-init AFTER the sprintf call, counter increment at the TOP of
  the do/while body, no source-level peel and no source-level `-= 1` tail) measures
  `sandbox --disable all` = **0**, 33/33 insns, rules_dropped 0, AND `wteng main build`
  prints **MATCH** with link SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == the oracle.
  Body: memory/grind/func_80037A20/candidate.c (== matched_body_s13.c).

- [s13] LOAD-BEARING LINK CORRECTION (cost the previous, discarded s13 run its outcome):
  the callees must be spelled `sprintf`, `firstfile`, `nextfile` � the names the linker
  resolves (declared at src/code6cac_c.c:156-157 and include/code6cac.h:501).  A body
  spelling them `func_80079A30` / `bios_firstfile_B` / `bios_nextfile_B` still scores
  `sandbox --disable all` = 0 with 33/33 insns, because the sandbox compares at OBJECT
  level with relocations masked, but the full build fails the link with three
  "undefined reference" errors.  GENERAL LESSON for every grind session: a sandbox 0 is
  necessary, not sufficient � the callee/global SPELLING is invisible to the sandbox and
  visible only to `build`.  This is the concrete instance of the ledger's
  "masked-0 register diff class" Judge constraints.

- [s13] The whole s12 GATE-1/GATE-2 frontier DISSOLVES rather than being defeated:
  * GATE 1 (sched.c birthing_insn_p boost sinking `la $s0` past the sprintf jal) never
    fires on this body because the single walking pointer has reg_n_sets == 2 (the `la`
    plus the in-loop `+= 0x28`), so no base/walking split � and no second-set spelling �
    is needed at all.  The three-pseudo vJ chassis (sandbox 8) is superseded.
  * GATE 2 (the cse1 REG_WAS_0 fold that produced `li $s1,1` on every earlier chassis)
    never fires because the surviving increment sits after the loop's CODE_LABEL, which
    terminates cse's extended basic block, so the dominating `var_s1 = 0` is not
    const-propagated into it.
  * The peel + compensating decrement that s1..s12 tried to spell in C are supplied by
    reorg.c: our matched output (== target) carries `addiu $s1,$s1,1` in the `bnez` delay
    slot and `addiu $s1,$s1,-1` after the loop, neither of which appears in the C.
    Writing the decrement at source level duplicates reorg's compensation and over-counts
    (rejected/s13-source-level-decrement-duplicates-reorg-compensation.c).

- [s13] The long-standing s10/s11 floor-6 body (zero-init HOISTED above the sprintf call,
  explicit `var_s1++` peel, explicit `var_s1 -= 1;` tail) was the trap: every one of its
  three deliberate deviations from the plain idiom was individually motivated by an
  allocation measurement, and together they cost the match.  The winning move was to
  delete all three and write the ordinary loop.
