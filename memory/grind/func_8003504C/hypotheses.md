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

## [s3] Cluster 2 closes on the STATEMENT ORDER of the three global stores plus the D_80102785 read (session 2's top frontier item).
- mechanism: sched1 fills the load-delay region of `lw $v0,0x20($t3)` with a later-in-program-order independent instruction, which is where target's `sh zero` (D_800A36F6 = 0) sits; and target's early `lui $v1` / `lb $v1` is the D_80102785 read being available early enough to fill the `lw 0x14` delay region instead of the p[8] load being hoisted there. rank_for_schedule breaks INSN_PRIORITY ties by INSN_LUID (original program order), so source order was the lever.
- probe: EXHAUSTIVE - generated and measured all 24 permutations of the four mutually independent statements (D_80102784 store / D_800A36F6 = 0 / D_80102786 store / val = D_80102785) with sandbox --disable all, plus the normalized position diff on representatives of both resulting shapes. Instruments: tmp/grind/func_8003504C/s3/gen_perm.py, sweep_score.sh, sweep_diff.sh.
- result: The final asm takes exactly TWO shapes and neither is closer than HEAD. The six orders with the val read LAST (ABCD ACBD BACD BCAD CABD CBAD) are 141 insns / score 24 with a diff BYTE-IDENTICAL to HEAD's, cluster 2 included - permuting the three stores among themselves is completely codegen-neutral. Every order with the val read earlier is 143 insns / score 24-27. Reading the RTL dumps shows why: sched1 interleaves the two extraction chains identically under every source order, so cluster 2's residual is a priority effect, not a LUID/program-order effect, and the LUID tie-break never gets to run.
- verdict: KILLED

## [s3] The `val = D_80102785;` read can be moved earlier (to give its `lb` a low LUID like target's position 51-52) if `val` is widened from s8 to s32, removing the QImode-pseudo sign-extension penalty.
- mechanism: with `s8 val`, combine folds an adjacent read into one extendqisi2_insn (`lb`); moved away from the compare, val stays a QImode pseudo and cc1 emits lbu + sll 24 + sra 24 (+2 insns). An s32 declaration makes the read a sign_extend(mem:QI) at RTL-generation time, which should be one `lb` at any distance.
- probe: v_s32val_{ABCD,DABC,ADBC,ABDC}.c - `s32 val` with the read first, second, third and last; sandbox --disable all + the position diff.
- result: The 2-insn penalty does disappear (all four at 141 insns, score 24), but the load itself becomes WRONG: cc1 emits `lbu` where target has `lb`, because val's only uses are equality compares against 2 and 5 and combine narrows the sign_extend to a zero_extend. Cluster 2's shape was otherwise unchanged (same 26 differing positions). KILLED - and the failure is positive evidence that the original declared val as a signed 8-bit local read LAST.
- verdict: KILLED

## [s3] The pre-loop constants `li 5` / `li 20` sit early in target because the base/ptr address setup was hoisted by loop.c AFTER them (i.e. the original computed that address inside loop 1, not in a pre-loop local).
- mechanism: loop.c hoists loop invariants into the preheader in discovery order, so an address computed inside the loop lands after the constants hoisted from the loop condition; sched1 then breaks the priority tie among these zero-dependent insns by INSN_LUID, reproducing target's order.
- probe: vP3_direct_globals.c - dropped the `base` / `ptr` locals and spelled the D_8010277C-area accesses as direct i-indexed globals (`(&D_8010277C)[i]`, `D_80102785`) inside loop 1, matching how HEAD already spells the D_80102780 / D_8010277E accesses; sandbox --disable all + the position diff over the pre-loop window.
- result: The hoist-order mechanism is CONFIRMED - the constants did move ahead of the address setup (li at positions 7,8 with lui/addiu at 9,10). But the form scores 30 / 44 differing positions because it loses target's `addiu $a1,$t0,-9` derivation (the address becomes an independent lui/addiu pair), which is a second independent corroboration - after session 2's giv result - that the original had HEAD's `base` / `ptr = base - 9` relationship. And it does NOT reproduce target's other half: target has `move $t3,$v0` and `move $a2,$t3` after the constants too. KILLED as a closing form; kept as mechanism evidence.
- verdict: KILLED

## [s3] Cluster 2 closes on the STATEMENT ORDER of the three global stores plus the D_80102785 read (session 2's top frontier item).
- mechanism: sched1 fills the load-delay region of `lw $v0,0x20($t3)` with a later-in-program-order independent instruction (target puts the `sh zero` for D_800A36F6 there), and target's early `lui $v1` / `lb $v1` is the D_80102785 read filling the `lw 0x14` delay region instead of the p[8] load being hoisted there. rank_for_schedule (tools/gcc-2.7.2/sched.c) breaks INSN_PRIORITY ties by INSN_LUID = original program order, so source order was the lever.
- probe: EXHAUSTIVE: generated and measured all 24 permutations of the four mutually independent statements (D_80102784 store / D_800A36F6 = 0 / D_80102786 store / val = D_80102785) with `sandbox func_8003504C --disable all`, plus the normalized position diff on representatives of both resulting shapes. Instruments tmp/grind/func_8003504C/s3/gen_perm.py + sweep_score.sh + sweep_diff.sh.
- result: The final asm takes exactly TWO shapes and neither is closer than HEAD. The six orders with the val read LAST (ABCD ACBD BACD BCAD CABD CBAD) are 141 insns / score 24 with a normalized diff BYTE-IDENTICAL to HEAD's, cluster 2 included — permuting the three stores among themselves is completely codegen-neutral. Every order with the val read earlier is 143 insns / score 24-27. The RTL dumps show why: sched1 interleaves the two extraction chains identically under every source order, so the residual is a PRIORITY effect and the LUID tie-break never gets to run.
- verdict: KILLED

