# Hypotheses — func_800645B0

## Session 1 (2026-08-12, recon)

### H1 — CONFIRMED. The target is a natural nested loop, not the goto chain HEAD carried.
Rewrote as nested do/while with `break`, deleted both `register asm()` pins:
21 -> 17; the callee-save assignments ($s3 accumulator, $s2 mask) snapped to
target immediately.

### H2 — CONFIRMED. loop.c hoists the constant `1` out of the inner loop, costing two instructions.
`mask = 1 << idx` gives a single-set loop-invariant pseudo; `scan_loop` /
`move_movables` hoist it into a fresh callee-save $s4 (save/restore pair,
81 insns vs 78). Fix: route the constant through a scratch variable that is
also assigned a genuinely-used loop-variant value later in the same loop
([[defeat-licm-hoist-var-reuse]]). 17 -> 10, build_insns 81 -> 78, and the
shared pseudo lands in $v1 — the register the target uses for both roles.

### H3 — CONFIRMED. The $v1/$a0 swap is a reference-count ordering, closable by making the scratch the OR destination.
`val = val | mask; D_800A3444 = val;` instead of `D_800A3444 = val | mask;`.
10 -> 3; val=$v1, j=$a0, and the in-place `or $v1,$v1,$s2` matched.

### H4 — CONFIRMED BUT BANNED. The final residual is a loop-top emission-order effect that an interposed statement flips.
An 8-variant hand sweep found `j += 1;` placed between `idx = i + j;` and
`val = 1;` closes 3 -> 0. The layer-1 cheat-reviewer FAILed it: a statement
relocation with no independent semantic reason, chosen from a directed sweep
purely to flip a scheduler/reorg outcome. It is now a BANNED construct.
Session 2 superseded the *diagnosis* as well — see H5.

## Session 2 (2026-08-12, recon)

### H5 — CONFIRMED. The loop-top residual is sched.c's `birthing_insn_p` register-pressure bonus, not a source-order tie; it is removable by making `idx` multi-set.
- **Mechanism:** `sched.c:birthing_insn_p` returns `reg_n_sets[dest] == 1` for
  a live destination, and `adjust_priority` raises that insn's INSN_PRIORITY
  to `max_priority`. The scheduler is a backward list scheduler, so a bonused
  insn is selected early and emitted LAST in the block. With `idx` single-set,
  `addu idx,i,j` is bonused and `li val,1` lands first in the block; reorg.c
  steals the block's first insn into the inner back-edge delay slot, where the
  target has the `addu`.
- **Probe:** `cc1 -da` on the real build, per-function extraction of
  `t.i.sched` (`tmp/grind/func_800645B0/s1/{dump.sh,extract.py}`); then the
  same dump with `idx` made multi-set (`idx2 = idx << 1; idx = idx2 + idx;`).
- **Result:** ready list at T-6 goes from `31 (1) 28 (7f000001), now 28 31` to
  `31 (1) 28 (1), now 31 28` — the bonus disappears, source order is restored
  by the INSN_LUID tie-break, `addu` is emitted first and lands in the
  back-edge delay slot exactly as target. Score still 3, but the residual has
  MOVED off the loop top entirely (see H6). This replaces the banned lever
  with a construct in a sanctioned family (variable reuse for codegen control)
  that the target's own allocation mirrors ($s0 = i+j, then the sum, then the
  byte offset).

### H6 — CONFIRMED (as a characterisation). The remaining 3 points are two independent effects around the first `rand()` call, and A_base vs K currently trade one for the other.
- Target: `jal rand` with `sll $s1,$s0,1` in the delay slot, then
  `addu $s0,$s1,$s0`. K: `sll` before the call, `addu $s0,$s0,$s1` in the
  delay slot.
- (a) In target the `*3` sum is emitted AFTER the call, because the original
  left it INLINE in the store's address expression (GCC expands the
  call-bearing RHS first). K hoists it into its own statement ahead of the
  first store, so it precedes the call and reorg fills the slot with it.
  A_base (inline sum) gets this region byte-exact.
- (b) The commutative PLUS operands print as (idx, idx2) here, (idx2, idx) in
  target; cse.c `fold_rtx`'s `must_swap` canonicalisation (cse.c:5278ff) is
  what fixes the order.

### H7 — KILLED. Declaration order does not control the commutative PLUS operand order.
Pseudo numbers DO follow declaration order (t.i.combine: i=72 j=73 idx=74
idx2=75 mask=76 val=77), but declaring `idx2` before `idx` (Q), before `idx`
with the `* 3` spelling (R), and first of all locals (S) all leave
`addu $s0,$s0,$s1` and score 3. The SOTN "named-intermediate declaration
order" family is spent on this residual.

### H8 — KILLED. Spelling the stride as a multiply does not help.
`idx * 3` is byte-identical to `idx2 + idx` (L/N == K, 3/78). `idx * 12`
(M/O) costs an extra instruction (79) because the `idx * 2` value stops being
shared with the s16 store; `P` (idx2 as a plain copy, s16 index `idx2 * 2`)
regresses to 21/81.

## Session 3 (2026-08-12, structural) — floor 3 -> 1

