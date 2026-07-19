/* s14 permuter find (chassis: p1/p2 shift-expression spelling + p2 table-source spelling).
 * base_score=15, close-form found at iter 19 / 40.1s / seed 1.
 *
 * REJECTED — same cheat class as:
 *   - rejected/permuter-s16-new_var2-p1-alias.c (s5, 2026-06-...)
 *   - rejected/permuter-long-new_var2-p1-alias.c (s4)
 *   - rejected/permuter-s16-new_var2-p2-alias.c (s13, p2-side variant)
 *   - rejected/block-scope-alias-p1.c (layer-1+2 FAIL)
 *
 * Cheat lens (per [[no-new-park-categories]] "cheats-by-any-spelling"):
 *   • No semantic purpose — a human writing "walk to prev-idx point, read
 *     x/y" would NOT introduce a separate `s16 *new_var2` local for the p1
 *     computation alone. The only observable effect is shifting pseudo 86's
 *     lifetime by giving the p1 SET a distinct anchor pseudo (128 or
 *     similar) that then bridges to pseudo 86 with the correct hard-reg
 *     copy-pref chain.
 *   • Justification references GCC internals — "shifts pseudo 86's
 *     copy-pref origin away from pseudo 129 v1 at insn 124 p2 addu",
 *     exactly the axis s6/s7 forensics identified as the discriminator.
 *   • Judge s10 binding constraint (this ledger) forbids ANY split of the
 *     shared pointer `p` into two source-level locals, under ANY spelling —
 *     semantic naming, numeric suffix, block scope, function scope, or
 *     FAKE-annotated. `new_var2` is exactly a numeric-suffix rename.
 */

void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3)
{
    s16 *new_var2;
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev, ang_next, ang_mid, scale, base, half;
    u16 cx, cy;
    s16 new_var;
    s16 *p;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *((s16 **)(arg0 + 4));
    cx = *((u16 *)(((s32)table + arg1 * 4) + 0));
    cy = *((u16 *)(((s32)table + arg1 * 4) + 2));
    if (((s16)prev_idx) < 0) {
        prev_idx = arg0[3] - 1;
    }
    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if (((s16)tmp) >= (s32)arg0[3]) {
            next_idx = 0;
        }
    }
    /* CHEAT: the split of p1's assignment through `new_var2` is a
     * pointer-alias holder — no semantic purpose. */
    new_var2 = (s16 *)((((s32)((s16)prev_idx)) << 2) + (s32)table);
    p = new_var2;
    ang_prev = single_game_getEnemyCharId(p[0] - (s16)cx, p[1] - (s16)cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16)cx, p[1] - (s16)cy) & 0xFFF;
    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = ((s32)(ang_prev - ang_next)) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }
    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