## [s3] The `val = D_80102785;` read can be moved earlier (giving its `lb` a low LUID like target's position 51-52) if `val` is widened from s8 to s32, removing the QImode-pseudo sign-extension penalty.
- mechanism: With `s8 val`, combine folds a read adjacent to its use into one extendqisi2_insn (`lb`); moved away from the compare, val stays a QImode pseudo and cc1 emits lbu + sll 24 + sra 24 (+2 insns). An s32 declaration makes the read a sign_extend(mem:QI) at RTL-generation time, which should be one `lb` at any distance.
- probe: v_s32val_{ABCD,DABC,ADBC,ABDC}.c — `s32 val` with the read first, second, third and last; sandbox --disable all plus the position diff.
- result: The 2-insn penalty does disappear (all four at 141 insns, score 24), but the load itself becomes wrong: cc1 emits `lbu` where target has `lb`, because val's only uses are equality compares against 2 and 5 and combine narrows the sign_extend to a zero_extend. Cluster 2's shape was otherwise unchanged (same 26 differing positions). The failure is positive evidence that the original declared val as a signed 8-bit local read LAST.
- verdict: KILLED

## [s3] The pre-loop constants `li 5` / `li 20` sit early in target because the base/ptr address setup was hoisted by loop.c AFTER them — i.e. the original computed that address inside loop 1 rather than in a pre-loop local.
- mechanism: loop.c hoists loop invariants into the preheader in discovery order, so an address computed inside the loop lands after the constants hoisted from the loop condition; sched1 then breaks the priority tie among these zero-dependent insns by INSN_LUID, reproducing target's order.
- probe: vP3_direct_globals.c — dropped the `base` / `ptr` locals and spelled the D_8010277C-area accesses as direct i-indexed globals (`(&D_8010277C)[i]`, `D_80102785`) inside loop 1, matching how HEAD already spells D_80102780 / D_8010277E; sandbox --disable all + the position diff over the pre-loop window.
- result: The hoist-order mechanism is CONFIRMED — the constants moved ahead of the address setup (li at positions 7,8, lui/addiu at 9,10). But the form scores 30 with 44 differing positions because it loses target's `addiu $a1,$t0,-9` derivation (the address becomes an independent lui/addiu pair) — a second independent corroboration, after session 2's giv result, that the original had HEAD's `base` / `ptr = base - 9` relationship. It also fails to reproduce target's other half: target has `move $t3,$v0` and `move $a2,$t3` after the constants too. Killed as a closing form; kept as mechanism evidence.
- verdict: KILLED

## [s4] Cluster 2's sched1 interleave is broken by staging the p[8] read through a named intermediate, and the intermediate must be a POINTER.
- mechanism: sched1 hoists the p[8] load above the D_80102784 store because nothing keeps a value live across the first extraction chain. Taking the ADDRESS of the eighth word (`q = &p[8]`) creates a pointer pseudo that is live across that chain, so the load cannot be hoisted; the two chains stop interleaving, their live ranges stop overlapping, and lreg reuses $v0 for the second chain instead of allocating $v1 — target's strictly-serialized block. A VALUE temp does not survive: combine/CSE folds `v8 = p[8]; ... v8 >> 3` straight back into the original shift, leaving the RTL byte-identical to the inline spelling.
- probe: decomp-permuter random campaign ws1 (base score 1120) surfaced the family within 15 s across four spellings; each was written out as a full variant and measured with `sandbox func_8003504C --disable all`: v_sval_ABCV (`s32 v8`), v_uval_ABCV (`u32 v8`), v_ptr_ABCV (`s32 *q`), v_reusei_ABCV (`i = p[8]`).
- result: `s32 v8` -> 24 (no change), `u32 v8` -> 24 (no change), `i = p[8]` -> 20, `q = &p[8]` -> **17** (floor lowered for the first time since session 1). All four at 141/141 instructions. CONFIRMED for the pointer spelling; the two value-temp spellings are banked as rejected/p8-value-temp-folded-back-by-cse.c.
- verdict: CONFIRMED

## [s4] With the pointer intermediate present, cluster 2's residual DOES respond to statement order (session 3's exhaustive sweep was chassis-specific, not a closure).
- mechanism: session 3 measured all 24 orders on the INLINE-p[8] chassis and found two output shapes, concluding the residual was a pure INSN_PRIORITY effect immune to program order. But order and priority are not independent: once the pointer intermediate removes the interleave, the remaining freedom is which independent store fills the `lw 0x20($t3)` load-delay region, and that IS decided by LUID (program order) among priority-tied insns.
- probe: all six orders of the four post-loop statements that keep `val = D_80102785;` legal, on the pointer chassis (v_ptr_{ABCV,AVBC,ACBV,ABVC,AVCB,ACVB}.c), each measured with `sandbox --disable all`.
- result: A-B-C-val 17, **A-C-B-val 13**, A-val-C-B 16, A-C-val-B 16, A-val-B-C 19 (143 insns), A-B-val-C 19 (143 insns). The winner puts `D_800A36F6 = 0;` BETWEEN the two bitfield extractions and keeps the `val` read last. At 13, cluster 2 has ZERO differing positions — the block matches target instruction for instruction, register for register.
- verdict: CONFIRMED