### H9 — CONFIRMED. Both of the previous session's residuals close simultaneously if the *3 sum is assigned to `idx` AND the first `rand()` is named into a temp ahead of it.
- **Mechanism:** the sum is a real second set of `idx`, killing the
  `birthing_insn_p` bonus at the loop top; and naming the call puts the CALL
  before the sum in source order, so GCC emits the sum after the `jal` and
  reorg.c fills the delay slot with the `sll` (the target's shape) instead of
  with the sum.
- **Probe:** sweep6.py variants AA/AB/AC (+ control AD, call named but sum
  inline).
- **Result:** AA/AB/AC = **1 / 78** (previous floor 3); AD = 3 / 78. The
  unmasked diff drops from three real pairs to one.
- **Verdict:** CONFIRMED — this is the session's floor move.

### H10 — KILLED. A second assignment to `idx` that CSE/copy-prop can fold does not remove the birthing bonus.
- **Mechanism claim tested:** `reg_n_sets` is a whole-function count, so ANY
  second assignment should do. False in practice — the count is taken after the
  fold passes, so a copy / split-init / recomputation of a live value leaves
  `reg_n_sets == 1`.
- **Probe:** sweep5 (T_split `idx = i; idx += j;`, U_half `idx = idx << 1;`
  after the stores, V_both, W_late) plus sweep11 (FA/FB/FC/FD, the halfword
  offset re-assigned into `idx` three ways); `.sched` dumps of A_base and
  U_half compared directly.
- **Result:** all nine variants 3 / 78, and the two `.sched` dumps carry an
  identical ready list `T-6: 31 (1) 28 (7f000001), now 28 31`.
- **Verdict:** KILLED.

### H11 — KILLED (and mechanism corrected). The commutative operand order of the sum is NOT a cse.c `fold_rtx`/`must_swap` effect and no source spelling changes it.
- **Mechanism (corrected):** `optabs.c:399-417`, `expand_binop` — for a
  commutative optab GCC swaps op0/op1 when `target == op1`. `idx = idx2 + idx;`
  therefore expands as `(plus idx idx2)` *in the .rtl dump*, before cse runs.
- **Probe:** sweep7 (eight respellings: `idx + idx2`, `idx += idx2`,
  declaration swap, `idx2 = idx + idx`, `idx2 = idx * 2`, call before the
  shift, `idx * 3`), plus a read of `dump_AA/f_rtl.txt` insn 53 and of
  optabs.c.
- **Result:** all eight = 1 / 78 with the identical `addu s0,s0,s1`. The
  previous session's frontier item 3 (vary the cse equivalence class) is
  therefore misdirected and is retired.
- **Verdict:** KILLED — the fix must change the assignment DESTINATION, not the
  operand syntax.

### H12 — KILLED. Giving the sum its own destination variable fixes the operand order but cannot keep the loop top, because of local_alloc ordering.
- **Mechanism:** `local_alloc` precedes `global_alloc`; the byte-offset value is
  normally a high-priority BLOCK-LOCAL pseudo that claims `$s0` before the
  block-local `idx2` (which overlaps it) can, and the multi-block `idx` then
  re-uses `$s0`. Routing the byte offset through `idx` removes that claimant,
  so `idx2` takes `$s0` and `idx` is pushed to `$s1`.
- **Probe:** sweep8 (CA/CB/CC/CD), sweep9 (DA/DB/DC/DD/DE), sweep10 (five
  declaration orders), with `.lreg`/`.greg` dumps of DA and AA and a read of
  `global.c:allocno_compare`.
- **Result:** CA = 3 / 78 (operand order + every register right, loop top
  wrong); DA = 12 / 78 (loop top + operand order right, `$s0`/`$s1` swapped);
  declaration order moves nothing.
- **Verdict:** KILLED as written; the underlying constraint (byte offset must
  stay block-local) is now a hard fact for any future shape.

### H13 — KILLED. Recomputing `i + j` as the sum's right operand to dodge the expand-time swap costs a real instruction.
- **Probe:** sweep12 GA/GB/GC/GD.
- **Result:** 19 / **79** (GA/GB/GC) and 13 / **79** (GD) — CSE does not fold
  the recomputation away.
- **Verdict:** KILLED.

## Frontier

0. **The last point: emit `(plus idx2 idx)` while keeping a real second set of
   `idx` and a block-local byte offset.** Mechanism: the three constraints are
   (a) `expand_binop` swaps when the sum's `target == op1`, so the sum's dest
   must not be `idx`; (b) `birthing_insn_p` needs `reg_n_sets[idx] > 1` with a
   set that survives the fold passes; (c) the byte offset must stay a
   block-local pseudo or `local_alloc` hands `$s0` to `idx2`. Every pairwise
   combination has now been measured; no shape satisfying all three has been
   found. Next probe: attack (b) from the other side instead — make the
   const-1 pseudo SINGLE-set so `li val,1` is bonused too (with both insns at
   max_priority the tie falls to INSN_LUID and the dump shows the LATER insn
   picked first, i.e. `addu` emitted first — see the `52/63` and `52/61` ready
   lines in `dump_AA/f_sched.txt`), while preventing loop.c from hoisting it
   (a plain single-set `val = 1;` is a movable and costs 81 insns). Read
   loop.c's `move_movables` preconditions (`m->cond`, `m->global`, the
   `threshold` register-pressure test at loop.c:532), dump `t.i.loop` for the
   AA build to see which test the current const-1 passes, and construct a C
   shape that fails exactly one of them.

