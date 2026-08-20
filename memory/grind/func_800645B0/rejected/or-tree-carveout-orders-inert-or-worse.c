/* REJECTED (s11, 2026-08-20) — the or-tree-shape-shift carve-out (owner
 * ruling 2026-08-20, .claude/rules/or-tree-shape-shift.md) has NO committable
 * move on func_800645B0.  This file banks the two orders measured this
 * session on the SB chassis (control = candidate.c, 1/78):
 *
 *   (1) `idx = idx + idx2;`  (the *3 sum swapped)  -> score 1, 78/78,
 *       BYTE-IDENTICAL to the control.  Re-confirms H24 on today's chassis:
 *       optabs.c expand_binop canonicalizes both C orders to (plus idx idx2)
 *       because the expansion target IS op1 for the `idx2 + idx` spelling and
 *       no swap is needed for the other; the target's `addu $s0,$s1,$s0`
 *       (destination == second operand) is unreachable for any body whose
 *       sum destination is `idx`.  There is no "target-matching order" for
 *       the carve-out to commit — the order axis is an RTL-expansion
 *       invariant, not a policy barrier.
 *
 *   (2) `idx = j + i;`  (loop-top sum swapped)  -> score 3, 78/78,
 *       strictly worse: it flips the loop-top addu's operand order AWAY from
 *       target (the control's `i + j` already emits the target's order).
 *
 * Parenthesization/grouping sub-axis: the function contains NO 3+-term
 * associative+commutative expression (every multi-term store RHS contains a
 * subtraction), so regrouping has no application surface at all.
 *
 * The representative body below is form (1); form (2) is the same body with
 * the loop-top sum written `j + i`. */
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
                idx = idx + idx2;
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
