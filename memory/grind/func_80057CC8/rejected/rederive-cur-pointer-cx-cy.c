/* s18 rederive KILLED — score 4 (regression +1 from candidate baseline 3).
 * Novel rederive angle from m2c fresh decompile: introduce a `u16 *cur`
 * pointer local for cx/cy reads instead of inlining the address twice.
 *
 *     u16 *cur = (u16 *)((s32)table + arg1 * 4);
 *     cx = cur[0];
 *     cy = cur[1];
 *
 * m2c produces `temp_v0 = (arg1*4) + temp_a2; temp_s4 = *(u16*)temp_v0;` —
 * the shape treats the current node's address as a stored intermediate.
 *
 * Result: sandbox --disable all = 4 (target=build=111 insns). The extra
 * pointer local introduces a fresh pseudo whose live range interferes with
 * the p1/p2 addu scheduling window; codegen regresses by 1 vs candidate.
 * Not a cheat-by-spelling (`cur` is a genuine semantic intermediate for
 * one node's address, unlike alias-p1 which held the SAME address `p`
 * points to) — just measurably worse.
 */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev, ang_next, ang_mid;
    s32 scale, base, half;
    u16 cx, cy;
    s16 new_var;
    s16 *p;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    {
        u16 *cur = (u16 *)((s32)table + arg1 * 4);
        cx = cur[0];
        cy = cur[1];
    }
    if ((s16) prev_idx < 0) { prev_idx = arg0[3] - 1; }
    { s32 tmp = arg1 + 1; next_idx = tmp; if ((s16) tmp >= (s32)arg0[3]) next_idx = 0; }
    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    if (ang_next < ang_prev) { base = ang_prev + 0x800; half = (s32)(ang_prev - ang_next) / 2; ang_mid = base - half; }
    else { ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev; }
    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