1. ~~**Get `idx` multi-set WITHOUT hoisting the sum ahead of the first call.**~~
   **RESOLVED by H9** (session 3): naming the first `rand()` into `last` does
   exactly this and moved the floor 3 -> 1. The paragraph below is kept only
   for its reasoning trail; the "enumerate second assignments" half of it is
   KILLED by H10 (foldable second sets are inert).
   Mechanism: `birthing_insn_p` counts `reg_n_sets` over the WHOLE function,
   while the delay-slot fill only depends on whether the sum statement sits
   before or after the call-bearing store. If a second, semantically-real
   assignment to `idx` exists anywhere else in the function while the sum
   stays inline in the store addresses (A_base's shape), both residuals close
   at once. Next probe: enumerate genuinely-used second assignments of the
   slot index (e.g. the index reused as the s16 element index, or the outer
   accumulator and the slot index sharing one variable) and sandbox each;
   reject any candidate whose second assignment is dead or unread.

2. **Attack the bonus from the other side: make the const-1 pseudo single-set
   AND unhoistable, so BOTH insns are bonused.** With both at max_priority the
   tie falls to INSN_LUID and source order is restored without touching `idx`.
   Mechanism: a single-set const-1 is normally a loop.c movable; find a
   spelling where `move_movables` rejects it (read loop.c's movable
   preconditions — `m->cond`, `m->global`, the `threshold` register-pressure
   test at loop.c:532 — and derive which C shape violates one) while
   `reg_n_sets == 1` still holds. Next probe: instrument/read loop.c's
   rejection reasons on the current build, then construct the shape.

3. ~~**Attack the commutative operand order through CSE state rather than
   syntax.**~~ **RETIRED by H11** (session 3): the operand order is fixed at
   RTL expansion by `expand_binop`'s `target == op1` swap, not by cse; the
   `.rtl` dump already shows the swapped form. Original text below. H7 killed declaration order and H8 killed the multiply spelling;
   what has NOT been probed is the CSE hash-table state at the point the sum
   is folded (cse.c `fold_rtx` must_swap). Next probe: dump `t.i.cse2` for the
   sum insn and identify which equivalence class supplies the swap, then vary
   what else is in that class (e.g. whether `idx * 2` is a named local or an
   inline expression, and whether the s16 store's use of it precedes or
   follows the word stores in source order).

## [s1] The distance-3 residual at the legitimate baseline (session 1 constructs 1-3, `j += 1;` at its natural position) is caused by sched.c's birthing_insn_p register-pressure bonus, not by a source-order/LUID tie, and is removable by making `idx` multi-set.
- mechanism: tools/gcc-2.7.2/sched.c:birthing_insn_p returns reg_n_sets[dest]==1 for a destination live at that point; adjust_priority then raises that insn's INSN_PRIORITY to max_priority. The list scheduler runs BACKWARD, so a bonused insn is selected first and therefore emitted LAST in the block. With `idx` single-set, `addu idx,i,j` is bonused and `li val,1` (val is set 3x, no bonus) lands first in the inner-loop block; reorg.c then steals the block's first insn into the inner back-edge delay slot, where the target has `addu $s0,$s3,$a0`.
- probe: cc1 -da on the real build (tmp/grind/func_800645B0/s1/dump.sh), per-function extraction of t.i.sched and t.i.combine (extract.py), read of sched.c:birthing_insn_p/adjust_priority/rank_for_schedule, then the same dump with `idx` made multi-set via `idx2 = idx << 1; idx = idx2 + idx;`.
- result: Ready list at T-6 goes from `31 (1) 28 (7f000001), now 28 31` to `31 (1) 28 (1), now 31 28`. The bonus disappears, the INSN_LUID tie-break restores source order, `addu` is emitted first in the block and lands in the back-edge delay slot exactly as target. The li/addu diff pair is gone from the unmasked diff.
- verdict: CONFIRMED

## [s1] The remaining 3 points at the new baseline are two independent effects around the first rand() call: the *3 sum must be emitted AFTER the call (so reorg fills the jal delay slot with the `sll`), and the commutative PLUS operands must print as (idx2, idx).
- mechanism: GCC expands a call-bearing RHS before the store's address arithmetic, so an INLINE `(idx2 + idx) << 2` offset lands after the jal (target's shape); hoisting the sum into its own statement ahead of the first store puts it before the call, and reorg.c fills the delay slot with it instead of the sll. Separately, cse.c fold_rtx's must_swap canonicalisation (cse.c:5278ff) fixes the commutative operand order regardless of how it is written in C.
- probe: Side-by-side unmasked instruction listing of target vs build (tmp/grind/func_800645B0/s1/listing.py) at indices 14-31.
- result: target 18-20 = `jal rand` / `sll s1,s0,1` (delay) / `addu s0,s1,s0`; build = `sll s1,s0,1` / `jal rand` / `addu s0,s0,s1`. Everything else in the function is identical (78/78; other pairs are section-offset branch addresses only). The old baseline (inline sum) gets this region byte-exact but has the loop top wrong, so the two residuals are currently mutually exclusive.
- verdict: CONFIRMED

## [s1] Declaration order controls the commutative PLUS operand order, so declaring `idx2` before `idx` will emit `addu $s0,$s1,$s0`.
- mechanism: GCC 2.7.2 assigns local pseudos at expand_decl time, so pseudo numbers follow declaration order (confirmed in t.i.combine: i=72, j=73, idx=74, idx2=75, mask=76, val=77); if cse's commutative canonicalisation ordered operands by regno, swapping the declarations would swap the emitted operands.
- probe: sweep4.py: Q_declswap (idx2 immediately before idx), R_declswap_mul3 (same with the sum written `idx * 3`), S_idx2_first (idx2 declared first of all locals).
- result: All three score 3 / 78 insns with `addu $s0,$s0,$s1` unchanged. The SOTN 'named-intermediate declaration order' family is spent on this residual.
- verdict: KILLED

