/* s8 candidate — MATCH. sandbox --disable all == 0, build_insns 143 ==
 * target_insns 143, rules_dropped 0, zero inline asm, zero FAKE annotations.
 *
 * This is a STRUCTURAL RE-DERIVATION (s8 modality: rederive), not a tweak of
 * the s3/s5 "h2a" form. It deletes the shared `complete_store:` label and the
 * two function-scope staging variables (var_v1 / var_v0) that every session
 * s3-s7 was built on, and replaces them with per-arm BLOCK-LOCAL variables
 * plus a per-arm OR/store. The two /* FAKE */ staged-value-reused-variable
 * annotations the Judge was gating on are gone with the construct.
 *
 * Why it closes (the s7 QTYDBG account, applied):
 *   s7 measured that our residual was entirely local-alloc.c's hard-register
 *   choice for the else-arm mask constant. In the THEN arm (blk=4) local-alloc's
 *   combine_regs TIES the lui/ori constant pseudo to the AND's destination
 *   pseudo, because BOTH are block-local — which is why the then arm emits
 *   lui/ori/and out of a single register. In the h2a else arm that tie was
 *   impossible: the AND's destination was `var_v1`, a function-scope (global)
 *   pseudo, and combine_regs merges only block-local quantities. The constant
 *   therefore became its own 2-ref quantity, and find_free_reg (no
 *   REG_ALLOC_ORDER in mips.h, so a linear scan from $v0) handed it $v0 — the
 *   same register the then arm used — so reorg.c's redundant_insn deleted the
 *   then-arm `lui $v0` after the delay slot was filled from the dead-branch
 *   thread. Target's else-arm constant lives in $v1, so target keeps both luis.
 *
 *   Declaring the mask holder `m` and the counter `c` INSIDE the else block
 *   makes both block-local, which does two things at once:
 *     1. combine_regs can now tie the mask constant to `m` exactly as it does
 *        in the then arm, so the else arm emits the target's one-register
 *        lui/ori/and shape instead of a separate constant quantity.
 *     2. `m`'s live range now SPANS the counter chain (it is born at the AND
 *        and dies at the `sw`, with the whole `c` chain in between), whereas
 *        h2a's two chains were always contiguous and disjoint (s7 P7a/P7b
 *        measured that no statement ordering could make them overlap). The
 *        counter quantity is shorter and denser, so local-alloc's
 *        qty_compare_1 priority allocates it FIRST and it takes $v0; `m`'s
 *        covering range then finds $v0 busy in regs_live_at and takes $v1 —
 *        target's register. The s6/s7 frontier goal (a block-local quantity
 *        live across the constant) is reached not by lengthening a chain but
 *        by making the OR/store consumer itself block-local.
 *
 *   The store-tail question that h2a's shared `complete_store` existed to
 *   solve resolves itself: the two else arms now store from $v1 while the two
 *   then arms store from $v0, so jump2's find_cross_jump can rtx_equal-merge
 *   only the two else tails with each other, leaving exactly target's three
 *   `sw` sites (two inline then-arm stores + one shared else store).
 *
 * The shared `do_call:` label is retained — s4 H4c measured that duplicating
 * the `func_8005C650(0, 0x7F, 0x7F)` call into each case regresses (6 -> 14).
 */
extern u32 D_800A34F8;
extern s32 D_800A350C;
s32 func_8006B92C(s32 *unused, u32 *arg1) {
    s32 sp10;
    s32 ret;
    s32 idx;
    s32 var_s0 = 0;
    u32 v;
    u32 a0;
    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC + 0xC, &D_800A350C);
    ret >>= 16;
    switch (ret) {
    case 1:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0x4000) {
            D_800A34F8 = a0 & 0xFFFF1FFF;
        } else {
            u32 m = a0 & 0xFFFF1FFF;
            s32 c = ((a0 >> 13) & 7) + 1;
            m |= (c & 7) << 13;
            D_800A34F8 = m;
        }
        goto do_call;
    case 2:
        a0 = D_800A34F8;
        if ((a0 & 0xE000) == 0) {
            D_800A34F8 = (a0 & 0xFFFF1FFF) | 0x4000;
        } else {
            u32 m = a0 & 0xFFFF1FFF;
            s32 c = ((a0 >> 13) & 7) - 1;
            m |= (c & 7) << 13;
            D_800A34F8 = m;
        }
    do_call:
        func_8005C650(0, 0x7F, 0x7F);
        break;
    }

    idx = (D_800A34F8 >> 13) & 7;
    switch (idx) {
    case 0:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 2;
        }
        break;
    case 1:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 3;
        }
        break;
    case 2:
        if (*arg1 & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            var_s0 = 1;
            D_800A34F8 = (D_800A34F8 & ~0x1C00) | (((((D_800A34F8 >> 10) & 7) + 1) & 7) << 10);
        }
        break;
    }

    if (*arg1 & 0x100010) {
        func_8005C650(2, 0x7F, 0x7F);
        var_s0 = 1;
    }
    return var_s0;
}
