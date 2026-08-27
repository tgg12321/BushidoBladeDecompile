/* REJECTED s31 2026-08-27 (forensics). formB + the `scale = arg0[2] * 40`
 * computation hoisted ABOVE the two ratan2 calls, to kill arg0's live range
 * across the call and free the 9th callee-save that the cached base needs.
 * MEASURED: 112 instructions -- UNCHANGED from formB. The .hon.s shows the
 * hoist WORKS at the level it was aimed at (arg0 lands in $t0, a call-clobbered
 * register, and is never saved), but `scale` simply takes arg0's place in the
 * live-across-call set, so the count of values live across the call is still 9
 * and $fp is still commandeered as the 9th callee-save. This is the measured
 * proof that the live-across set cannot be shrunk by MOVING a computation:
 * every value the tail needs must cross the call in SOME register. Only
 * eliminating a value (or replacing two values with one, which is what the s31
 * candidate's hoisted next-neighbour address does to next_idx+base) reduces it.
 * Artifacts: tmp/grind/func_80057CC8/s31/v1.hon.s */
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

    scale = arg0[2] * 40;
    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    ni = (s16) next_idx;
    ang_next = ratan2(table[ni * 2] - (s16) cx, table[ni * 2 + 1] - (s16) cy) & 0xFFF;

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(new_var = *(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
