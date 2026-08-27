/* REJECTED s32 — score 42 at 110 insns. Structural re-derivation: compute the
 * next-neighbour dx/dy BEFORE the first ratan2 call so the sign-extended centre twins
 * die before the call and only SEVEN values cross it. It does what it claims (the twin
 * pair leaves the live-across set) but GCC then folds the centre loads into `lh` and
 * the whole body diverges. KILLED — shrinking the live-across set below the target's 8
 * is strictly counterproductive. */
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
    s32 pi;
    s32 ni;
    u16 cy;
    s16 *table;
    s16 *next_vert;

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

    pi = (s16) prev_idx;
    ni = (s16) next_idx;
    next_vert = &table[ni * 2];
    {
        s32 dxn = next_vert[0] - (s16) cx;
        s32 dyn = next_vert[1] - (s16) cy;
        ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
        ang_next = ratan2(dxn, dyn) & 0xFFF;
    }

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