## [s1] Spelling the 12-byte stride as a multiply (`idx * 3` / `idx * 12`) reproduces the target's operand order, because expmed.c's synth_mult expands x*3 as (plus (ashift x 1) x) with the shift result first.
- mechanism: synth_mult builds the multiply RTL directly rather than through the C-level PLUS, so its operand order might escape cse's commutative canonicalisation.
- probe: sweep3.py: L_mul3_mul2, N_mul3_shift2, M_mul12_mul2, O_mul12_shift2, P_mul12_only.
- result: `* 3` is byte-identical to the explicit `idx2 + idx` (L/N == K: 3 / 78) — synth_mult's order does not survive canonicalisation. `* 12` regresses to 12 / 79 because the `idx * 2` value stops being shared with the D_800F0BCC s16 store; P_mul12_only regresses to 21 / 81.
- verdict: KILLED

## [s2] Both of the previous session's residuals (the loop-top emission order and the jal delay-slot fill) close simultaneously if the *3 word-index sum is assigned back into `idx` AND the first rand() call is named into the existing `last` temp ahead of it.
- mechanism: The sum is a REAL second set of `idx`, so reg_n_sets[idx] == 2 and sched.c's birthing_insn_p bonus no longer fires on `addu idx,i,j`; the inner-loop block emits `addu` first and reorg.c steals it into the back-edge delay slot as target. Independently, naming the call puts the CALL ahead of the sum in source order, so GCC emits the sum after the `jal` and reorg fills the jal delay slot with the `sll` rather than with the sum.
- probe: tmp/grind/func_800645B0/s2/sweep6.py variants AA (first rand() named), AB (every rand() named), AC (sum spelled `idx * 3`), plus control AD (call named, sum left inline); each scored with `sandbox func_800645B0 --disable all` and diffed unmasked with diffvar6.py.
- result: AA/AB/AC = score 1, build_insns 78 (previous floor 3); AD = 3/78. The unmasked diff drops from three real pairs (li/addu swap at the loop top and back edge, plus the operand order) to one (the operand order).
- verdict: CONFIRMED

## [s2] Any second assignment to `idx` anywhere in the function removes the birthing_insn_p bonus, because reg_n_sets is a whole-function count.
- mechanism: reg_n_sets is recomputed by reg_scan/flow AFTER cse, combine and copy propagation, so a second set whose value is already available in another live pseudo is folded away before the count is taken.
- probe: sweep5.py (T_split `idx = i; idx += j;`, U_half `idx = idx << 1;` after the stores with the s16 store using `idx`, V_both, W_late `idx = idx2 + idx;` after stores that already compute the sum inline) and sweep11.py (FA/FB/FC/FD: the halfword offset re-assigned into `idx` as `<<1`, `+idx`, `*2`); plus a direct comparison of the extracted t.i.sched blocks for A_base and U_half.
- result: All nine variants score 3 / 78, and the two .sched dumps carry a byte-identical inner-loop ready list (`;; ready list at T-6: 31 (1) 28 (7f000001), now 28 31`). A reg_n_sets lever needs a set whose VALUE is not already live.
- verdict: KILLED

## [s2] The commutative operand order of the *3 sum (`addu s0,s1,s0` in target vs `addu s0,s0,s1` in our build) is a cse.c fold_rtx/must_swap effect that can be steered by respelling the expression.
- mechanism: CORRECTED: it is optabs.c:399-417 (expand_binop), which swaps op0/op1 for a commutative optab whenever `target == op1` (or op1 is a REG and op0 is not). `idx = <x> + idx;` therefore expands as (plus idx x) at RTL EXPANSION, before cse ever runs.
- probe: sweep7.py, eight respellings (`idx = idx + idx2`, `idx += idx2`, declaration swap, `idx2 = idx + idx`, `idx2 = idx * 2`, call before the shift, `idx = idx * 3`, s16-offset variation); plus reading the expand dump tmp/grind/func_800645B0/s2/dump_AA/f_rtl.txt (insn 53) and tools/gcc-2.7.2/optabs.c.
- result: All eight measure 1 / 78 with the identical `addu s0,s0,s1`, and the .rtl dump already carries `(plus (reg 74=idx) (reg 75=idx2))` — the swap is pre-cse. The previous session's frontier item 'vary the cse equivalence class' is retired as misdirected.
- verdict: KILLED

## [s2] Giving the *3 sum its own destination variable fixes the operand order and can be combined with a real second set of `idx` (the byte offset) to reach 0.
- mechanism: local_alloc runs BEFORE global_alloc. The 12-byte-stride byte offset is normally a BLOCK-LOCAL CSE temp with high allocno priority (floor_log2(n_refs)*n_refs/live_length, global.c:allocno_compare), so it claims $s0 first; the block-local `idx2`, whose range overlaps it, is pushed to $s1; the multi-block `idx` then re-uses $s0. Routing the byte offset through `idx` removes that early claimant, so `idx2` takes $s0 and `idx` is forced to $s1.
- probe: sweep8.py (CA/CB/CC third local `wid` for the sum; CD sum before the call), sweep9.py (DA/DB/DC/DD/DE byte offset routed through `idx`), sweep10.py (five declaration orders on the DA shape), with .lreg/.greg dumps of DA and AA and a read of global.c:allocno_compare.
- result: CA = 3/78: operand order and EVERY register correct, only the loop top wrong. DA = 12/78: loop top and operand order both correct, but idx=$s1 / idx2=$s0 where target has idx=$s0 / idx2=$s1, renaming 12 points. All five declaration orders on DA stay at 12/78 (second independent confirmation that declaration order is dead here).
- verdict: KILLED

