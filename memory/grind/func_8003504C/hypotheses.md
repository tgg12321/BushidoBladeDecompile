# hypotheses — func_8003504C

Floor history: s1 baseline 24 → 24 (no improvement; four hypotheses measured,
three killed, one CONFIRMED-as-mechanism but overshooting).

## KILLED in session 1

### H1 — the `i`/`src` register inversion is driven by init statement order
Mechanism claimed: pseudo creation / LUID order from the order the two locals
are first assigned. Probe: swapped `i = 0;` and `src = (u8 *)p;`.
**Result: floor 24 → 25, allocation UNCHANGED** (`src` still in $a3; the only
change was the emitted order of the three `move`s). KILLED — statement order
does not touch the allocation, and moving `src`'s init earlier costs 1.
Rejected form: `rejected/init-order-swap-src-before-i.c`.

### H1' — ...driven by DECLARATION order
Mechanism claimed: GCC 2.7.2 `expand_decl` creates pseudos in declaration
order, so declaring `u8 *src;` before `s32 i;` makes `src`'s allocno number
lower and wins the priority tie. Probe: swapped the two declarations.
**Result: floor stays exactly 24 and the normalized diff is byte-identical to
baseline** (all 26 positions identical) — the swap is completely codegen-neutral
here. KILLED. Note the allocno order is decided by `reg_n_refs / live_length`
(measured 7 vs 5 refs, see evidence.md), not by declaration order, so there was
no tie to break in the first place.
Rejected form: `rejected/decl-order-swap-src-before-i.c`.

### H2 — cluster 2 serializes if both extractions go through one shared temp
Mechanism claimed: reusing one C local for both `p[5]>>4` and `p[8]>>3` creates
an anti-dependence on a single pseudo, denying sched1 the freedom to interleave
the two chains, which would reproduce target's strictly-serialized $v0-only
form. Probe: added `u32 fld;` and wrote `fld = (u32)p[5] >> 4; D_80102784 =
fld & 0x3F; D_800A36F6 = 0; fld = (u32)p[8] >> 3; D_80102786 = fld & 1;`.
**Result: floor stays 24; cluster 2 RESHAPED but no closer** — the shared
variable got its own hard reg ($a0) and the block became
`lui at; sh zero; srl a0,v0,4; lw v0,0x20(t3); andi v1,a0,63; ...`, i.e. the
chains are still interleaved and now a third register is involved. KILLED as
stated: cc1 does not keep the reuse as one serializing pseudo here (CSE/copy
propagation splits it), so the anti-dependence never materializes.
Rejected form: `rejected/shared-temp-fld-both-extractions.c`.

### H4 — shortening `src`'s live range raises its priority enough to flip
Mechanism: priority is `reg_n_refs / live_length`; with refs fixed at 5, making
`src`'s live range shorter raises its ratio above `i`'s 7/L. Probe: moved
`src = (u8 *)p;` to be the LAST of the five init statements (after `base` and
`ptr`), shortening its range by the base/ptr computation.
**Result: floor 24 → 26, no allocation flip.** KILLED — the achievable
shortening (a handful of insns out of a ~40-insn range) is far too small to
overcome a 7-vs-5 ref gap, and the reordered init costs 2.
Rejected form: `rejected/src-init-last.c`.

## CONFIRMED (mechanism), needs calibration — the live frontier

### H3 — the cluster-1 flip is a global.c allocno-priority problem, and it IS reachable
Mechanism: `global.c` sorts allocnos by `reg_n_refs / live_length` and hands
out hard regs in the MIPS allocation order, so `src` takes $a2 (and `i` takes
$a3, as target wants) as soon as `src` outranks `i`. Because `src`'s refs
cannot easily be raised, the lever is **lowering `i`'s ratio by extending its
live range** (`reg_n_refs` in `flow.c` is incremented by loop depth, so where
the extra refs land matters as much as how many there are).

Probe run: reused `i` as loop 2's OUTER counter (deleted `s32 j`, used `i` for
the `do { ... } while (i < 5)` in the `val == 5` branch — genuine reuse of a
dead scratch local, no dead store).
**Result: floor 24 → 28, but the intended flip HAPPENED** — positions 12, 38
and 39 went clean, i.e. `src` landed in **$a2 exactly as target wants**. The
cost is that `i`'s priority fell too far: the new order became
`ptr=$a1, src=$a2, base=$a3, i=$t0`, whereas target wants
`ptr=$a1, src=$a2, i=$a3, base=$t0`. So the extension over-shot by exactly one
position — `i` dropped below `base` (3 refs, short range) instead of landing
between `src` and `base`.
Rejected-as-committed form (kept for its measurement value, NOT to re-run):
`rejected/reuse-i-as-loop2-outer-counter.c`.

