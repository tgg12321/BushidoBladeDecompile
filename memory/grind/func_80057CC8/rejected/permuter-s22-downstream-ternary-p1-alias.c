/* s22 permuter close (chassis: downstream-ternary + *arg2/*arg3 write-order).
 * Score 0 at iter 4265 under PERM_RANDOMIZE overlay + 3-alt PERM_GENERAL on the
 * ang_mid ternary + 3-alt PERM_GENERAL on the *arg2/*arg3 write order.
 *
 * REJECTED as pointer-alias holder — sixth chassis to converge on the SAME
 * cheat class (block-scope-alias-p1, permuter-long-new_var2-p1-alias [s4],
 * permuter-s16-new_var2-p1-alias [s5], permuter-s16-new_var2-p2-alias [s13],
 * permuter-s14-p1-shift-cast-alias [s14]). The neither-ternary-nor-write-
 * order alternative was selected in the closing form; the closer is purely
 * the PERM_RANDOMIZE-injected `s16 *new_var2` local holding the p1 pointer
 * expression, then `p = new_var2;`.
 *
 * Cheat vector: no semantic purpose (a programmer writes `p = <expr>;`);
 * dead in emitted output; GCC-internals-only justification (RTL pseudo
 * lifetime shift to steer expand_preferences propagation into pseudo 86's
 * pref set). Closed by [[no-new-park-categories]] and Judge s10 binding.
 *
 * Consequence: permuter modality is exhausted across SIX structurally
 * distinct chassis (s4 undirected random; s5-1 directed PERM_GENERAL on
 * prev_idx/p1/p2; s5-2 randomize overlay of same; s13 downstream ternary
 * only; s14 shift-spelling + table-source; s22 downstream-ternary +
 * *arg2/*arg3 write-order). Every closing basin discovered by the permuter
 * across all six axes is the same alias-holder cheat family.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s16 *new_var2;
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

    if ((s16)prev_idx < 0) { prev_idx = arg0[3] - 1; }

    {
        s32 tmp = arg1 + 1;
        next_idx = tmp;
        if ((s16)tmp >= (s32)arg0[3]) { next_idx = 0; }
    }

    /* CHEAT: alias holder — the sole effect is to bind pseudo 86's copy-pref
     *  origin to `new_var2`'s pseudo instead of directly to the address
     *  expression's, shifting RA to pick v0 for p1 dest. Closed by policy. */
    new_var2 = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    p = new_var2;
    ang_prev = single_game_getEnemyCharId(p[0] - (s16)cx, p[1] - (s16)cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16)cx, p[1] - (s16)cy) & 0xFFF;

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
