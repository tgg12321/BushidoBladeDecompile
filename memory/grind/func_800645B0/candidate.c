/* 2026-08-24 MIGRATION NOTE: HEAD is now INCLUDE_ASM — migrated in
   a7892ba2 (2026-08-24 sweep 2); all rules retired and all in-source cheat-asm removed
   from main. Statements below about "HEAD", pins, rules carried, or
   "applied to src" describe the PRE-MIGRATION tree (banked at
   retired-chassis-2026-08/body.c). This body must be pasted over the
   INCLUDE_ASM line before any sandbox re-measure. */
/* func_800645B0 (src/text1b.c) — the "SB" chassis, the standing honest floor.
 * `sandbox func_800645B0 --disable all` = score 1, target_insns 78,
 * build_insns 78, rules_dropped 1 — re-measured in grind session 9c
 * (permuter modality, 2026-08-13) with this exact body in src/text1b.c.
 *
 * PROVENANCE OF THIS FILE.  The previous contents of candidate.c were the
 * session-9 "WC" body, which added `val = idx; idx = idx2 + val;` before the
 * *3 sum and measured 0 / 78.  The layer-1 cheat-reviewer FAILed it (the
 * staging was a directed-sweep artifact chosen to defeat optabs.c's
 * commutative swap and cherry-picked from six functionally-identical
 * spellings), the driver banned the construct for this function, and its
 * binding next-action was: revert to `idx = idx2 + idx;` and treat 1 / 78 as
 * the honest floor.  That is exactly this body.  Do NOT re-derive a staging
 * spelling for the sum — every variable-staging respelling is the same banned
 * construct.
 *
 * WHAT IT IS.  The canonical for-loop spelling of the function: an outer group
 * loop stepping i by 4, an inner loop over the four slots in the group, the
 * first free slot claimed and its three word fields plus one halfword field
 * initialised from rand(), the occupancy bit OR'd into D_800A3444, and the
 * group abandoned via `break`.
 *
 * THE ENTIRE RESIDUAL is one instruction's operand order: at index 20 the
 * target has `addu $s0,$s1,$s0` and this body emits `addu $s0,$s0,$s1`.
 * Mechanism (H24, session 6): optabs.c:403-421 (`expand_binop`) swaps a
 * commutative operand pair when the expansion target IS op1, and the sum must
 * be written back into `idx` because that second set of `idx` is what denies
 * sched.c's `birthing_insn_p` priority lift on the loop-top `addu idx,i,j`.
 *
 * s11 (2026-08-20, escalation post-unpark): floor re-confirmed 1/78 on that
 * day's tree with this exact body.  The 2026-08-20 or-tree-shape-shift
 * carve-out has NO move here: the swapped sum `idx = idx + idx2;` is
 * byte-identical (H24 re-confirmed — the target's operand order is an
 * RTL-expansion impossibility, not a policy-blocked choice), the loop-top
 * swap `j + i` is strictly worse (3/78), and no 3+-term assoc+commutative
 * expression exists for the grouping sub-axis.  See hypotheses.md H55/H56 and
 * the 2026-08-20 decisions.md disposition entry.
 *
 * SIBLING CHASSIS (both banked, both 3 away, both structurally distinct):
 *   chassis_jd_inline_index_arith.c — 3 / 78, index arithmetic written inline
 *     with no idx2/wid locals; residual is the three loop-top points only.
 *   rejected/maintained-index-nonfold-reset-costs-one-insn.c ("OA") — 3 / 79.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                idx = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (idx << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (idx << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (idx << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
