/* func_800645B0 (src/text1b.c) -- BEST HONEST FORM.  The "SB" chassis.
 * Honest distance 1 / 78 (target_insns 78, build_insns 78, rules_dropped 0),
 * RE-MEASURED in grind session s17b (2026-09-02, structural modality) on the
 * current tree with `sandbox func_800645B0 --disable all`.  Ordinary C: zero
 * cheat-asm, zero pins, zero dead stores, one sanctioned FAKE (the `val`
 * variable-reuse that denies loop.c the const-1 hoist).
 *
 * WHY THIS AND NOT THE 0/78 BODY.  The previous session's distance-0 body (the
 * WD chassis plus a `do { idx = i + j; } while (0);` wrap) was FAILED by the
 * layer-1 cheat-reviewer and the wrap is now on this function's mechanically
 * enforced BANNED list.  It is preserved, with the full FAIL reasoning, at
 * rejected/do-while0-wrap-scores-0-but-layer1-FAIL-banned-construct.c.  Do not
 * restore it: the driver discards a candidate-ready that re-declares it before
 * the Judge is ever spawned.
 *
 * THE ONE REMAINING INSTRUCTION.  Stream index 20: we emit `addu $s0,$s0,$s1`
 * where the target has `addu $s0,$s1,$s0`.  optabs.c `expand_binop`
 * (tools/gcc-2.7.2/optabs.c:398-421) swaps a commutative operand pair whenever
 * the expansion target IS op1, so both `idx = idx2 + idx;` and
 * `idx = idx + idx2;` emit the same swapped order.  Emitting the target's order
 * requires the sum's destination pseudo to be distinct from both operands --
 * the "WD" chassis (`wid = idx2 + idx;`), which is ordinary C and fixes index
 * 20, but then loses the inner-loop head (indices 11/12) and the back-edge
 * delay slot (65) to sched.c's `birthing_insn_p` max-priority lift on the
 * loop-top `addu`, because `wid` leaves `idx` single-set.  Measured this
 * session: SB 1/78, WD 3/78, h (second real write `idx = rand() & 7`) 2/78,
 * k (byte offset routed through `idx`) 12/78.
 *
 * SESSION 18 (structural) re-measured this body at 1 / 78 on today's tree and
 * mapped the residual's full geometry.  The *3 sum spelled as a multiplication
 * or a parenthesised subexpression (`idx = idx * 12;`, `idx = (idx2 + idx) * 4;`)
 * fixes index 20 with NO extra local (3 / 78, WD loop-head residual), and
 * `idx = (idx2 + idx) << 2;` reaches 12 / 78 with every one of the 78 opcodes and
 * every position exact -- the first form on this function to hold the operand
 * order, the inner-loop head and the delay slot at once using only the target's
 * own seven locals; its residual is purely register seats.  Those forms are all
 * banked under rejected/.  This SB body remains the floor because it is the only
 * one at 1.
 * Sixteen sessions of banked negatives (53 rejected forms), the RA-seat
 * foreclosure verdicts and the loop.c / optabs.c / sched.c mechanism proofs are
 * in evidence.md + hypotheses.md.  Read them before proposing anything.
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