**This is the highest-value frontier item.** The target ordering is one
discrete step away from a measured, reproducible configuration.

## [s1] The loop-1 i/src register inversion is driven by the order the two locals are first assigned (init statement order).
- mechanism: Pseudo creation / LUID order from first assignment feeding the allocno ordering.
- probe: Swapped `i = 0;` and `src = (u8 *)p;` in the init block; sandbox --disable all + normalized position diff.
- result: Floor 24 -> 25. Allocation UNCHANGED (src still $a3, i still $a2); the only delta was the emitted order of the three pre-loop `move`s.
- verdict: KILLED

## [s1] The inversion is driven by DECLARATION order (declaring `u8 *src` before `s32 i` lowers its allocno number and wins the priority tie).
- mechanism: GCC 2.7.2 expand_decl creates pseudos in declaration order; global.c breaks priority ties by allocno number.
- probe: Swapped the two declarations; sandbox --disable all + normalized position diff.
- result: Floor stays exactly 24 and the normalized diff is byte-identical to baseline (all 26 differing positions unchanged) - completely codegen-neutral. There is no tie to break: measured reg_n_refs are i=7 vs src=5, so the ratio ordering decides it outright.
- verdict: KILLED

## [s1] Cluster 2 (the p[5]/p[8] bitfield extractions) serializes into target's $v0-only form if both extractions are staged through one shared temp local.
- mechanism: Reusing one C local gives one pseudo, whose anti-dependence denies sched1 the freedom to interleave the two independent chains.
- probe: Added `u32 fld;` and rewrote as `fld = (u32)p[5] >> 4; D_80102784 = fld & 0x3F; D_800A36F6 = 0; fld = (u32)p[8] >> 3; D_80102786 = fld & 1;`.
- result: Floor stays 24; cluster 2 reshaped but not closer. cc1 did not keep the reuse as a single serializing pseudo - the shared variable got its own hard reg ($a0) and the block came out `lui at; sh zero; srl a0,v0,4; lw v0,0x20(t3); andi v1,a0,63; ...`, still interleaved, now across three registers.
- verdict: KILLED

## [s1] Shortening `src`'s live range (assigning it last, after base/ptr) raises its reg_n_refs/live_length ratio above `i`'s and flips the allocation.
- mechanism: global.c allocno_compare orders by reg_n_refs / live_length.
- probe: Moved `src = (u8 *)p;` to be the last of the five init statements; sandbox --disable all + normalized position diff.
- result: Floor 24 -> 26, no allocation flip. The achievable shortening (a few insns out of a ~40-insn range) cannot overcome a 7-vs-5 ref gap, and the reordered init costs 2.
- verdict: KILLED

## [s1] Cluster 1's inversion is a global.c allocno-priority problem and is reachable by lowering `i`'s reg_n_refs/live_length ratio below `src`'s via a live-range extension.
- mechanism: global.c sorts allocnos by reg_n_refs/live_length and hands out hard regs in the MIPS allocation order; flow.c weights reg_n_refs by loop depth. Extending i's live range past loop 1 drops its ratio, so src takes the earlier register ($a2).
- probe: Reused `i` as loop 2's outer counter (deleted `s32 j`, ran the `val == 5` do/while on `i`) - a genuine reuse of a dead scratch local, no dead store; sandbox --disable all + normalized position diff.
- result: Floor 24 -> 28 as a closing form, BUT the intended flip happened: positions 12/38/39 went clean and `src` landed in $a2 exactly as target wants. The extension overshot by one position - got ptr=$a1, src=$a2, base=$a3, i=$t0 where target wants ptr=$a1, src=$a2, i=$a3, base=$t0 (i fell below `base`, 3 refs / short range, instead of landing between src and base).
- verdict: CONFIRMED

