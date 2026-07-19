/* s11 F1 variant B: split next_idx into if/else, duplicate p1 into both arms.
 * Keeps p1 in its post-next_idx scheduling window position.
 * Result: sandbox --disable all = 8 (regression from candidate baseline 3).
 * Reason: rewriting the next_idx `default; if(...) override;` shape into a
 * proper if/else disrupts sched1's delay-slot fill; the duplicated p1 does
 * NOT compensate. Cross-jump does not merge (score up, not neutral).
 * KILLED. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev, ang_next, ang_mid, scale, base, half;
    u16 cx, cy;
    s16 new_var, *p, *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_idx = 0;
            p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table); /* FAKE */
        } else {
            next_idx = tmp;
            p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table); /* FAKE */
        }
    }
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) { base = ang_prev + 0x800; half = (s32)(ang_prev - ang_next) / 2; ang_mid = base - half; }
    else ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