## [s4] Cluster 1's i/src register inversion flips if `src` (dead after loop 1) is reused to carry loop 2's D_801027D8 destination pointer.
- mechanism: extending `src`'s live range past loop 1 into the `val == 5` branch changes its allocno ranking relative to `i`, so `src` is allocated first and takes $a2 (the first free register in REG_ALLOC_ORDER) while `i` inherits the conflict and takes $a3 — target's assignment. This is the same class of lever as session 1's H3 (live-range extension) but applied to `src` instead of `i`, and it does NOT overshoot: `base` keeps $t0 and `ptr` keeps $a1.
- probe: permuter campaign ws2 (base score 175) surfaced it at 132 s as `src = &D_801027D8; ... u8 *dst_d = src;`; rebuilt as tmp/grind/func_8003504C/s4/v_reuse_src_dstd.c and measured with `sandbox --disable all` plus tmp/grind/func_8003504C/s2/probe.sh for the dispositions.
- result: Floor 13 -> **4**, 141/141 instructions, 7 differing positions. Dispositions `72 in 11 / 73 in 7 / 74 in 6 / 75 in 5 / 76 in 8` = p=$t3, i=$a3, src=$a2, ptr=$a1, base=$t0 — target exactly. Positions 4, 12, 38, 39, 41, 44, 46, 47 all clean.
- verdict: CONFIRMED

## [s4] Session 2's arithmetic closure of the cluster-1 allocation axis ("src can never outrank i, the axis has no reachable position") is correct.
- mechanism: session 2 modelled allocno_compare as `floor_log2(refs)*refs/live_length` with flow.c's loop-depth ref weighting, measured i=11 refs and src=9 refs with both live across the whole loop body, and derived a requirement of L_i > 1.222*L_src against a maximum achievable 1.088.
- probe: measured refs/live_length + the post-qsort allocno order + the register dispositions on the session-4 winning form (tmp/grind/func_8003504C/s2/probe.sh), i.e. on a form where the flip demonstrably HAPPENED.
- result: DISPROVEN. On the winning form i has refs=11 len=37 (ratio 0.297, model priority 0.892) and src has refs=11 len=43 (ratio 0.256, model priority 0.767) — the model still predicts i first — yet `.greg`'s post-qsort order is `... 75 74 73 ...` (src BEFORE i) and src takes $a2. Either `allocno_live_length` differs from the `.flow` "used R times across L insns" figure the probe parses, or another allocno_compare term dominates. The axis was reachable all along; the closure was a modelling artefact. Do not close an allocation axis on that ratio alone again.
- verdict: KILLED

## [s4] The pre-loop rotation (the two LICM-hoisted constants moving from last to first) falls out of decomp-permuter random search on the floor-4 chassis.
- mechanism: random source mutation around a chassis whose only residual is a 7-insn intra-block rotation should be able to find whatever spelling gives the constants a higher INSN_PRIORITY or a lower LUID, the same way random search found the three levers that took 24 -> 4.
- probe: campaign ws3 `reuse-src-chassis-floor4` (base permuter score 120), -j 8, --stop-on-zero, 20373 iterations over 610 s of wall clock, harvested and stopped in-session.
- result: NO find below the base score at all (one score-120 tie). The rotation does not fall out of random search on this chassis. Killed as a permuter target; the next attack must be directed at WHY the constants sit last — they are LICM-hoisted to the END of the preheader and have no in-block dependents, so they carry the lowest INSN_PRIORITY in a block where `move t3,v0 -> move a2,t3` is a 2-deep chain.
- verdict: KILLED

## [s4] Cluster 2's sched1 interleave is broken by staging the p[8] read through a named intermediate, and the intermediate must be a POINTER.
- mechanism: sched1 hoists the p[8] load above the D_80102784 store because nothing keeps a value live across the first extraction chain. Taking the ADDRESS of the eighth word (q = &p[8]) creates a pointer pseudo live across that chain, so the load cannot be hoisted; the chains stop interleaving, their live ranges stop overlapping, and lreg reuses $v0 for the second chain instead of allocating $v1 - target's strictly-serialized block. A VALUE temp does not survive: combine/CSE folds `v8 = p[8]; ... v8 >> 3` straight back into the original shift.
- probe: decomp-permuter random campaign ws1 (base permuter score 1120) surfaced the family within 15 s across four spellings; each was written out as a full variant and measured with `sandbox func_8003504C --disable all`: v_sval_ABCV (s32 v8), v_uval_ABCV (u32 v8), v_ptr_ABCV (s32 *q), v_reusei_ABCV (i = p[8]).
- result: s32 v8 -> 24 (no change), u32 v8 -> 24 (no change), i = p[8] -> 20, q = &p[8] -> 17. All four at 141/141 instructions. The pointer spelling lowered the floor for the first time since session 1; the two value-temp spellings are banked as rejected/p8-value-temp-folded-back-by-cse.c.
- verdict: CONFIRMED