## [s2] H3-calibration: reusing `i` as loop 2's INNER counter is the "one discrete position weaker" live-range extension that lands ptr=$a1, src=$a2, i=$a3, base=$t0.
- mechanism: global.c allocno priority = floor_log2(refs)*refs/live_length; flow.c weights refs by loop depth, so refs placed in a DEEPER loop were expected to raise the numerator while the live range grew by a similar span, giving a smaller net ratio drop than session 1's outer-loop reuse.
- probe: Replaced loop 2's inner counter `s32 k` with `i` (vA, tmp/grind/func_8003504C/s2/vA_i_as_inner.c); sandbox --disable all plus the new refs/live_length + allocno-order + disposition instrument.
- result: Floor 24 -> 39 and the allocation moved the WRONG WAY. The inner loop sits at loop depth 3, so i's weighted refs went 11 -> 19 while its live length only went 37 -> 52: priority ROSE from 0.892 to 0.365*floor_log2 (19 refs -> 4*19/52 = 1.46), putting i FIRST and pushing ptr off $a1 (got i=$a1, ptr=$a2, src=$a3). Deeper reuse raises priority; it cannot lower it.
- verdict: KILLED

## [s2] The cluster-1 inversion is reachable at all through allocno priority (src outranking i) under the fixed 141-instruction stream.
- mechanism: allocno_compare = floor_log2(refs)*refs/live_length. Target's asm pins the original's occurrence counts to i=11 and src=9 weighted refs (5 vs 4 in-loop occurrences at depth 2, plus one pre-loop init at depth 1), and the LICM-hoisted li 5 / li 20 prove loop 1 had loop notes in the original. So src outranks i only if L_i > (33/27)*L_src = 1.222*L_src.
- probe: Measured refs/live_length for every pseudo across four variants (baseline; src-init-last; goto-spelled loop 1 with baseline init order; goto-spelled loop 1 with src-init-last), plus BB2_FINDREG_DEBUG dumps for pseudos 73 and 74.
- result: i and src are both loop-carried and therefore live across the entire loop body, so their lengths can differ only by the pre-loop distance between their initializations - and the whole pre-loop block is 8 insns. Measured extremes: baseline 37/36 = 1.028, src-init-last 37/34 = 1.088, both far below 1.222. Removing the loop notes (if/goto spelling) rescales both sides identically (i=6, src=5 refs, same floor_log2 bucket 2), leaving the requirement at 1.2 and additionally demoting ptr off $a1. KILLED - the axis has no reachable position, which retires session 1's H3 frontier.
- verdict: KILLED

## [s2] The inversion can be steered through find_reg instead of priority (make i skip $a2 via a hard-reg conflict or a someone_prefers bit).
- mechanism: find_reg's pass 0 excludes registers preferred by other allocnos and registers never used so far; pass 1 takes the first non-conflicting reg in REG_ALLOC_ORDER.
- probe: BB2_FINDREG_DEBUG=73 / =74 on the instrumented cc1 (tools/gcc-2.7.2/cc1) for the HEAD form.
- result: i's hard-reg conflicts are exactly {$v0,$v1,$a0,$a1} and its someone_prefers set is EMPTY, so $a2 is simply the first free register. A someone_prefers bit for $a2 would require a copy between a pseudo and an argument register, and this function has no such copy at all (both calls are 0-arg). A hard conflict on $a2 would require an earlier-allocated conflicting pseudo to hold $a2, but every pseudo live anywhere in loop 1's body conflicts with src exactly as much as with i, so it cannot separate them. KILLED.
- verdict: KILLED

## [s2] Loop 1's walking pointers were loop.c strength-reduction givs in the original (pure i-indexed source), and that different pseudo provenance changes the i/src allocation.
- mechanism: loop.c reduces a multi-use `base[i*K]` address expression to a walking pointer while leaving single-use i-indexed arrays as lui+addu, which would explain why target walks $a2 over p and $a1 over D_8010277C but keeps lui+addu+%lo for D_80102780 / D_8010277E.
- probe: vE (both walkers as givs, all four arrays i-indexed) and vF (only the p-walk as a giv, HEAD's base/ptr kept); sandbox + the disposition instrument on each.
- result: vE = 142 insns / score 32 - one insn OVER target, because target derives the D_8010277C pointer as `addiu $a1,$t0,-0x9` from the D_80102785 address while a giv needs its own lui+addiu; that is positive evidence that the original DID have HEAD's `base` / `ptr = base - 9` source relationship. vF = 141 insns / score 26 with the identical inversion: the giv is created late and allocated after i, so i still takes $a2. KILLED as an allocation lever (kept as evidence about ptr's provenance).
- verdict: KILLED

