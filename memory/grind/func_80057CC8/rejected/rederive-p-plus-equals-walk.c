/* REJECTED 2026-07-19 (s9 rederive) — REGRESSION 3 -> 7.
 * Form: `p = table; p += (s16)prev_idx * 2;` for both slots.
 * Rationale for trying: fresh rederive angle — walking-pointer shape reinterprets
 * the two adds as a semantic table walk (init base then advance to element).
 * Result: sandbox --disable all == 7 (from baseline 3). GCC 2.7.2 does NOT fold
 * `p = base; p += off;` back to `p = base + off;` cleanly; the two-SET form
 * produces additional insns and worse RA. Not a lever. KILLED.
 * See s9 ledger; hypothesis KILLED — pointer-arith walking-pointer restructure
 * is not a valid rederive-modality axis for this function. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    unsigned short next_idx;
    s32 ang_prev, ang_next, ang_mid, scale, base, half;
    u16 cx, cy;
    s16 new_var;
    s16 *p;
    s16 *table;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) prev_idx = arg0[3] - 1;
    { s32 tmp = arg1 + 1; next_idx = tmp;
      if ((s16) tmp >= (s32)arg0[3]) next_idx = 0; }

    p = table;
    p += (s16)prev_idx * 2;
    ang_prev = single_game_getEnemyCharId(p[0] - (s16) cx, p[1] - (s16) cy) & 0xFFF;
    p = *(s16 **)(arg0 + 4);
    p += (s16)next_idx * 2;
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