## [s4] With the pointer intermediate present, cluster 2's residual DOES respond to statement order - session 3's exhaustive 24-permutation sweep was chassis-specific, not a closure.
- mechanism: Session 3 concluded the residual was a pure INSN_PRIORITY effect immune to program order. But order and priority are not independent: once the pointer intermediate removes the interleave, the remaining freedom is which independent store fills the `lw 0x20($t3)` load-delay region, and among priority-tied insns rank_for_schedule breaks the tie on INSN_LUID = program order.
- probe: All six orders of the four post-loop statements that keep the `val = D_80102785;` read legal, on the pointer chassis (v_ptr_{ABCV,AVBC,ACBV,ABVC,AVCB,ACVB}.c), each measured with `sandbox --disable all`.
- result: A-B-C-val 17, A-C-B-val 13, A-val-C-B 16, A-C-val-B 16, A-val-B-C 19 (143 insns), A-B-val-C 19 (143 insns). The winner puts `D_800A36F6 = 0;` BETWEEN the two bitfield extractions and keeps the val read last (session 3's `s8 val` read-last finding is unchanged). At 13, cluster 2 has ZERO differing positions - it matches target instruction for instruction and register for register.
- verdict: CONFIRMED

## [s4] Cluster 1's i/src register inversion flips if `src` (dead after loop 1) is reused to carry loop 2's D_801027D8 destination pointer.
- mechanism: Extending src's live range past loop 1 into the `val == 5` branch changes its allocno ranking relative to i, so src is allocated first and takes $a2 (the first free register in REG_ALLOC_ORDER) while i inherits the conflict and takes $a3 - target's assignment. Same class of lever as session 1's H3 live-range extension, but applied to src rather than i, and it does not overshoot: base keeps $t0 and ptr keeps $a1.
- probe: Permuter campaign ws2 (base score 175) surfaced it at 132 s as `src = &D_801027D8; ... u8 *dst_d = src;`; rebuilt as tmp/grind/func_8003504C/s4/v_reuse_src_dstd.c and measured with `sandbox --disable all` plus tmp/grind/func_8003504C/s2/probe.sh for the register dispositions.
- result: Floor 13 -> 4, 141/141 instructions, 7 differing positions. Dispositions `72 in 11 / 73 in 7 / 74 in 6 / 75 in 5 / 76 in 8` = p=$t3, i=$a3, src=$a2, ptr=$a1, base=$t0 - target exactly. Positions 4, 12, 38, 39, 41, 44, 46 and 47 all went clean in a single step.
- verdict: CONFIRMED

## [s4] Session 2's arithmetic closure of the cluster-1 allocation axis ("src can never outrank i; the axis has no reachable position") is correct.
- mechanism: Session 2 modelled allocno_compare as floor_log2(refs)*refs/live_length with flow.c's loop-depth ref weighting, measured i=11 and src=9 weighted refs with both live across the whole loop body, and derived a requirement L_i > 1.222*L_src against a maximum achievable ratio of 1.088.
- probe: Measured refs/live_length, the post-qsort allocno order and the register dispositions (tmp/grind/func_8003504C/s2/probe.sh) on the session-4 winning form - i.e. on a form where the flip demonstrably HAPPENED.
- result: DISPROVEN. On the winning form i has refs=11 len=37 (model priority 0.892) and src has refs=11 len=43 (model priority 0.767), so the model still predicts i first - yet .greg's post-qsort allocno order is `... 75 74 73 ...` (src BEFORE i) and src takes $a2. Either allocno_live_length differs from the .flow "used R times across L insns" figure the probe parses, or another allocno_compare term dominates. The axis was reachable all along; the closure was a modelling artefact.
- verdict: KILLED

## [s4] The remaining pre-loop rotation (the two LICM-hoisted constants moving from last to first) falls out of decomp-permuter random search on the floor-4 chassis.
- mechanism: Random source mutation around a chassis whose only residual is a 7-insn intra-block rotation should be able to find whatever spelling gives the constants a higher INSN_PRIORITY or a lower LUID - the same way random search found the three levers that took the floor from 24 to 4.
- probe: Campaign ws3 `reuse-src-chassis-floor4` (base permuter score 120), -j 8, --stop-on-zero, 20373 iterations over 610 s of wall clock, harvested and stopped in-session.
- result: No find below the base score at all (one score-120 tie). The rotation does not fall out of random search on this chassis. The next attack must be directed at WHY the constants sit last, not sampled for.
- verdict: KILLED

## [s5] The pre-loop rotation is reachable by a DIRECTED permuter chassis over the pre-loop init statement order and the order of loop 1's four condition disjuncts (session 4's top frontier item).
- mechanism: rank_for_schedule breaks INSN_PRIORITY ties by INSN_LUID, so the order of the five pre-loop init statements sets the block's LUIDs; and the order of the four disjuncts in `(u32)(lv-3) < 2 || (s8)lv == 5 || (u32)(lv-18) < 2 || (s8)lv == 20` sets loop.c's invariant DISCOVERY order and hence the order the two constants are appended to the preheader. Session 4's undirected random search on this chassis found nothing in 20373 iterations, but neither axis had been swept on the floor-4 chassis.
- probe: built workspace tmp/grind/func_8003504C/s5/ws5 from the floor-4 base with tmp/grind/func_8003504C/s5/inject.py - PERM_LINESWAP over `p = func_80077D00(); i = 0; base = &D_80102785;`, a second PERM_LINESWAP over `src = (u8 *)p; ptr = (u8 *)(base - 9);`, and a PERM_GENERAL over all 24 permutations of the four disjuncts - and ran campaign `preloop-directed-lineswap-disjuncts` (base permuter score 120).
- result: The perm space is finite (6 x 2 x 24 = 288) and the permuter ENUMERATED ALL 288 combinations in 14 s with ZERO finds below base. Both axes are dead. The reason is now understood: GCC 2.7.2 sched.c gives every latency-1 insn priority 1 ("hence no scheduling will be done"), so the block is emitted in pure RTL order - and no permutation of PRE-LOOP statements can move insns that loop.c appends AFTER all of them.
- verdict: KILLED

## [s5] The pre-loop block's order is a sched1 INSN_PRIORITY effect that a source spelling can steer.
- mechanism: session 4 recorded the two constants as having "the lowest INSN_PRIORITY in the block" because they have no in-block dependents, while `move t3,v0 -> move a2,t3` is a 2-deep chain - implying that giving the constants an in-block dependent, or the copies a shallower chain, would rotate the block.
- probe: read the priority computation in tools/gcc-2.7.2/sched.c (the `prev_priority = priority (x) + insn_cost (x, prev, insn) - 1;` block and its comment) and rank_for_schedule's tie-break chain, then checked the prediction against every measured variant's emitted pre-loop order.
- result: DISPROVEN as a priority effect. The subtracted one exists precisely so that "when all instructions have a latency of 1 ... all instructions will end up with a priority of one, and hence no scheduling will be done". Every insn in this block is a latency-1 move / lui / addiu, so all priorities tie at 1, the dependence-class test ties too, and rank_for_schedule falls through to INSN_LUID: the block is emitted in PURE RTL ORDER. Every measured variant's pre-loop order is exactly its RTL order, with no exception. The residual is an RTL EMISSION-ORDER problem, not a scheduling problem.
- verdict: KILLED

## [s5] Target's pre-loop order is produced by loop 1's walking pointers being loop.c strength-reduction GIVs rather than pre-loop pointer locals, because giv initial values (and the call-result copy) are emitted at loop_start AFTER loop.c's movables.
- mechanism: three RTL emission classes land in this block - pre-loop source statements (lowest LUIDs, first), loop.c movables emitted by move_movables immediately before NOTE_INSN_LOOP_BEG (after every pre-loop statement, in discovery order), and loop.c giv initial values emitted at loop_start after the movables. Target's block is `i=0 / li 5 / li 20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9` = one class-1 statement, then the movables, then the giv inits - so only the counter init was a pre-loop statement.
- probe: generated the walker-provenance cross-product with tmp/grind/func_8003504C/s5/gen5.py and measured each with `sandbox --disable all` plus the normalized position diff over the pre-loop window: v_ptr_src_reuse (candidate), v_ptr_src_plain, v_ptr_idx_plain, v_idx_src_reuse, v_idx_src_plain, v_idx_idx_plain, plus v_idx85_idx_plain.
- result: CONFIRMED as the ordering mechanism. v_ptr_idx_plain (p walker as a giv, score 15) moves BOTH p-copies from the front of the block to behind the two constants. v_idx_src_reuse (D_8010277C walker as a giv, score 10) lifts the constants ahead of the address setup but not ahead of the p-copies. v_idx_idx_plain (both walkers as givs, score 18, 141/141) reproduces target's pre-loop block ORDER exactly - the first form in five sessions to do so. It is not a closing form: the `addiu a1,t0,-9` derivation collapses to 2 insns under cse and cluster 1's register inversion returns for want of a `src` local to carry session 4's live-range lever.
- verdict: CONFIRMED

## [s5] On the giv chassis, writing the D_8010277C walker as `(&D_80102785)[i - 9]` makes cse materialize %hi/%lo(D_80102785) into a register and derive the walker as `-9`, reproducing target's three-insn `lui t0 / addiu t0 / addiu a1,t0,-9`.
- mechanism: target keeps &D_80102785 live in $t0 across loop 1 (the guard reads `lb v0,0(t0)`) and derives the walker's initial value from it; expressing the walker relative to that same symbol should give cse one address to materialize and one register-plus-offset derivation.
- probe: tmp/grind/func_8003504C/s5/v_idx85_idx_plain.c - both walkers as givs, the store spelled `(&D_80102785)[i - 9] = lv;` and the guard `D_80102785 == 0`; `sandbox --disable all` plus the position diff.
- result: KILLED. cse folds the -9 into the %lo of the giv's own initial address, emitting `lui a1 / addiu a1,a1,-9` - still two insns, no separate base register - and the in-loop guard then gets its OWN lui. Score 18, byte-identical pre-loop shape to the plain giv chassis. Banked as rejected/giv-walker-based-on-D80102785-cse-folds-the-minus9.c.
- verdict: KILLED

## [s5] The giv chassis is a productive basin for undirected permuter search (it might recover the register assignment and the -9 derivation on its own).
- mechanism: the giv chassis has target's pre-loop order and only two localized gaps, so random source mutation around it should be able to find the spelling that keeps &D_80102785 in a register and restores cluster 1's allocation - the same way random search found the three levers that took the floor from 24 to 4.
- probe: campaign ws6 `giv-chassis-both-walkers-random` on tmp/grind/func_8003504C/s5/ws6 (base permuter score 495), -j 8, --stop-on-zero, 10255+ iterations over ~500 s, harvested and stopped in-session.
- result: KILLED as a closing chassis. Four novel finds (495 / 450 / 495 / 465); the best, 450, is far above the floor-4 chassis's base of 120, so the basin is not competitive. Its proposals were constant-holder reuses of the dead `i` / `tmp` locals (`i = 5;` then `p[i]`, `tmp = 5;` then `(s8)lv == tmp`) - a different lever, not adopted here.
- verdict: KILLED

## [s5] The pre-loop rotation is reachable by a DIRECTED permuter chassis over the pre-loop init statement order and the order of loop 1's four condition disjuncts (session 4's top frontier item).
- mechanism: rank_for_schedule breaks INSN_PRIORITY ties by INSN_LUID, so the order of the pre-loop init statements sets the block's LUIDs; and the order of the four disjuncts in `(u32)(lv-3) < 2 || (s8)lv == 5 || (u32)(lv-18) < 2 || (s8)lv == 20` sets loop.c's invariant DISCOVERY order and hence the order the two constants are appended to the preheader. Session 4's undirected random search on this chassis found nothing in 20373 iterations, but neither axis had been swept on the floor-4 chassis.
- probe: Built workspace tmp/grind/func_8003504C/s5/ws5 from the floor-4 base with tmp/grind/func_8003504C/s5/inject.py (PERM_LINESWAP over `p = func_80077D00(); i = 0; base = &D_80102785;`, a second PERM_LINESWAP over `src = (u8 *)p; ptr = (u8 *)(base - 9);`, and a PERM_GENERAL over all 24 permutations of the four disjuncts) and ran campaign `preloop-directed-lineswap-disjuncts` at base permuter score 120, harvested and stopped in-session.
- result: The perm space is finite (6 x 2 x 24 = 288) and the permuter ENUMERATED ALL 288 combinations in 14 s with ZERO finds below base. Both axes are dead, and the reason is now understood: no permutation of PRE-LOOP statements can move insns that loop.c appends AFTER all of them.
- verdict: KILLED

## [s5] The pre-loop block's order is a sched1 INSN_PRIORITY effect that a source spelling can steer.
- mechanism: Session 4 recorded the two hoisted constants as carrying the lowest INSN_PRIORITY in the block (no in-block dependents) while `move t3,v0 -> move a2,t3` is a 2-deep chain, implying that giving the constants an in-block dependent would rotate the block.
- probe: Read the priority computation in tools/gcc-2.7.2/sched.c (the `prev_priority = priority (x) + insn_cost (x, prev, insn) - 1;` block and its comment) plus rank_for_schedule's tie-break chain, and checked the prediction against the emitted pre-loop order of all seven variants measured this session.
- result: DISPROVEN as a priority effect. The subtracted one exists precisely so that 'when all instructions have a latency of 1 ... all instructions will end up with a priority of one, and hence no scheduling will be done'. Every insn in this block is a latency-1 move / lui / addiu, so all priorities tie at 1, the dependence-class test ties too, and rank_for_schedule falls through to INSN_LUID. Every measured variant's pre-loop order is exactly its RTL order, with no exception. The residual is an RTL EMISSION-ORDER problem, not a scheduling problem.
- verdict: KILLED

## [s5] Target's pre-loop order is produced by loop 1's walking pointers being loop.c strength-reduction GIVs rather than pre-loop pointer locals, because giv initial values (and the call-result copy) are emitted at loop_start AFTER loop.c's movables.
- mechanism: Three RTL emission classes land in this block: pre-loop source statements (lowest LUIDs, first), loop.c movables emitted by move_movables immediately before NOTE_INSN_LOOP_BEG (after every pre-loop statement, in discovery order), and loop.c giv initial values emitted at loop_start after the movables. Target's block is `i=0 / li 5 / li 20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9` = one class-1 statement, then the movables, then the giv inits.
- probe: Generated the walker-provenance cross-product with tmp/grind/func_8003504C/s5/gen5.py and measured each with `sandbox func_8003504C --disable all` plus the normalized position diff over the pre-loop window: v_ptr_src_reuse (the session-4 candidate), v_ptr_src_plain, v_ptr_idx_plain, v_idx_src_reuse, v_idx_src_plain, v_idx_idx_plain, plus v_idx85_idx_plain.
- result: CONFIRMED as the ordering mechanism. Scores, all at 141/141 instructions: 4 / 13 / 15 / 10 / 19 / 18 / 18. v_ptr_idx_plain (p walker as a giv) moves BOTH p-copies from the front of the block to behind the two constants; v_idx_src_reuse (D_8010277C walker as a giv) lifts the constants ahead of the address setup but not ahead of the p-copies; v_idx_idx_plain (both walkers as givs) reproduces target's pre-loop block ORDER exactly - the first form in five sessions to do so. It is not a closing form: the `addiu a1,t0,-9` derivation collapses to two insns under cse, and cluster 1's register inversion returns for want of a `src` local to carry session 4's live-range lever.
- verdict: CONFIRMED

## [s5] On the giv chassis, writing the D_8010277C walker as `(&D_80102785)[i - 9]` makes cse materialize %hi/%lo(D_80102785) into a register and derive the walker as `-9`, reproducing target's three-insn `lui t0 / addiu t0 / addiu a1,t0,-9`.
- mechanism: Target keeps &D_80102785 live in $t0 across loop 1 (the guard reads `lb v0,0(t0)`) and derives the walker's initial value from it; expressing the walker relative to that same symbol should give cse one address to materialize and one register-plus-offset derivation.
- probe: tmp/grind/func_8003504C/s5/v_idx85_idx_plain.c - both walkers as givs, the store spelled `(&D_80102785)[i - 9] = lv;` and the guard `D_80102785 == 0`; sandbox --disable all plus the normalized position diff.
- result: KILLED. cse folds the -9 into the %lo of the giv's own initial address, emitting `lui a1 / addiu a1,a1,-9` - still two insns, no separate base register - and the in-loop guard then gets its own lui. Score 18, byte-identical pre-loop shape to the plain giv chassis.
- verdict: KILLED

## [s5] The giv chassis is a productive basin for undirected permuter search (it might recover the register assignment and the -9 derivation on its own).
- mechanism: The giv chassis has target's pre-loop order and only two localized gaps, so random source mutation around it should be able to find the spelling that keeps &D_80102785 in a register and restores cluster 1's allocation - the way random search found the three levers that took the floor from 24 to 4.
- probe: Campaign ws6 `giv-chassis-both-walkers-random` on tmp/grind/func_8003504C/s5/ws6 (base permuter score 495), -j 8, --stop-on-zero, 10255+ iterations over ~500 s, harvested and stopped in-session.
- result: KILLED as a closing chassis. Four novel finds (495 / 450 / 495 / 465); the best, 450, is far above the floor-4 chassis's base of 120, so the basin is not competitive. Its proposals were constant-holder reuses of the dead `i` / `tmp` locals (`i = 5;` then `p[i]`, `tmp = 5;` then `(s8)lv == tmp`) - a different lever, not adopted here.
- verdict: KILLED

## [s6] The pre-loop block is emitted in pure RTL order and sched1 has nothing to do with it (session 5's closure).
- mechanism: session 5 read GCC 2.7.2 sched.c's priority computation, saw that the subtracted one makes every latency-1 insn tie at priority 1, and concluded rank_for_schedule falls through to INSN_LUID and therefore emits the block in RTL order.
- probe: compared the .loop and .sched cc1 -da dumps of the pre-loop block on BOTH chassis (tmp/grind/func_8003504C/s6/d_floor4, d_giv) with tmp/grind/func_8003504C/s3/rtlblock.py, and re-ran the block through the instrumented cc1 with BB2_RANK_DEBUG=1 on a new SOLO TU (one function, so uids are unambiguous).
- result: DISPROVEN as stated. sched1 reorders on both chassis: floor-4 goes `11 14 17 20 23 342 344` -> `14 11 17 20 23 342 344`, and the giv chassis goes `11 14 347 349 355 379` -> `14 347 349 11 355 379`, i.e. the call-result copy is sunk below BOTH hoisted constants. The tie-break IS LUID (every RANKDBG line in this block reads cls=3 x cls2=3 val=0), but only among insns READY that cycle, and insn 11 alone carries a true dependence on the call. Session 5's practical conclusion survives - the movables are still appended after every pre-loop source statement, so no floor-4-chassis spelling can lift them ahead of the base address setup - but the stated mechanism was wrong and must not be re-used to close a scheduling question.
- verdict: KILLED

## [s6] The giv chassis's missing &D_80102785 base register is recovered by assigning the guard pointer INSIDE loop 1's body, where loop.c sees it as an invariant MOVABLE rather than a pre-loop source statement.
- mechanism: three RTL emission classes decide the preheader's order (pre-loop source statements, then loop.c movables, then giv initial values). A pre-loop `base = &D_80102785;` is class 1 and emits ahead of the hoisted constants, destroying target's order; the same assignment written inside the loop body is loop-invariant, so move_movables hoists it into the preheader as class 2 - after the constants - and the pseudo stays live across the loop so the guard can dereference it at offset 0.
- probe: tmp/grind/func_8003504C/s6/gen6.py generated four placements on the session-5 giv chassis (b first in the body, b after the `lv` load, b inside the guarded arm, b as a pre-loop local), each measured with `sandbox func_8003504C --disable all` plus the normalized position diff.
- result: CONFIRMED. Score 18 -> 13 at 141/141 for the in-body placements (14 at 142 insns for the placement inside the guarded arm; 13 for the pre-loop local too, but only the in-body form keeps target's block order). The emitted preheader becomes `move a3,zero / li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0` - target's, through position 10 - and the in-loop guard reads `lb v0,0(t0)`.
- verdict: CONFIRMED

## [s6] The remaining `addiu a1,t0,-9` is recovered by giving the D_8010277C walker giv an initial value that IS the -9 subtraction, i.e. a second in-loop invariant `w = (u8 *)b - 9` walked as `w[i]`.
- mechanism: loop.c's emit_iv_add_mult materializes a giv's initial-value expression at loop_start. With the walker written `((u8 *)b)[i - 9]` the -9 is part of the giv's INDEX, so the init is a bare copy (`move a1,t0`) and every access carries -9 in its memory offset. Making `b - 9` its own in-loop invariant moves the subtraction into the giv's initial value, which is exactly target's third preheader insn.
- probe: tmp/grind/func_8003504C/s6/gen6b.py variant h1 (`s8 *b = &D_80102785; u8 *w = (u8 *)b - 9; ... w[i] = lv; if (*b == 0) w[i] = w[i] - 3;`), measured with `sandbox --disable all` and the position diff.
- result: CONFIRMED. Score 13 -> 9 at 141/141 instructions, and the preheader is now target's in full: `move a3,zero / li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9`. All nine remaining positions are the single i/p-walker register swap. Banked as memory/grind/func_8003504C/chassis_h1_score9.c.
- verdict: CONFIRMED

## [s6] Session 4's src-reuse allocation lever transplants onto the giv chassis if `b` / `w` are hoisted to function scope so one of them can carry loop 2's D_801027D8 destination.
- mechanism: session 4 flipped the i/walker allocation by extending a dead loop-1 pointer's live range into the `val == 5` branch. On the giv chassis the only source-visible pointers left in loop 1 are the two in-loop invariants, so hoisting one to function scope should give the lever something to attach to.
- probe: gen6b.py variants h2 (reuse `w`), h3 (reuse `b`, with the -9 fold) and h4 (reuse `b`, without it); `sandbox --disable all` on each.
- result: KILLED. 35 / 50 / 51 at 121 / 118 / 116 instructions against target's 141 - the function COLLAPSES. A function-scope pointer assigned inside loop 1 is no longer a loop-invariant movable, so the whole emission-class structure the chassis depends on is lost and loop 2's code folds. The lever cannot be spelled this way.
- verdict: KILLED

## [s6] On the h1 chassis the i/p-walker inversion flips by lowering `i`'s allocno rank through a live-range extension (session 1's H3 lever, re-aimed at the new chassis).
- mechanism: .greg on h1 ranks the source pseudo 73 (`i`) ahead of every loop.c-created giv pseudo, so `i` takes $a2 as the first free register in REG_ALLOC_ORDER and the p-walker giv inherits $a3. Extending `i` past loop 1 should drop it below a giv.
- probe: three reuses of the dead loop-1 counter measured with `sandbox --disable all`: as loop 2's outer counter (v_i1), as the val==5 table index (v_i2 - the mildest possible extension, one extra ref outside any loop), and as loop 2's inner counter (v_i3).
- result: KILLED. 33 (136 insns) / 26 (141 insns) / 58 (122 insns) - every extension moves AWAY from target and even the mildest costs 17 points. On this chassis the allocation lever is not lowering `i`; it must be raising a giv's rank, or eliminating `i` as a source pseudo altogether.
- verdict: KILLED

## [s6] The pre-loop block is emitted in pure RTL order and sched1 has nothing to do with it (session 5's stated closure).
- mechanism: Session 5 read GCC 2.7.2 sched.c's priority computation, saw that the subtracted one makes every latency-1 insn tie at priority 1, and concluded rank_for_schedule falls through to INSN_LUID and therefore emits the block in RTL order.
- probe: Compared the .loop and .sched cc1 -da dumps of the pre-loop block on BOTH chassis (tmp/grind/func_8003504C/s6/d_floor4, d_giv) with s3/rtlblock.py, and re-ran the block through the instrumented cc1 with BB2_RANK_DEBUG=1 on a new SOLO TU (one function per TU, so insn uids are unambiguous).
- result: DISPROVEN as stated. sched1 reorders on both chassis: floor-4 goes 11 14 17 20 23 342 344 -> 14 11 17 20 23 342 344, and the giv chassis goes 11 14 347 349 355 379 -> 14 347 349 11 355 379 (the call-result copy sunk below BOTH hoisted constants). The tie-break IS LUID - every RANKDBG line in this block reads cls=3 x cls2=3 val=0 - but only among insns READY that cycle, and insn 11 alone carries a true dependence on the call (insn_list 9 in LOG_LINKS) while i=0 and the constants carry only REG_DEP_ANTI 9. Session 5's practical conclusion survives (movables are still appended after every pre-loop source statement) but its mechanism must not be reused to close a scheduling question.
- verdict: KILLED

## [s6] The giv chassis's missing &D_80102785 base register is recovered by assigning the guard pointer INSIDE loop 1's body, where loop.c sees it as an invariant MOVABLE rather than a pre-loop source statement.
- mechanism: Three RTL emission classes decide the preheader's order: pre-loop source statements, then loop.c movables (move_movables, immediately before NOTE_INSN_LOOP_BEG), then giv initial values at loop_start. A pre-loop base = &D_80102785 is class 1 and emits ahead of the hoisted constants; the same assignment written inside the loop body is loop-invariant, so it is hoisted as class 2 - after the constants - and its pseudo stays live across the loop so the guard can dereference it at offset 0.
- probe: tmp/grind/func_8003504C/s6/gen6.py generated four placements on the session-5 giv chassis (b first in the body, b after the lv load, b inside the guarded arm, b as a pre-loop local); each measured with sandbox --disable all plus the normalized position diff.
- result: CONFIRMED. Score 18 -> 13 at 141/141 for the in-body placements (14 at 142 insns for the guarded-arm placement; 13 for the pre-loop local as well, but only the in-body form keeps target's block order). The preheader becomes move a3,zero / li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 - target's, through position 10 - and the in-loop guard reads lb v0,0(t0) exactly like target.
- verdict: CONFIRMED

## [s6] The remaining addiu a1,t0,-9 is recovered by giving the D_8010277C walker giv an initial value that IS the -9 subtraction, i.e. a second in-loop invariant w = (u8 *)b - 9 walked as w[i].
- mechanism: loop.c's emit_iv_add_mult materializes a giv's initial-value expression at loop_start. Written ((u8 *)b)[i - 9] the -9 is part of the giv's INDEX, so the init is a bare copy (move a1,t0) and every access carries -9 in its memory offset. Making b - 9 its own in-loop invariant moves the subtraction into the giv's initial value, which is exactly target's third preheader insn.
- probe: tmp/grind/func_8003504C/s6/gen6b.py variant h1 (s8 *b = &D_80102785; u8 *w = (u8 *)b - 9; ... w[i] = lv; if (*b == 0) w[i] = w[i] - 3;), measured with sandbox --disable all and the position diff.
- result: CONFIRMED. Score 13 -> 9 at 141/141 instructions and the preheader is target's in full: move a3,zero / li t2,5 / li t1,20 / move t3,v0 / move a2,t3 / lui t0 / addiu t0 / addiu a1,t0,-9. All nine remaining differing positions are the single i/p-walker register swap. Banked as memory/grind/func_8003504C/chassis_h1_score9.c.
- verdict: CONFIRMED

## [s6] Session 4's src-reuse allocation lever transplants onto the giv chassis if b / w are hoisted to function scope so one of them can carry loop 2's D_801027D8 destination.
- mechanism: Session 4 flipped the i/walker allocation by extending a dead loop-1 pointer's live range into the val == 5 branch; on the giv chassis the only source-visible pointers left in loop 1 are the two in-loop invariants, so hoisting one to function scope should give that lever something to attach to.
- probe: gen6b.py variants h2 (reuse w), h3 (reuse b with the -9 fold) and h4 (reuse b without it); sandbox --disable all on each.
- result: KILLED. 35 / 50 / 51 at 121 / 118 / 116 instructions against target's 141 - the function COLLAPSES, because a function-scope pointer assigned inside loop 1 is no longer a loop-invariant movable, so the emission-class structure the chassis depends on is lost and loop 2's code folds. Banked as rejected/giv-chassis-function-scope-pointer-collapses-loop2.c.
- verdict: KILLED

## [s6] On the h1 chassis the i/p-walker inversion flips by lowering i's allocno rank through a live-range extension (session 1's H3 lever re-aimed at the new chassis).
- mechanism: .greg on h1 ranks the source pseudo 73 (i) ahead of every loop.c-created giv pseudo, so i takes $a2 as the first free register in REG_ALLOC_ORDER and the p-walker giv inherits $a3; extending i past loop 1 should drop it below a giv.
- probe: Three reuses of the dead loop-1 counter, each measured with sandbox --disable all: as loop 2's outer counter (v_i1), as the val==5 table index (v_i2, the mildest possible extension - one extra ref outside any loop), as loop 2's inner counter (v_i3).
- result: KILLED. 33 (136 insns) / 26 (141 insns) / 58 (122 insns) - every extension moves AWAY from target and even the mildest costs 17 points. On this chassis the lever is not lowering i; it must be raising a giv's allocno rank, or eliminating i as a source pseudo. Banked as rejected/h1-chassis-extend-i-liverange-wrong-direction.c.
- verdict: KILLED
