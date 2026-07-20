/* KILLED s27 rederive: swap the two call blocks so ang_next is computed first
 * (with p = table + next_idx*4) and ang_prev second (with p = *(arg0+4) +
 * prev_idx*4). Distinct from s2's swap-if-block-order kill (which reordered
 * the prev_idx / next_idx compute blocks). Both index computes are kept in
 * their original order; only the call sites swap.
 * Result: score=3 -> 16, target_insns=111, build_insns=112 (+1 insn regression).
 * Mechanism: swapping which slot is computed first inverts the p1/p2 identity
 * for RA. The 'first-slot addu ends in v1' allocation stays put, but now the
 * NEXT-neighbor address is computed via p1 (v1 dest), so pseudos 129/130 map
 * differently at insn 89/124. The +1 insn regression suggests scheduling loses
 * the target's delay-slot fill entirely (target's shape packs prev-side sll
 * into the next_idx bnez delay slot at 80057D48 -- that pattern requires
 * prev-side compute to happen first). Confirms s2/s3/s9-style scheduling
 * asymmetry: the pass-order between prev_idx-if, next_idx-block, and the two
 * call sites is not freely permutable; target selected exactly ONE topology
 * and any deviation loses the delay-slot fill.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s16 *p;
    u16 cy;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }

    /* CALL ORDER SWAPPED: next first, then prev */
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)table);
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
