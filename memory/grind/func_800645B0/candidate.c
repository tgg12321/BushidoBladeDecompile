/* func_800645B0 (src/text1b.c) — grind session 1 (recon modality), 2026-08-12.
 *
 * HONEST PURE-C SANDBOX DISTANCE: 0  (`sandbox func_800645B0 --disable all`,
 * target_insns 78 / build_insns 78, rules_dropped 1, cheat_asm_stripped).
 * Verified byte-exact against asm/funcs/func_800645B0.s with an UNMASKED
 * instruction diff (engine.diagnose.diff_pairs, mask=False): the only four
 * residual text differences are absolute branch/jump target addresses, and all
 * four carry IDENTICAL relative displacements (+0xBC, +0x1C, -0xD0, -0xE8) —
 * they exist only because the sandbox object places the function at a different
 * section offset than build/src/text1b.o.
 *
 * WHAT THE PRIOR SOURCE WAS: a goto-based contortion carrying two cheat-asm
 * register pins (`register s32 s3 asm("$19")`, `register s32 one asm("$3")`) and
 * a bare `do { } while (0);`. Honest distance with the pins stripped: 21.
 *
 * WHAT CHANGED, AND WHY (each step measured):
 *
 *  1. 21 -> 17  Rewrote the goto chain as the natural nested do/while with a
 *     `break`. The target asm IS a plain nested loop: outer `i += 4` until
 *     `i < 15`, inner `j` 0..3, `break` out of the inner loop after the stores.
 *     This fixed the accumulator (s3) and mask (s2) register assignments.
 *
 *  2. 17 -> 10  Defeated loop.c's invariant hoist of the constant `1`.
 *     `mask = 1 << idx` makes the const-1 pseudo a single-set loop invariant, so
 *     `scan_loop`/`move_movables` hoisted it into a fresh callee-save ($s4),
 *     costing an extra save/restore pair (build_insns 81 vs target 78) and
 *     cascading the whole allocation. Fix per [[defeat-licm-hoist-var-reuse]]:
 *     route the constant through a scratch variable (`val`) that is ALSO
 *     assigned a genuinely-used loop-variant value later in the same loop, so
 *     `n_times_set > 1` and the pseudo is not a movable. The variant value is
 *     the D_800A3444 read of the read-modify-write, which the target keeps in
 *     the same register ($v1) — so the reuse also reproduces the target's
 *     register choice, exactly as that rule prescribes ("pick the variable the
 *     target actually reuses").
 *     The same edit split the RMW into `val = D_800A3444; ...; D_800A3444 =
 *     val | mask;` with `last = rand();` named ahead of the halfword store,
 *     which reproduces the target's early `lw $v1,%gp_rel(D_800A3444)` (target
 *     fills the load-delay with the `andi`/`lui`/`sh` cluster; the naive
 *     trailing RMW emitted a `nop` there instead).
 *
 *  3. 10 -> 3   `val = val | mask; D_800A3444 = val;` instead of
 *     `D_800A3444 = val | mask;`. Making `val` the destination of the OR both
 *     reproduces the target's `or $v1,$v1,$s2` (in-place, not into a fresh
 *     temp) and lifts `val`'s reference count above `j`'s, which flipped the
 *     $v1/$a0 assignment so that val=$v1 and j=$a0 as in the target.
 *
 *  4. 3 -> 0    Moved `j += 1;` to sit BETWEEN `idx = i + j;` and `val = 1;`.
 *     At distance 3 the ONLY residual was which of the two ready-at-cycle-0
 *     loop-top insns (`addu $s0,$s3,$a0` vs `li $v1,1`) cc1's first-pass
 *     scheduler emitted first; reorg.c then steals whichever one lands first
 *     into the inner back-edge delay slot and moves the loop label past it.
 *     Target steals the `addu`; our build was stealing the `li`. Interposing
 *     the (already-present, semantically-required) `j += 1;` statement between
 *     them reorders the RTL so the `addu` is emitted first. Note this is pure
 *     statement ORDERING of code that has to exist anyway — nothing was added.
 *     Measured in the tmp/grind/func_800645B0/s1/sweep.py variant sweep:
 *     A_base(3) B_j_between(0) D_bits_named(3) E(7) F_mask_two_step(3)
 *     G_idx_selfadd(3) H(3).
 *
 * INTEGRATION NOTE: regfix.txt still carries `func_800645B0: reorder 3,1,2 @ 1-3`
 * (line 2521). The sandbox drops it (rules_dropped 1) and the function matches
 * WITHOUT it, so it is now redundant and must be retired by the operator/driver
 * (`engine retire func_800645B0`) — a grind session may not touch regfix.txt.
 */
extern s32 rand(void);
extern void *D_800A347C;
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            j += 1;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                *((s32 *)(((s32)(&D_800F0D78)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[0] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
