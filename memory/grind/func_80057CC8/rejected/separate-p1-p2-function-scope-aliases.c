/* REJECTED 2026-06-15 — cheat-by-spelling (function-scope alias variants of `p`).
 * Sweep variant v17. Replaces the single function-scope `s16 *p` with two
 * function-scope locals `s16 *p1`, `s16 *p2`. Same intent as the block-scope
 * `p1` form (rejected/block-scope-alias-p1.c) — distinct locals so GCC's RA
 * treats them with different interference patterns vs reusing one variable.
 * Sandbox --disable all == 0.
 *
 * Same fault profile: the function behaves identically whether one `p` is reused
 * or two distinct `p1`, `p2` are used (the second write to `p` is never read
 * across the boundary because both assignments precede their dereferences). The
 * naming pattern (`p1`, `p2`) is a numeric-suffix variant of `p` that announces
 * codegen-role distinction with no problem-domain content; a programmer with
 * semantic motivation would name them `prev_p` and `next_p`. Worker
 * self-rejected without invoking cheat-reviewer. */
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
    s16 *p1;
    s16 *p2;
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

    p1 = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p1[0] - (s16) cx, p1[1] - (s16) cy) & 0xFFF;
    p2 = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p2[0] - (s16) cx, p2[1] - (s16) cy) & 0xFFF;

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
