# Hypothesis ledger — func_8003D888

## s1 (recon, 2026-08-31) — floor 32 → 13

- H1 CONFIRMED: the function is a plain bitstream reader (state {word ptr, cur word, bits
  avail} through the u32* param); natural C compiles to the right CFG and 35-37/37 insns with
  zero coercion constructs. The legacy committed chassis (full register-pin + __asm__ body)
  was pure cheat scaffolding and is replaced.
- H2 CONFIRMED: target's $a1 carrying avail→ptr→word and the in-place `subu a3,a3,a1` mean the
  original REUSES the param `n` as the if-arm ptr/word carrier while a copy (`cnt`) carries the
  bit count. Measured gradient: fresh-locals 30 → param-as-need 26 → merged tmp 22 →
  param-as-ptr/word + cnt copy **13** (candidate.c, 37/37 insns, both arg-copy moves present).
- H3 CONFIRMED (mechanism, greg-dump-proven): our residual seat rotation is driven by
  global.c preference machinery. n's allocno carries "preferences: 5" from the entry copy;
  regs_someone_prefers makes every higher-priority allocno SKIP $a1 (find_reg first pass,
  global.c:1001), so whoever should share $a1 can't. The preference is purged
  (prune_preferences, global.c:908) only when hard $a1 is live inside n's range — achieved
  when local-alloc seats a block-local ptr/word carrier in $a1. Verified empirically: the
  fresh-`w` variant purged the pref (n moved off $a1).
- H4 KILLED (measured): source-order reordering (`n = s[0]` above `cnt -= avail`) to force the
  in-place subtract — scores 20. It does produce the in-place subu, but sched1 then fills the
  avail-load delay slot with the cnt entry-copy, keeping h5 live past the avail load → avail
  conflicts hard $a1 → seat rotation gets WORSE. See rejected/ptr-store-before-subtract-reorder.c.
