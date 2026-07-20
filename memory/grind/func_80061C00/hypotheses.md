# Hypothesis ledger — func_80061C00

## s1 (recon)

- H1 **CONFIRMED (mechanism):** the distance-2 diff = arg1's a1→v1 copy sunk by sched1's `adjust_priority` birthing boost (single-set + live pseudo → LAUNCH_PRIORITY → placed at block-0 bottom in the backward scheduler). Proven by cc1 -dS dump + sched.c read + two probes that defeated the boost and landed the copy at target slot 3.
- H2 **KILLED:** K&R-style definition changes the narrow-param copy RTL → no, byte-identical.
- H3 **KILLED:** dead self-assign `arg1 = arg1;` creates a second set → no, cse/flow delete it before sched1; closes the dead-store-FAKE road for this mechanism.
- H4 **KILLED:** live param-reuse (arg1 = D_800F1164+2 pointer holder; two spellings) → copy position fixed but RA gives $a3 + address-fold / rotation damage (28, 21).
- H5 **KILLED:** live param-reuse (arg1 = sp20[1] repack temp) → copy position fixed but repack temp loses its own boost; sh reorders, pointer lui hoists, $v1/$a0/$a1 rotate (12).

## Frontier (for s2+)

1. **Inside-window second set.** A second LIVE set of arg1 whose entire lifetime sits inside the entry→`sh 0x1A` window (before the first jal), so segment 2 never exists and RA keeps $v1. No natural value found yet in s1 — search for a spelling where the second set is the SAME sh consumer path (e.g. staging another to-be-stored s16 through arg1 BEFORE the calls — the only pre-call sh's are the sp10/sp18 constants; probe `arg1 = 0xA00`-style constant staging is a real live store but changes the li→move bytes; measure before dismissing).
2. **Permuter campaign** seeded from the distance-2 baseline (fresh-seed discipline, PERM statement/type variants). The space of natural respellings around the clamp + array inits is small; watch specifically for any form whose prologue has `move v1,a1` at slot 3 without post-call rotation.
3. **Sched2-side lever.** After reload the boost is off; sched2 already lifts the copy one slot. Look for a C shape that changes block-0 dependence structure so sched2 (not sched1) can lift it two more slots — e.g. forms that give the lui/addiu chain an earlier consumer, pushing it up and letting the copy trail at slot 3. Un-probed in s1.

## [s1] The whole distance-2 diff is the arg1 param copy (move v1,a1) sunk from prologue slot 3 to after the lui/addiu pair
- mechanism: GCC 2.7.2 sched.c backward list scheduler: adjust_priority boosts birthing insns (SET of live, reg_n_sets==1 pseudo) to LAUNCH_PRIORITY 0x7f000001 -> picked early backward = placed at block-0 bottom; arg2's copy escapes via the clamp's second set
- probe: cc1 -dS sched dump (block 0 trace + post-sched RTL) + sched.c:187/2534/2566 read; diagnostic multi-set probes
- result: Dump shows insn 8 (set reg:HI 73 <- a1) at 0x7f000001 in ready list, sinking; both multi-set probes moved the copy to exact target slot 3
- verdict: CONFIRMED

## [s1] K&R-style definition changes the narrow-param copy RTL enough to alter its scheduling
- mechanism: old-style promotion path could emit SI->HI narrowing instead of a plain HI copy
- probe: K&R definition, sandbox --disable all
- result: byte-identical output, distance still 2
- verdict: KILLED

## [s1] A dead self-assign (arg1 = arg1;) in the clamp arm creates the second set that defeats the boost
- mechanism: reg_n_sets[73] would become 2 -> birthing_insn_p false
- probe: if (arg2 != 1) { arg2 = 0; arg1 = arg1; }, sandbox
- result: byte-identical: cse/flow delete the no-op move before sched1 counts reg_n_sets; dead-store-FAKE road mechanically closed for this diff
- verdict: KILLED

## [s1] Live reuse of arg1 (s32) as the D_800F1164+2 pointer holder double-sets the pseudo and matches, since target's v1 later holds that pointer
- mechanism: multi-set kills the boost; RA hoped to assign the merged pseudo v1
- probe: arg1 = (s32)(D_800F1164+2) with u8* accesses (score 28) and with original spellings + single read-back (score 21)
- result: copy lands at target slot 3 but pseudo allocates $a3; +1 offsets fold to %lo(sym+3) / post-call regs rotate
- verdict: KILLED

## [s1] Live reuse of arg1 as the sp20[1] repack temp (the value that occupies v1 in target) double-sets without touching the pointer temp
- mechanism: same multi-set lever, second value chosen to coincide with target's v1 occupancy
- probe: arg1 = sp20[1]; sp10[1] = arg1; sandbox
- result: score 12: copy at slot 3 but repack temp loses ITS boost -> sh reorders, pointer lui hoists into arg1 live range, $v1/$a0/$a1 rotate
- verdict: KILLED

## s2 (structural)

- H6 **CONFIRMED (partial) / KILLED as 0-path:** arg1 (s32) second set via arm-1 D_800A3468 reload defeats the boost and lands the copy at exact slot 3 with perfect schedule — but alone leaks 0x10016 into $v1 (local_alloc precedence), score 7.
- H7 **CONFIRMED attractor / KILLED as 0-path:** + val = {sp20[2] repack, arm-1 0x10016} rescues all registers -> score 2 (NEW residual: lw v1/lw a0 swap). The swap is proven immovable inside this shape: sched1 pick LAUNCH-v1lw > prio1-a0lw regardless of source order (variant A byte-identical).
- H8 **KILLED (frontier #3):** sched2-side lift impossible — rank_for_schedule ties = descending post-sched1 LUID; sched2 is deterministic given sched1 output (sched.c:2455 + hazard bumps only for memory-unit insns; a move can't be bumped).
- H9 **KILLED:** naming the D+2 pointer (t carrier, (u8*)t derefs) -> cse const-folds derefs to absolute addressing, 31 (+4 insns).
- H10 **KILLED:** val second set = (s32)(D_800F1164+3) in arm 2 -> cse-derivable from live 91 -> substituted+deleted, n_sets collapses, regression to 7. General law: second sets survive only for non-derivable values.
- H11 **KILLED:** second carrier u = {(s32)sp10, 0x10017} -> RA priority inversion (u=8000 before val) + sched hoist -> u steals $a0, full rotation, 17.
- H12 **KILLED (analytic):** pre-call in-window second set of arg1 — no li-consuming store exists after sh 0x1A; interleaved $v0/$v1 values can't share one pseudo.

## Frontier (for s3+)

1. **Permuter campaign, TWO seeds:** baseline attractor (block-0 sink) and s2 candidate (lw-swap). Small mutation space around each; watch for any form where the three repack loads emit v0,v1,a0 with val's carrier intact — or a boost-symmetric respelling nobody derived.
2. **Break the lw-swap coupling:** find a spelling where sp20[1]'s load is ALSO unboosted with target-consistent RA. Its only $v1-second-set hosts are exhausted; unexplored direction: restructure the repack so sp20[1]'s value flows WITHOUT its own pseudo (e.g. through arg2/s1 staging is semantically blocked; maybe an (s16)-typed access pattern changing the load's mode/insn shape so birthing_insn_p sees a non-plain-REG dest — STRICT_LOW_PART shapes via union/s16-view of a s32 local were NOT probed).
3. **Re-examine whether the ORIGINAL needed the val-carrier at all:** maybe the original's arm-1 spelling avoids a const temp entirely (e.g. a different assignment order that lets reload_cse-era mechanics or operand shapes reuse a reg). Un-probed: arm-1 store order `*(s32*)arg1 = 0x10016;` FIRST with base+0x14 second (stores to provably-distinct offsets CAN reorder in sched1 — same-base const offsets are true_dependence-provable-distinct); measure whether sched1 re-derives target order with the const temp born earlier (longer local range -> different local-alloc priority -> maybe const gets a0 naturally).

- H13 **KILLED (measured):** arm-1 source order with `val = 0x10016` hoisted before the +0x14 store — byte-identical (score 2); arm-1 internal statement order is sched1-invariant.
- Frontier-2 sub-note (analytic kill): STRICT_LOW_PART/subreg-dest shapes cannot unboost the sp20[1] load — `lw` is a full-word load whose RTL dest is inherently a plain REG; partial-write shapes change the opcode (lh/lhu). The permuter seeds are the live remainder of frontier 2.

## [s2] arg1 (retyped s32, byte-neutral) second-set via arm-1 D_800A3468 reload defeats the birthing boost and places move v1,a1 at target slot 3
- mechanism: reg_n_sets>=2 kills birthing_insn_p; the arm-1 lw is the only unclaimed target $v1-write; second segment is short and gap-dead so live_length/priority stay healthy
- probe: retype + arg1 = D_800A3468 in the 0x10016 arm; sandbox + objdump
- result: Schedule PERFECT (every position matches target) but score 7: local_alloc hands the vacated arm-1 $v1 to the 0x10016 const temp, global arg1 -> $a3
- verdict: KILLED

## [s2] Adding val = {sp20[2] repack, arm-1 0x10016} globalizes the const past local_alloc and restores all target registers
- mechanism: val is the only $a0-targeted load host; global allocno ordering (allocno_compare: floor_log2(refs)*refs/len*10000) puts 91->v1, val->a0, arg1->v1, 89->a1, 94->a0 per conflict walk
- probe: sandbox + objdump + ALLOCDBG (tmp/gccdbg/cc1)
- result: Score 2: ALL registers and both arms match target; sole residual = lw v1,0x24/lw a0,0x28 swap (val's load lost its LAUNCH boost)
- verdict: CONFIRMED

## [s2] The lw swap can be fixed by source order of the repack statements
- mechanism: rank_for_schedule ties resolve by luid only below priority; v1lw keeps LAUNCH
- probe: val-load hoisted first (variant A), measured
- result: Byte-identical: sched1 pick {v1lw(LAUNCH) vs a0lw(1)} ignores luid; swap immovable inside this shape
- verdict: KILLED

## [s2] sched2 can lift the copy independently of sched1 (inherited frontier #3)
- mechanism: post-reload boost is off; hoped block-0 dependence reshaping gives the lui chain an earlier consumer
- probe: cc1 -dR sched2 dump + sched.c rank_for_schedule read + simulation
- result: sched2 ties = descending post-sched1 LUID (sched.c:2455); output fully determined by sched1 order + memory-unit hazard bumps (moves can't be bumped); no independent lever exists
- verdict: KILLED

## [s2] Naming the D_800F1164+2 pointer into a C variable unboosts the sp20[1] load symmetrically
- mechanism: t = {sp20[1], (s32)(D+2)} would double-set both problem pseudos
- probe: variant F, sandbox
- result: Score 31 (+4 insns): cse const-folds every ((u8*)t)[k] deref to absolute %lo addressing; the anonymous compound spelling is load-bearing
- verdict: KILLED

## [s2] val's second set can live in arm 2 as val=(s32)(D_800F1164+3), keeping all repack loads boosted
- mechanism: both sets outside the post-call block
- probe: variant G, sandbox + sched2 RTL dump
- result: Score 7 regression: D+3 is cse-derivable from live pseudo 91 -> use substituted to derived temp, set flow-deleted (flow.c:1284 rebuilds reg_n_sets post-DCE), n_sets collapses to 1
- verdict: KILLED

## [s2] A second carrier u = {(s32)sp10, 0x10017} unboosts the addiu stealing a0lw's window
- mechanism: vacate the C6 scheduling window for a0lw
- probe: variant H, sandbox + ALLOCDBG
- result: Score 17: u pri 8000 allocated before val, sched1 hoists u's addiu into the lw/sh overlap -> v0 blocked -> u steals $a0 -> full register rotation
- verdict: KILLED

## [s2] A pre-call in-window second set of arg1 exists (inherited frontier #1 narrow form)
- mechanism: post-sh-0x1A pre-call stores are all $zero stores; interleaved $v0 constants cannot share arg1's single pseudo/register
- probe: target byte census (asm/funcs/func_80061C00.s)
- result: No candidate instruction exists; analytically impossible
- verdict: KILLED

## [s2] arm-1 internal statement order (const set before/after ptr store) changes bytes
- mechanism: distinct const offsets are true_dependence-provably distinct
- probe: measured swap
- result: Byte-identical, score 2 either way
- verdict: KILLED