## [s2] Writing the sum's right operand as a recomputation of the slot index (`idx = idx2 + (i + j);`) makes op1 a fresh pseudo at expand time, dodging the `target == op1` swap at zero cost because CSE folds the recomputation back onto `idx`.
- mechanism: expand_binop's swap test compares the assignment target against op1 by rtx identity, so a recomputed temp escapes it; CSE was expected to eliminate the duplicate `addu` afterwards.
- probe: sweep12.py GA (`idx = idx2 + (i + j)`), GB (`(i + j) + idx2`), GC (`idx2 + i + j`), GD (`idx2 = (i+j) << 1; idx = idx2 + (i+j);`).
- result: GA/GB/GC = 19 / 79 insns, GD = 13 / 79 — the recomputed addu SURVIVES (one instruction over the 78-instruction target) and cascades the allocation. Axis dead; it was also the only shape this session that would have needed a cheat-checklist defence, and the measurement settles it without one.
- verdict: KILLED

## Session 4 (2026-08-12, structural) — floor stays 1; the residual is now closed on three sides

### H14 — CONFIRMED. The maintained-index loop form satisfies BOTH previously-exclusive constraints, and its residual is a different single instruction.
- **Mechanism:** setting `idx` before the inner loop and again at the loop's
  tail gives it two genuinely different, non-foldable values, so
  sched.c's `birthing_insn_p` bonus cannot fire on the loop-top `addu`; and
  because the *3 sum no longer has to be the second set, it can take its own
  destination `wid`, which avoids optabs.c `expand_binop`'s `target == op1`
  commutative swap.
- **Probe:** sweep13.py IA/IB/IC/IE, sweep16.py RA/RB.
- **Result:** IA/IC/IE/RA/RB = 1 / 78; IB (sum back into `idx`) = 2 / 78. IA's
  unmasked diff is a SINGLE pair at index 11: target `addu s0,s3,a0`, build
  `move s0,s3`.
- **Verdict:** CONFIRMED as a mechanism; dead as a route to zero — see H15.

### H15 — KILLED. The loop-entry copy of a maintained index cannot be made to emit `addu s0,s3,a0`.
- **Mechanism:** GCC knows `j == 0` in the basic block that contains
  `j = 0; idx = i + j;`, so the PLUS is folded to a copy. In the target that
  insn is not source-level at all: it is reorg.c's duplicate of the loop-top
  insn it stole into the back-edge delay slot (the same `addu $s0,$s3,$a0`
  appears at 0x800645DC and 0x800646B4).
- **Probe:** sweep13.py IC (`idx = i;`), sweep16.py RB (`idx = j + i;`),
  sweep14.py LA (`j` reset at the OUTER loop tail so the entry copy cannot see
  a constant), sweep13.py ID (`idx += 1;` tail update).
- **Result:** IC/RB byte-identical at 1/78; LA = 5 / 79 (costs a real
  instruction); ID = 19 / 83.
- **Verdict:** KILLED.

### H16 — CONFIRMED. A second set of `idx` with a genuinely new value fixes CA's loop top; the residual then moves to that value's register.
- **Mechanism:** `reg_n_sets[idx] > 1` denies `addu idx,i,j` the
  birthing_insn_p bonus, so the inner-loop block emits it first and reorg.c
  steals it into the back-edge delay slot. But `idx` is a multi-block pseudo
  allocated `$s0` and GCC 2.7.2 does not split live ranges, so whatever value
  is staged through `idx` is emitted in `$s0`.
- **Probe:** sweep15.py MA (`idx = last & 7;` before the s16 store), MB (same,
  before the D_800A3444 read), MD (`idx = val | mask;` feeding the store),
  MC (`idx = rand();` with the mask re-derived at the use site); diffs via
  diffvar15.py.
- **Result:** MA/MB/MD = 2 / 78 (CA control 3 / 78) with all three loop-top
  diffs gone and exactly two new ones, always the staged value's register
  (`andi s0,v0,0x7` / `sh s0,0(at)` for MA; `or s0,v1,s2` / `sw s0,0(gp)` for
  MD). MC = 3 / 78 — the value was still live elsewhere, so the fold rule from
  H10 applies again.
- **Verdict:** CONFIRMED, and it yields the closure argument in H17.