## [s2] Reusing `i` as loop 2's INNER counter is the one-discrete-position-weaker live-range extension that lands ptr=$a1, src=$a2, i=$a3, base=$t0 (session 1's frontier probe).
- mechanism: global.c allocno priority = floor_log2(refs)*refs/live_length, and flow.c increments reg_n_refs by loop_depth, so refs placed in a deeper loop were expected to raise the numerator while the live range grew by a comparable span, giving a smaller net ratio drop than session 1's outer-loop reuse.
- probe: vA (tmp/grind/func_8003504C/s2/vA_i_as_inner.c): replaced loop 2's inner counter `s32 k` with `i`; sandbox --disable all plus the new refs/live_length + allocno-order + register-disposition instrument.
- result: Floor 24 -> 39, and the allocation moved the WRONG WAY: the inner loop is at depth 3, so i's weighted refs went 11 -> 19 while its live length only went 37 -> 52; its priority ROSE (4*19/52 = 1.46 vs 0.892), putting i FIRST and displacing ptr off $a1 (i=$a1, ptr=$a2, src=$a3). Deeper reuse raises priority and can never lower it.
- verdict: KILLED

## [s2] The cluster-1 inversion is reachable at all through allocno priority (getting src allocated before i) under the fixed 141-instruction stream.
- mechanism: allocno_compare = floor_log2(refs)*refs/live_length. Target's own asm pins the original compile's weighted ref counts to i=11 and src=9 (5 vs 4 in-loop occurrences at loop_depth 2, plus one pre-loop init at depth 1), and the LICM-hoisted `li $t2,5` / `li $t1,20` prove loop 1 carried loop notes in the original, so the depth-2 weighting applied there too. src outranks i only if L_i > (33/27)*L_src = 1.222*L_src.
- probe: Measured refs/live_length + allocno order + dispositions for four variants: baseline HEAD, src-init-last (do/while), goto-spelled loop 1 with baseline init order, goto-spelled loop 1 with src-init-last.
- result: Both i and src are loop-carried and therefore live across the whole loop body, so their live lengths can differ only by the pre-loop distance between their two initializations - and the entire pre-loop block is 8 insns. Measured extremes: baseline L_i/L_src = 37/36 = 1.028; src-init-last = 37/34 = 1.088; both far below the 1.222 requirement. Removing the loop notes (if/goto spelling) rescales both sides identically (refs i=6, src=5, same floor_log2 bucket 2), leaving the requirement at 1.2, and additionally demotes ptr off $a1 (p's allocno overtakes it). The axis has no reachable position - this retires session 1's H3 frontier.
- verdict: KILLED

## [s2] The inversion can be steered through find_reg instead of priority - make i skip $a2 via a hard-reg conflict or a regs_someone_prefers bit, leaving $a2 for src.
- mechanism: find_reg's pass 0 excludes registers preferred by other allocnos and registers not yet used; pass 1 takes the first non-conflicting register in REG_ALLOC_ORDER.
- probe: BB2_FINDREG_DEBUG=73 and =74 on the INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1, not build/cc1) over the HEAD form; script tmp/grind/func_8003504C/s2/findreg.sh.
- result: i's hard-reg conflicts are exactly {$v0,$v1,$a0,$a1} and its someone_prefers set is EMPTY, so $a2 is simply the first free register in REG_ALLOC_ORDER. Populating someone_prefers requires a copy between a pseudo and an argument register, and this function has none (both calls are 0-arg). A hard conflict on $a2 requires an earlier-allocated conflicting pseudo to hold $a2, but every pseudo live in loop 1's body conflicts with src exactly as much as with i, so no pseudo can separate them.
- verdict: KILLED

## [s2] Loop 1's walking pointers were loop.c strength-reduction givs in the original (i.e. the original source was purely i-indexed), and that different pseudo provenance changes the i/src allocation.
- mechanism: loop.c reduces a multi-use base[i*K] address to a walking pointer while leaving single-use i-indexed arrays as lui+addu+%lo - which would explain why target walks $a2 over p and $a1 over D_8010277C but keeps lui+addu for D_80102780 / D_8010277E.
- probe: vE (all four arrays i-indexed, both walkers as givs) and vF (only the p-walk as a giv, HEAD's base/ptr kept); sandbox --disable all + the disposition instrument on each.
- result: vE = 142 insns / score 32 - one insn OVER target, because target derives the D_8010277C pointer as `addiu $a1,$t0,-0x9` from the D_80102785 address while a giv needs its own lui+addiu; that is positive evidence the original had HEAD's `base` / `ptr = base - 9` source relationship. vF = 141 insns / score 26 with the identical inversion: the giv is created late by loop.c and allocated after i, so i still takes $a2. Killed as an allocation lever, kept as provenance evidence for ptr.
- verdict: KILLED
