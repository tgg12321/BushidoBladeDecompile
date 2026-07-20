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
