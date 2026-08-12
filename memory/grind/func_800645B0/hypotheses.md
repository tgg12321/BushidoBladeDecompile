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

## Frontier

1. **Get `idx` multi-set WITHOUT hoisting the sum ahead of the first call.**
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

3. **Attack the commutative operand order through CSE state rather than
   syntax.** H7 killed declaration order and H8 killed the multiply spelling;
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
