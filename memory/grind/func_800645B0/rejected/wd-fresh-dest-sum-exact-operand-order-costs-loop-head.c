/* REJECTED (session 12, 2026-08-25) — "WD": the fresh-destination sum.
 * Measured 3 / 78 by `sandbox func_800645B0 --disable all` (SB chassis = 1/78).
 *
 * WHAT IT PROVES (this is the reason the file is banked, not merely rejected).
 * `wid = idx2 + idx;` with a FRESH local as the sum's destination is the FIRST
 * honest spelling ever measured on this function that emits the target's exact
 * operand order at stream index 20: `addu $s0,$s1,$s0` (all previous forms
 * emitted `addu $s0,$s0,$s1`).  No staging, no dead store, no reordered
 * statement — three distinct quantities (slot index k, halfword offset 2k, word
 * offset 3k) each carrying their own name, which is ordinary C.
 *
 * WHY IT COSTS 3.  Ground-truth diff (tmp/grind/func_800645B0/s12/diff.py)
 * against asm/funcs/func_800645B0.s, 78 vs 78 insns:
 *     20  OURS addu s0,s1,s0   TGT addu s0,s1,s0   <- now EXACT
 *     11  OURS addiu v1,zero,1 TGT addu s0,s3,a0
 *     12  OURS addu s0,s3,a0   TGT addiu v1,zero,1
 *     65  OURS addiu v1,zero,1 TGT addu s0,s3,a0
 * i.e. the entire residual relocates to the inner-loop HEAD placement.  Target
 * puts `addu $s0,$s3,$a0` (idx = i + j) in the OUTER-loop preheader and opens
 * the inner loop at `.L800645E0: addiu $v1,$zero,0x1`; this chassis inverts
 * that (li hoisted into the preheader, the addu at the inner-loop head), and
 * index 65 is the back-edge delay slot, which follows.
 *
 * WHY THE TRADE IS A SOURCE-LEVEL IDENTITY, NOT A SEARCH ARTIFACT.
 * tools/gcc-2.7.2/optabs.c:398-421 (expand_binop, commutative case) swaps op0
 * and op1 when `((target == 0 || REG) ? ((op1 REG && op0 not REG) || target ==
 * op1) : rtx_equal_p (op1, target)) || op0 is CONST_INT`.  With the sum's
 * expansion target == idx's pseudo there are exactly two cases and both lose:
 * `idx = idx2 + idx` has target == op1 so the swap fires and emits (idx, idx2);
 * `idx = idx + idx2` never swaps and emits (idx, idx2) directly.  So ANY C
 * statement whose destination is `idx` emits `addu $s0,$s0,$s1`.  Target's
 * order therefore REQUIRES a destination pseudo distinct from both operands —
 * which is exactly what removes the second set of `idx`, and the second set of
 * `idx` is what the ledger (session 1/6, H24) attributes the loop-head
 * placement to.  The 1-vs-3 trade is closed-form: you cannot have both halves
 * through the destination of this statement.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
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
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
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