### H17 — CONFIRMED (as a characterisation). Only three values may legally be staged through `idx`, and all three are now measured.
The target keeps exactly three values in `$s0`: `i + j`, the *3 sum, and the
12-byte byte offset. Therefore a second set of `idx` must be one of them:
the sum (the shipped 1-point form, blocked by expand_binop's swap), the byte
offset (variant DA, 12/78 — local_alloc assigns `$s0` to the block-local
`idx2` before global_alloc reaches the multi-block `idx`), or `i + j` again
(the maintained-index form, 1/78, blocked by H15). Nothing else in the function
is derived from the slot index.

### H18 — KILLED. The const-1 pseudo cannot be single-set and stay inside the loop.
- **Mechanism:** loop.c:695-716 admits a single-set loop-invariant SET as a
  movable through any one of three alternatives — a compiler temp passes
  `! REG_USERVAR_P && ! REG_LOOP_TEST_P`, a user variable used only in the
  set's own basic block passes `reg_in_basic_block_p`, and in both cases
  `! maybe_never && ! loop_reg_used_before_p` also holds because the set is the
  first insn of the loop body. move_movables' desirability test at loop.c:1631
  (`threshold * savings * m->lifetime >= insn_count`, threshold =
  `(loop_has_call ? 1 : 2) * (1 + n_non_fixed_regs)`) is trivially satisfied,
  and `m->global` does not block the move.
- **Probe:** sweep14.py KA (named `one`), KB (`one` also the return value),
  KD; sweep17.py TA (unnamed `mask = 1 << idx;`), TB (control).
- **Result:** KA/KD = 12 / 80, KB = 13 / 80, TA = 12 / 80 — all hoisted into a
  fresh callee-save against a 78-instruction target. TB = 3 / 78.
- **Verdict:** KILLED. Frontier item 2 of the previous session is closed.

### H19 — KILLED. A loop-tail `idx = i + j;` added to the loop-top recompute is a dead store.
- **Probe:** sweep14.py JA. **Result:** 3 / 78, byte-identical to CA; flow.c
  deletes it before reg_n_sets is taken. **Verdict:** KILLED.

### H20 — KILLED (no gradient). The canonical `for` spelling of both loops is byte-identical to the do/while form.
- **Probe:** sweep16.py SA (for-spelling of CA) and SB (for-spelling of AA).
- **Result:** SA = 3 / 78 (== CA), SB = 1 / 78 (== AA). The previous session's
  frontier item 3 (block membership via loop form) has no gradient here.
- **Verdict:** KILLED as a lever — but adopted as the shipped spelling, since it
  is the more natural C and removes any question about where `j += 1;` sits.

## Frontier (rewritten by session 4)

0. **Make `local_alloc` give `$s0` to the multi-block `idx` in the DA shape.**
   DA (`wid = idx2 + idx; idx = wid << 2;`, stores use `idx`) is the ONLY
   untried-in-detail way to satisfy all three constraints at once: correct
   operand order, a real second set of `idx`, and a value the target really
   does keep in `$s0`. It scores 12/78 and the ENTIRE cost is that `idx` and
   `idx2` are swapped between `$s0` and `$s1`. Mechanism: local_alloc runs
   before global_alloc; in the AA/CA shape the pseudo that claims `$s0` from
   local_alloc is the byte-offset CSE temp (block-local, lives across all four
   `rand()` calls, so it needs a callee-save), and `idx2` is pushed to `$s1`;
   in DA that claimant is gone, `idx2` takes `$s0`, and the multi-block `idx`
   gets `$s1`. Next probe: read `local-alloc.c`'s `block_alloc` /
   `find_free_reg` ordering (qty priority, `qty_phys_reg` suggestions,
   `accept_call_clobbered`), dump `.lreg`/`.greg` for DA and for the shipped
   form side by side, and look for a C shape in which the halfword-offset value
   is NOT a block-local pseudo (e.g. genuinely used in the loop-top block) or
   in which some other block-local callee-save-needing value out-prioritises
   it — the target's own allocation proves such a state exists.

1. **Attack the `expand_binop` swap through the expansion target rather than
   the syntax.** The shipped 1-point residual is `idx = idx2 + idx;` expanding
   as `(plus idx idx2)` because optabs.c:399-417 swaps a commutative pair when
   `target == op1`. Every source-level respelling is measured dead (session 3's
   H11, eight spellings). What has NOT been probed is whether the assignment
   can be expanded with a target that is not the `idx` pseudo yet still lands
   in `idx`'s register — e.g. an assignment whose LHS is a narrower or
   differently-typed view of the same variable, or an expression GCC expands
   into a temp and then copies (the copy being coalesced away). Next probe:
   read `expand_expr`'s `TREE_CODE (to) == VAR_DECL` store path in expr.c to
   enumerate which C constructs cause `expand_binop` to be called with
   `target == 0`, then measure each.

2. **Re-derive the original's inner-loop block from the .sched dump of the
   shipped form vs a synthetic CA dump.** All three shapes are now one insn
   away and each fails on a DIFFERENT, fully-named GCC decision, which is the
   signature of a shape that is close but not the original. Next probe: dump
   `t.i.sched` and `t.i.greg` for SB, CA and IA (the tooling is
   `tmp/grind/func_800645B0/s2/dumpvar.sh` + `dumpAA.py`), and compare the
   inner-loop block's insn list and the register dispositions against what the
   target's 78 instructions imply, looking for a pseudo count or live-range
   shape none of the three reproduce.

## [s3] The maintained-index loop form (slot index set before the inner loop and again at its tail, with the *3 sum taking its own destination variable) satisfies BOTH constraints the previous session called mutually exclusive: a real non-foldable second set of `idx` AND a sum whose destination is not `idx`.
- mechanism: Two genuinely different values in `idx` make reg_n_sets[idx] == 2, so sched.c's birthing_insn_p bonus cannot fire on the loop-top `addu idx,i,j` and the block emits it first; and because the sum is no longer required to be the second set, it can be assigned to `wid`, avoiding optabs.c expand_binop's commutative swap (which fires only when the expansion target IS op1).
- probe: tmp/grind/func_800645B0/s3/sweep13.py variants IA/IB/IC/IE and sweep16.py RA/RB, each scored with `sandbox func_800645B0 --disable all`; unmasked instruction pairs via diffvar13.py.
- result: IA/IC/IE/RA/RB = score 1, build_insns 78 (IB, sum written back into idx, = 2/78). IA's ENTIRE unmasked residual is a single pair at index 11: target `addu s0,s3,a0`, build `move s0,s3`.
- verdict: CONFIRMED

## [s3] The loop-entry copy of a maintained index can be made to emit `addu s0,s3,a0` instead of `move s0,s3`.
- mechanism: GCC constant-folds `i + j` in the block that also contains `j = 0;`. In the target that instruction is not source-level at all: `addu $s0,$s3,$a0` appears TWICE in asm/funcs/func_800645B0.s (0x800645DC, immediately before .L800645E0, and 0x800646B4, in the back-edge delay slot), which is reorg.c's steal-from-target transformation plus the loop-entry duplicate it forces.
- probe: sweep13.py IC (`idx = i;`), sweep16.py RB (`idx = j + i;`), sweep14.py LA (`j` reset at the OUTER loop tail so the entry copy cannot see a constant), sweep13.py ID (`idx += 1;` as the tail update).
- result: IC and RB are byte-identical at 1/78; LA costs a real instruction (5 / 79); ID = 19 / 83. No spelling defeats the fold at zero cost.
- verdict: KILLED

## [s3] In the CA shape (sum in its own `wid`) a second set of `idx` whose VALUE is genuinely new removes sched.c's birthing_insn_p bonus and fixes the loop-top emission order.
- mechanism: birthing_insn_p (sched.c:2504-2537) returns reg_n_sets[dest]==1 for a live destination and adjust_priority raises that insn to max_priority; the backward list scheduler then emits it LAST in the block, so `li val,1` lands first and reorg.c steals the wrong insn into the back-edge delay slot. A second set whose value is not already live survives the fold passes and kills the bonus.
- probe: sweep15.py MA (`idx = last & 7;` staged before the s16 store), MB (same, staged before the D_800A3444 read), MD (`idx = val | mask;` feeding the store), MC (`idx = rand();` with the mask re-derived at the use site); CA control; unmasked diffs via diffvar15.py.
- result: MA/MB/MD = 2 / 78 against the CA control at 3 / 78, with ALL THREE loop-top diffs gone. The two remaining diffs are always the staged value's register: MA emits `andi s0,v0,0x7` / `sh s0,0(at)` where target has $v0; MD emits `or s0,v1,s2` / `sw s0,0(gp)` where target has $v1. MC = 3/78 (value still live elsewhere, so the fold rule applies).
- verdict: CONFIRMED

## [s3] Only three values may legally be staged through `idx`, and all three are now measured — this closes the enumeration for the last instruction.
- mechanism: `idx` is a multi-block pseudo allocated $s0 and GCC 2.7.2 has no live-range splitting, so every value routed through `idx` is emitted in $s0. The target keeps exactly three values in $s0: `i + j`, the *3 sum, and the 12-byte byte offset.
- probe: Direct reading of asm/funcs/func_800645B0.s (78 instructions) plus the measured variants: the sum (shipped form), the byte offset (variant DA from the previous session), and `i + j` again (this session's IA).
- result: Sum -> optabs.c's `target == op1` swap, 1/78. Byte offset -> DA, 12/78, because local_alloc runs before global_alloc and the block-local `idx2` claims $s0 first. `i + j` again -> IA, 1/78, blocked by the entry-copy constant fold. Nothing else in the function is derived from the slot index.
- verdict: CONFIRMED

## [s3] The const-1 pseudo can be made single-set (so `li val,1` is bonused too and the loop-top tie falls back to source order) while keeping loop.c from hoisting it out of the inner loop.
- mechanism: loop.c:695-716 admits a single-set loop-invariant SET as a movable through any one of three alternatives: a compiler temp passes `! REG_USERVAR_P && ! REG_LOOP_TEST_P`; a user variable used only in the set's own basic block passes `reg_in_basic_block_p`; and in both cases `! maybe_never && ! loop_reg_used_before_p` also holds because the set is the first insn of the loop body. move_movables' desirability test at loop.c:1631 is trivially satisfied and m->global does not block the move.
- probe: sweep14.py KA (named user variable `one`), KB (`one` also the function's return value, so m->global is set), KD; sweep17.py TA (UNNAMED constant, `mask = 1 << idx;`, i.e. loop.c's REG_EQUAL / m->move_insn path) and TB (control).
- result: KA/KD = 12 / 80, KB = 13 / 80, TA = 12 / 80 — every spelling hoisted into a fresh callee-save, 80 instructions against a 78-instruction target. TB control = 3 / 78. The previous session's frontier item 2 is closed.
- verdict: KILLED

## [s3] A loop-tail `idx = i + j;` added on top of the loop-top recomputation gives `idx` a second set without disturbing the CA shape's operand order.
- mechanism: The tail store is dead — the loop top recomputes `idx` on the next iteration and the break path never reads it — so flow.c deletes it before reg_n_sets is taken, the same fold rule the previous session established for copies, split-inits and recomputations.
- probe: sweep14.py JA against the CA control.
- result: JA = 3 / 78, byte-identical to CA.
- verdict: KILLED

## [s3] Writing both loops in the canonical `for` form changes inner-loop basic-block membership and therefore what reorg.c can steal into the back-edge delay slot (previous session's frontier item 3).
- mechanism: In the `for` spelling the inner counter's increment sits at the loop bottom instead of being an explicit `j += 1;` statement before the occupancy test, which changes the candidate set reorg.c sees independently of any priority lever.
- probe: sweep16.py SA (for-spelling of CA) and SB (for-spelling of AA), each scored and diffed unmasked with diffvar16.py.
- result: SA = 3 / 78, byte-identical to CA; SB = 1 / 78, byte-identical to AA (single diff at index 20, the commutative operand order). No gradient. Adopted as the shipped spelling anyway: it is the more natural C and it retires any question about where `j += 1;` sits, since the canonical form compiles to the same bytes.
- verdict: KILLED

## Session 5 (2026-08-12, permuter) — floor stays 1

### H21 — KILLED. Random permuter search from the shipped for-loop chassis finds nothing below the floor.
- **Mechanism claim tested:** the shipped form's single residual is an
  expand-time commutative swap (`target == op1`); random source mutation might
  stumble on a spelling whose expansion target is not `idx` while keeping the
  loop top and the block-local byte offset.
- **Probe:** clean offset-0 workspace (`tmp/grind/func_800645B0/s4/mkws.sh`,
  base 78 insns vs target 78, permuter base score 10), campaign
  `shipped-for-chassis`, -j 6, 39,731 iterations / ~21 min.
- **Result:** one novel find, at score 10 (== base): `idx = (last = rand());`,
  a dead store overwritten on the next line — a coercion construct with zero
  gain. Nothing below base.
- **Verdict:** KILLED for this chassis.

### H22 — KILLED. Random permuter search from the IA maintained-index chassis converges on an already-measured shape and never beats it.
- **Mechanism claim tested:** IA's residual (`move s0,s3` where target has
  `addu s0,s3,a0`) is a constant fold of `j == 0` at the loop-entry copy;
  random mutation of the loop skeleton might produce a shape where `j` is not
  provably constant at that point without paying an instruction.
- **Probe:** second workspace built from the IA body
  (`tmp/grind/func_800645B0/s4/{mkia.py,mkws2.sh}`, permuter base score 200),
  campaign `ia-maintained-index-chassis`, -j 6, 52,757 iterations / ~35 min.
- **Result:** 10 novel finds; best = 10, i.e. the CA+`idx = last & 7;` shape
  that session 3 already measured as MA (sandbox 2/78) — rediscovered from a
  different chassis, and never improved on. All other finds (125-200) defeat
  the entry-copy fold only by breaking semantics (`j` read uninitialised,
  `j = mask;`, `j = idx2;` with `idx2` hoisted), i.e. they confirm session 3's
  LA measurement that defeating the fold costs real instructions.
- **Verdict:** KILLED.

### H23 — CONFIRMED (as corroboration). The near-floor basin contains exactly the shapes the hand sweeps already enumerated.
Two structurally distinct chassis, ~92,500 iterations, a clean per-function
metric, and the ONLY legitimate shape random search surfaced is one already in
the ledger (MA). Combined with session 3's H17 closure argument (only three
values may be staged through `idx`, all three measured), the remaining work is
NOT more sampling of this basin — it is the two named GCC-internals probes on
the frontier (expand_binop target selection; local_alloc ordering in DA).

