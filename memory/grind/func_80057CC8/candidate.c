/* candidate floor: sandbox --disable all = 3 (masked). HEAD floor = 9.
 * Lever: reassociate BOTH p-pointer adds to (offset + table) instead of
 * (table + offset). That alone took the masked distance 9 -> 3.
 * The s3 pin is REMOVED here (COMPLETED-C requires zero pins); masking can't
 * see the pin so the masked floor is identical (3) with or without it — the
 * real test of whether GCC naturally picks s3 for next_idx is the full SHA1.
 * Remaining gap (the 3): first p's pointer addu does NOT coalesce its offset
 * temp into the dest register (target: `addu v0,v0,a2` reusing v0; ours:
 * `addu v1,v0,a2` allocating v1), which cascades to the two `lh` base regs.
 * The second p coalesces correctly; the asymmetry is the `if((s16)prev_idx<0)`
 * reload giving prev_idx a two-def (phi) live range. */
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

    p = (s16 *)((((s32)(prev_idx << 16) >> 16) << 2) + (s32)table);
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
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
