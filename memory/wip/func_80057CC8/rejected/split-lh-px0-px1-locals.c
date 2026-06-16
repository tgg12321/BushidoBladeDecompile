/* REJECTED 2026-06-15 — cheat-by-spelling (duplicated address-expression coercion).
 * Sweep variant v15. Splits the two s16 reads `p[0]`, `p[1]` into named locals
 * `px0`, `px1`, each computed from a DUPLICATED `(s16 *)((((s32)(prev_idx << 16)
 * >> 16) << 2) + (s32)table)` cast expression (px1's address uses `+ 2`). The
 * full address-cast subexpression is duplicated; CSE eliminates it in emitted
 * asm. Sandbox --disable all == 0.
 *
 * Same fault as [[duplicate-address-expr-pseudo-inline]]: the duplication has no
 * semantic purpose; it shapes RTL so GCC's RA reuses the offset register for
 * the load-base instead of allocating fresh. The px0/px1 locals are SOTN-style
 * named intermediates, but the underlying address-expression DUPLICATION is the
 * coercion — the locals only spell it differently. Worker self-rejected without
 * invoking cheat-reviewer. */
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
    s16 px0, px1;

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

    px0 = *(s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    px1 = *(s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table + 2);
    ang_prev = single_game_getEnemyCharId(px0 - (s16) cx, px1 - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

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