- H5 OPEN (next session's frontier): the last structural fact to reproduce is target's entry
  sequence `move s; move n; lw avail; nop; slt` — an UNFILLED load-delay slot, i.e. sched1 kept
  both entry copies ABOVE the lw where ours sinks the second copy into the slot. Once that
  holds, per the conflict analysis in evidence.md s1 the seating cascades to target
  (avail→$a1 shares with ptr/word, cnt→$a3, s→$a2). Probe with the .sched dump (read the two
  insns' priorities / dependence classes at the tie) and/or tools/sched_solver; check whether
  the sched-rank-class-tie-wall rule (equal priority ⇒ rank_for_schedule dependence-class
  tie-break) applies to the {n-copy, avail-lw} pair.
- H6 OPEN: remaining if-arm slot diffs at floor 13 (`sllv r` after the `and` vs after the word
  load; `li 32`/`lw word` order) are downstream of seating; re-diff only after H5 resolves.

## [s1] func_8003D888 is a plain bitstream reader (state {word ptr, cur word, bits avail}) writable as natural C with no coercion constructs
- mechanism: semantics from asm + caller func_8003D7B4 (reads 4-bit length then n-bit value; decisions.md:483)
- probe: replaced the committed register-pin/__asm__ chassis with natural C; sandbox --disable all
- result: 35-37/37 insns, correct CFG; gradient 30/26/22/13 across spellings
- verdict: CONFIRMED

## [s1] the original reuses the PARAM n as the if-arm ptr/word carrier ($a1) with a fresh copy (cnt) carrying the bit count; need is decremented in place
- mechanism: target $a1 carries avail->ptr->word; entry copy addu a3,a1 is real; subu a3,a3,a1 is in-place; only the param's entry-copy preference legitimately holds $a1 for ptr/word
- probe: spelled cnt = n; n = s[0]; s[0] = n + 4; n = *(u32 *)n; sandbox
- result: score 13 (from 22), 37/37 insns, s->$a2 ptr/word->$a1 r->$v1 one/m2->$a0 all correct
- verdict: CONFIRMED

## [s1] our residual seat rotation is caused by n's allocno $a1 copy-preference making higher-priority allocnos skip $a1 (regs_someone_prefers), purged only when hard $a1 is live inside n's range
- mechanism: global.c find_reg:1001 first-pass IOR of regs_someone_prefers; prune_preferences:908 purge on hard-reg conflict; set_preference:1661; verified in .greg dump ('73 preferences: 5')
- probe: fresh merged w local variant: local-alloc seats w in $a1 in the if-arm
- result: n's pref purged (n moved off $a1) exactly as predicted; variant scored 20 for a separate scheduling reason
- verdict: CONFIRMED

## [s1] moving the ptr assignment above the subtract (to invalidate the cnt==n cse equivalence and force the in-place subu) lowers the floor
- mechanism: cse.c equivalence substitution extends n's live range into conflict with avail
- probe: reordered source (n = s[0] before cnt -= avail); sandbox
- result: 20 (worse than 13): in-place subu achieved but sched1 fills the avail-load delay slot with the cnt copy, h5 stays live past the load, avail permanently conflicts $a1
- verdict: KILLED

## [s2] H5 (s1): reproducing an "unfilled load-delay slot" entry schedule is the gating lever
- mechanism claimed by s1: sched1 sinks the second entry copy into the avail-load delay slot,
  keeping hard $a1 live past the load so avail can never take $a1
- probe: read tmp/grind/func_8003D888/dumps/code6cac_c2.sched for block 0 (variant B); read the
  target asm around 8003D890
- result: block 0's scheduling region is insns {12,15,16} — the entry copies (4, 6) are above
  NOTE_INSN_FUNCTION_BEG and are never scheduled. The delay-slot move is placed by sched2 /
  the delay-slot filler AFTER register allocation, so it cannot affect seating; the target's
  `nop` is an assembler-inserted load-delay nop, not a compiler scheduling decision.
- verdict: KILLED

## [s2] the carrier (avail->ptr->word) can be pushed into $a1 by a structural spelling of the
##      SAME 4-allocno shape (merged carrier + count)
- mechanism: find_reg pass 0 excludes regs_someone_prefers; the count's $a1 copy-preference is
  what excludes $a1 from the carrier (FINDREGDBG, variant A: someone_prefers = {3,5})
- probe: variants A (param = count, fresh merged carrier), E (param = carrier, fresh count),
  B (block-local ptr/word), A2/A3 (if-arm statement re-association); all sandboxed + greg/lreg
  dumped
- result: every spelling gives the count an $a1 copy-preference — directly in A (param entry
  copy), by cse hard-reg propagation in E (which also deletes the carrier's entry copy, so the
  carrier ends up with NO preference at all). Scores 22/22/20/22/22 vs the 13 floor.
- verdict: KILLED for these five spellings; the family is not yet foreclosed (see frontier)

## [s2] source statement order inside the if-arm changes the schedule (and hence live lengths)
- mechanism: sched1 list scheduling from the dependence DAG
- probe: A2 (`r <<= n` after the s[2] store), A3 (`m2 = (1<<n)-1` after the word load)
- result: byte-identical to A (22 / 36 insns) — sched1 normalizes the block, text order is inert
- verdict: KILLED

## [s2] H7 OPEN (new frontier, replaces H5/H6): target seating needs TWO independent conditions
  (1) the count allocno must NOT hold an $a1 preference, or the carrier must hold one too
      (prune_preferences carve-out, global.c:918-928 — needs the carrier to be the param
      variable AND the param's incoming value to keep a use that cse cannot rewrite to hard $a1);
  (2) priority(s) > priority(count) in allocno_compare — measured 1.038 vs 1.125, so the count's
      live length must reach 26 (tie ⇒ allocno 72 wins) or its refs drop to 8. The target's
      if-arm has `sllv v1,v1,a3` LATE (after `lw a1,0(a1)`), i.e. its count lives ~3 insns
      longer than ours.
- next probe: attack (2) first, because it is measurable independently: change the DEPENDENCE
  GRAPH (not the text order) so the r<<=count shift cannot be scheduled early — e.g. make the
  shifted value depend on the word load (r built from the word-derived term first), or make the
  count's last use the OR/store. Re-read lreg's "used N times across M insns" for the count
  after each spelling; the goal is count-live-length >= s-live-length with refs unchanged.
  Only then re-attack (1).

## [s2] s1's H5: reproducing the target's 'unfilled load-delay slot' entry schedule (both entry copies above the avail lw) is the gating lever for target seating.
- mechanism: claimed sched1 tie between the n-copy and the avail lw, sinking the copy into the load-delay slot and keeping hard $a1 live past the load
- probe: read the .sched dump for block 0 (variant B) and the target asm at 8003D890-8003D898
- result: block 0's scheduling region is insns {12,15,16} only; the two entry copies (insns 4,6) sit above NOTE_INSN_FUNCTION_BEG and sched1 never moves them. The move that appears in the load-delay slot is placed by sched2/the delay-slot filler AFTER register allocation, so it cannot influence seats. The target's nop at 8003D894 is an assembler-inserted load-delay nop, not a compiler decision.
- verdict: KILLED

## [s2] The avail->ptr->word carrier can be driven into $a1 by restructuring which variable is the param and which is fresh (merged-carrier family).
- mechanism: global.c find_reg pass 0 ORs in regs_someone_prefers; the carrier is excluded from $a1 by the COUNT allocno's $a1 copy-preference
- probe: four spellings measured with sandbox --disable all plus greg/lreg/FINDREGDBG dumps: A (param=count, fresh merged carrier), E (param=carrier, fresh count), B (block-local ptr/word), 13-candidate
- result: A=22/36, E=22/36, B=20/36, candidate=13/37. Every spelling gives the COUNT the $a1 copy-preference: directly in A (param entry copy, greg '73 preferences: 5'), and in E by cse propagating hard $a1 into the fresh copy, which also deletes the carrier's entry copy so the carrier ends with no preference at all (greg '74 preferences: 5', none for 73). FINDREGDBG for variant A pseudo 74: conflicts {2,3,4,29}, someone_prefers {3,5}, pass0 succeeds at $a2 so the pass-1 fallback that ignores someone_prefers is never reached.
- verdict: KILLED

## [s2] Re-associating the if-arm statements changes the schedule and hence the allocno live lengths.
- mechanism: sched1 list scheduling from the dependence DAG + INSN_PRIORITY
- probe: A2 (r <<= n moved after the s[2] store) and A3 (m2 computed after the word load), both sandboxed
- result: both byte-identical to A (22 / 36 insns). sched1 normalizes the block from the dependence graph; source text order inside a basic block is inert here.
- verdict: KILLED

## [s2] Target seating requires TWO independent conditions, not one (this replaces s1's H5/H6 framing).
- mechanism: (1) prune_preferences/find_reg: the count must not hold an $a1 preference, or the carrier must hold one too (global.c:918-928 carve-out); (2) allocno_compare: priority(s) must exceed priority(count) so s takes $a2 before the count takes $a3
- probe: derived from the greg dispositions + lreg ref/live-length counts and checked against the target's own insn order
- result: measured priorities s = 9 refs / 26 insns = 1.038, count = 9 refs / 24 insns = 1.125, so the count is always ordered first and takes $a2. The target's if-arm places sllv v1,v1,a3 (r <<= count) LATE, after lw a1,0(a1), giving its count ~3 more live insns. H6 (the if-arm slot diffs) is therefore UPSTREAM of seating, not downstream of it as s1 assumed.
- verdict: CONFIRMED

## [s3] Source statement order / expression association inside the if-arm is inert (s2's conclusion)
- mechanism: s2 claimed sched1 rebuilds the block from the dependence DAG so text order cannot matter
- probe: read rank_for_schedule in tools/gcc-2.7.2/sched.c (third tie-break = INSN_LUID = original insn order); then measured A7/A8/A9/A10/A11/A12/A13/A14 plus a 24-variant cross product, reading REG_LIVE_LENGTH out of the .lreg dump for every one
- result: the association of the final OR changes the count allocno's live length from 25 to 27 and flips the allocno_compare order from `74 73 72 75` to the target's `74 72 73 75`; scores move 22 -> 21 -> 19 -> 15 across the family. Statement text order is a REAL lever whenever the competing insns tie on INSN_PRIORITY, which they all do here.
- verdict: KILLED (s2's inertness claim is false; the lever is live and was used to solve gate G2)

## [s3] The ptr/word carrier must be the same variable as avail (the merged-carrier family A/E)
- mechanism: s1/s2 assumed the target's $a1 holding avail then ptr then word implied ONE pseudo
- probe: built the alternative shape -- `avail` a global, ptr/word a fresh BLOCK-LOCAL `p` declared inside the if-arm -- and dumped lreg/greg/sched
- result: local_alloc seats `p` in hard $a1 inside block 1, hard $a1 becomes live inside the count's range, prune_preferences purges the count's $a1 copy-preference (the `;; 73 preferences: 5` line disappears), and the count moves to $a3 with s at $a2 in the b-family. Scores 15/19 vs the merged family's 22. The target's $a1 holds TWO different pseudos that happen not to overlap, not one merged carrier.
- verdict: KILLED (merged-carrier is the wrong shape; the block-local shape is the right one)

## [s3] Statement ORDER of the memory operations in the if-arm is an available lever
- mechanism: GCC 2.7.2's sched.c has no alias analysis, so loads and stores are mutually dependent and source order fixes their emitted order
- probe: enumerated every semantics-preserving permutation of {load s[1], load s[0], store s[0], load *ptr, store s[2], store s[1]} against the target's own emitted order
- result: the target's order IS our order; every alternative would emit a different load/store sequence and cannot match. The axis is closed by inspection, no measurement needed.
- verdict: KILLED

## [s3] H7a/H7b/H7c (s2 frontier) are all resolved or superseded
- H7a (push the count allocno's live length to >= 26 by changing the dependence graph): CONFIRMED, but the mechanism is the LUID tie-break on the final OR's association, not a dependence-graph change -- measured 25 -> 27, which flips the allocation order to the target's.
- H7b (the carrier keeps its own $a1 preference if it IS the param): SUPERSEDED. The carrier never needs its own preference; the correct fix is to delete the COUNT's preference by making the ptr/word value a block-local that local_alloc seats in hard $a1.
- H7c (the two gates may be separable): CONFIRMED -- they are separable, and separating them moved the structural score from 22 to 15.

## [s3] H8 OPEN (new frontier)
  The block-local-ptr shape solves G1 (count off $a1) and the OR association solves G2
  (allocation order 74 72 73 75).  What is left is G3: (a) the AVAIL allocno picks up a hard-$a1
  conflict because sched1 schedules the `p = s[0]` load at block-1 index 1, ahead of avail's last
  use (the `1 << avail` shift) at index 3, so avail cannot share $a1 with p the way the target
  does; and (b) the same OR association that fixes G2 makes local_alloc put the const-1 pseudo in
  $v1 rather than $a0, which pushes r off $v1.
- mechanism: global_conflicts records a hard-reg conflict for every global that is live while a
  local_alloc-assigned hard reg is live; the p load's sched1 position is INSN_PRIORITY-driven
  (its source position is byte-inert -- D1/D2 measured), so the fix must lower avail's death
  index rather than raise the load's index.
- next probe: (i) find a spelling in which avail's LAST use lands at block-1 index 0 or 1 -- e.g.
  give the `1 << avail` mask chain a dependence that raises its sched1 depth above the p load's,
  or express m1 so that its final insn is consumed earlier; verify by re-reading
  `;; 74 conflicts:` in the greg dump for the absence of 5.  (ii) decouple the local_alloc seating
  from the OR association: sweep spellings of the const-1 / m2 chain (named vs inlined, mask via
  a separate local, `1 << n` hoisted above the loads) inside the b-family and watch which
  block-local takes $v1 in the greg dispositions; the goal is b-family allocation ORDER with
  a-family local seating.  Start from memory/grind/func_8003D888/frontier_blocklocal_ptr_*.c and
  the 24-variant generator in tmp/grind/func_8003D888/s3/.

## [s3] s2's conclusion that source statement order / expression association inside the if-arm is codegen-inert holds in general for this function.
- mechanism: s2 argued sched1 rebuilds each basic block from the dependence DAG plus INSN_PRIORITY, so text order cannot survive. But rank_for_schedule (tools/gcc-2.7.2/sched.c) breaks ties on INSN_PRIORITY, then dependence class vs last_scheduled_insn, then INSN_LUID -- the original source insn order. Every scheduling decision in this if-arm is a priority tie, and sched1's output is what REG_LIVE_LENGTH (and hence allocno_compare) is computed from.
- probe: Read rank_for_schedule in full; then measured A7/A8/A9/A10/A11/A12/A13/A14 and a 24-variant cross product (OR association x m2 named/inlined x m1 before/after the subtract x s[1]=p before/after the OR) with `sandbox --disable all`, reading the count and s allocnos' 'used N times across M insns' out of the .lreg dump and the allocation order out of the .greg dump for each.
- result: The final OR's operand association moves the count allocno's live length from 25 to 27 with the insn stream otherwise unchanged, flipping the allocno_compare order from `74 73 72 75` to the target's `74 72 73 75`. Scores across the family: 22 -> 21 -> 19 -> 15.
- verdict: KILLED

## [s3] The target's $a1 (avail -> ptr -> word) is ONE merged carrier pseudo, so the correct C shape is the merged-carrier family A/E that s1 and s2 pursued.
- mechanism: s1/s2 inferred a single variable from a single hard register holding three values in sequence. The alternative is two pseudos with disjoint live ranges that local_alloc/global_alloc happen to seat in the same hard register.
- probe: Built the alternative shape -- `avail` stays a global, the ptr/word value is a fresh BLOCK-LOCAL `p` declared inside the if-arm -- and dumped .lreg/.greg/.sched for it (Bx, Bx2, Bx5-Bx7, C_* cross product).
- result: local_alloc seats `p` in hard $a1 inside block 1. Hard $a1 is then live inside the count allocno's range, so prune_preferences (global.c:908-910) PURGES the count's $a1 copy-preference -- the `;; 73 preferences: 5` line disappears from the greg dump entirely. The count lands in $a3 (target) and the s2 3-cycle seat rotation is eliminated. Scores 15 (a-family) / 19 (b-family) vs the merged family's 22.
- verdict: KILLED

## [s3] Reordering the memory operations in the if-arm (the s[0]/s[1]/s[2] loads and stores) is an available structural lever.
- mechanism: GCC 2.7.2's sched.c has no alias analysis, so all loads and stores in a block are mutually dependent and their source order fixes their emitted order.
- probe: Enumerated every semantics-preserving permutation of {load s[1], load s[0], store s[0], load *ptr, store s[2], store s[1]} and checked each against the target's own emitted order in asm/funcs/func_8003D888.s.
- result: The target's memory order IS our current order (load s[1], load s[0], store s[0], load *ptr, store s[2], store s[1]). Every alternative source order would emit a different load/store sequence and therefore cannot match. Axis closed by inspection, no measurement needed.
- verdict: KILLED

## [s3] H7a (s2): the count allocno's live length can be pushed to >= 26 so that allocno_compare orders s ahead of it.
- mechanism: allocno_compare priority = floor_log2(refs)*refs/live_length; with refs pinned at 9 for both s and the count by the target's own insn stream, s (live length 26) can only win if the count's live length reaches 26 (tie -> lower allocno number 72 = s wins).
- probe: Measured live lengths for every variant in the A* and C_* sweeps out of the .lreg dump.
- result: CONFIRMED, and achieved: writing the final OR as `(masked_word) | (r << n)` gives the count live length 27 > s's 26, and the .greg dump then reports the target allocation order `74 72 73 75` with s in $a2 (6) and the count in $a3 (7). The mechanism is the LUID tie-break, not a dependence-graph change as s2 predicted.
- verdict: CONFIRMED
