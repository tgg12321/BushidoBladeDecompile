/* s11 F1 variant A: duplicated-statement-into-arms applied to prev_idx if.
 * Result: sandbox --disable all = 12 (regression from candidate baseline 3).
 * Reason: moves p1 assignment BEFORE next_idx block. Matches s3's rejected
 * hoist-p1-before-nextidx-block score-12 kill. Cross-jump did NOT merge the
 * duplicated arms byte-neutrally — p1's assignment out of its scheduling
 * window kills delay-slot fill regardless of duplication.
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
        p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table); /* FAKE */
    } else {
        p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table); /* FAKE */
    }
    { s32 tmp = arg1 + 1; next_idx = tmp; if ((s16) tmp >= (s32)arg0[3]) next_idx = 0; }
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) { base = ang_prev + 0x800; half = (s32)(ang_prev - ang_next) / 2; ang_mid = base - half; }
    else ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
