/* s11 F1 variant C: duplicate `table = *(s16**)(arg0+4);` into both arms of
 * prev_idx if (natural [[split-read-defeats-hoist]]-style pattern).
 * Result: sandbox --disable all = 3 (byte-neutral, no change from baseline).
 * Reason: GCC's cse1 pass merges the two redundant reloads back into one
 * before global-alloc sees them; reg_n_refs on table's pseudo is NOT bumped.
 * The duplicated statement disappears at CSE, so no priority lift reaches
 * pseudo 86's allocation. Not a lever.
 * KILLED (measured byte-neutral). */
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
        table = *(s16 **)(arg0 + 4); /* FAKE */
    } else {
        table = *(s16 **)(arg0 + 4); /* FAKE */
    }
    { s32 tmp = arg1 + 1; next_idx = tmp; if ((s16) tmp >= (s32)arg0[3]) next_idx = 0; }
    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = (s16 *)((((s32)(next_idx << 16) >> 16) << 2) + (s32)(*(s16 **)(arg0 + 4)));
    ang_next = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) { base = ang_prev + 0x800; half = (s32)(ang_prev - ang_next) / 2; ang_mid = base - half; }
    else ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