## [s4] Random permuter search from the shipped for-loop chassis finds a form below the floor of 1 by stumbling on a spelling whose expansion target is not `idx` (defeating optabs.c expand_binop's `target == op1` commutative swap) while keeping the loop-top emission order and the block-local byte offset.
- mechanism: The shipped form's whole residual is one instruction, `addu s0,s0,s1` vs target `addu s0,s1,s0`, fixed at RTL expansion by optabs.c:399-417. Eight hand-authored respellings are already measured dead (s3 H11), so the remaining hope on this axis was a shape no human enumerated.
- probe: Built a clean per-function permuter workspace (tmp/grind/func_800645B0/s4/mkws.sh): full-TU cpp of src/text1b.c as base.c, compile.sh running the honest pipeline (cc1 -mel | prologue_fix | maspsx | multu_pad, NO regfix/asmfix) then extracting the `.ent func_800645B0 .. .end func_800645B0` region and assembling it alone so the function sits at offset 0 like target.o. Validated: base 78 insns vs target 78, single diff. Campaign `shipped-for-chassis`, -j 6, via tools/permuter_campaign.py launch/wait/harvest.
- result: 39,731 iterations / ~21 min. Permuter base score 10 (the lone operand-order diff counts as two register diffs x5). Exactly ONE novel find, at score 10 — equal to base, no improvement. Its only mutation is `idx = (last = rand());`, a dead store to `idx` overwritten by the *3 sum on the next line: a dead-store coercion with zero measured gain. Rejected and banked.
- verdict: KILLED

