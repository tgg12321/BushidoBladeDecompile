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