## [s4] Random permuter search from the IA maintained-index chassis finds a shape where `j` is not provably 0 at the loop-entry copy — so GCC emits the target's `addu s0,s3,a0` instead of `move s0,s3` — without paying an instruction elsewhere.
- mechanism: IA (slot index set before the inner loop and again at its tail, *3 sum in its own `wid`) is 1/78 and its entire residual is that GCC constant-folds `i + j` in the block that also contains `j = 0;`. Session 3's LA probe defeated the fold by resetting `j` at the outer-loop tail but cost a real instruction (5/79); the question was whether some unenumerated shape defeats it for free.
- probe: Second workspace built from the IA body (tmp/grind/func_800645B0/s4/mkia.py + mkws2.sh), sharing compile.sh/target.o with the first; validated at 78 vs 78 with the single `move`/`addu` diff. Campaign `ia-maintained-index-chassis`, -j 6.
- result: 52,757 iterations / ~35 min from permuter base score 200 (a `move` vs `addu` residual is charged as insert+delete, 100 each, not as register diffs). 10 novel finds; best = 10, nothing below it and nothing at 0. The score-10 find is the CA chassis with `idx = last & 7;` staged before the s16 store — session 3's MA shape (sandbox 2/78) — rediscovered by mutation from a different chassis. Every other find (125-200) defeats the entry-copy fold only by breaking semantics: `j = 0;` relocated into the found-slot arm so `j` is read uninitialised (160), `j = idx2;` with `idx2 = 0;` hoisted above the outer loop (166), `j = mask;` inside an `if (1) { }` wrapper with the test rewritten `(j + 1) < (4 + 1)` (135).
- verdict: KILLED

## [s4] The near-floor basin around this function contains exactly the shapes the previous sessions' hand sweeps already enumerated.
- mechanism: If independent random search over two structurally distinct chassis, with a clean per-function metric, converges on shapes already in the ledger and never beats them, the enumeration argument from session 3's H17 (only three values may legally be staged through `idx`, all three measured) is corroborated by a second, non-hand-directed method.
- probe: The two campaigns above, ~92,500 iterations total, plus inspection of every novel output's source diff.
- result: The only legitimate shape surfaced was MA (already banked at 2/78). No form below the floor of 1 exists anywhere in the sampled neighbourhood of either chassis.
- verdict: CONFIRMED
