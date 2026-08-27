/* NOT A CHEAT REJECTION -- banked s31 2026-08-27 as the measured TWIN of the
 * s31 candidate. Both neighbour vertex addresses (prev and next) formed above
 * the calls, instead of only the next one. MEASURED: 110 instructions, same
 * regime and same live-across-call set as the candidate; kept out of candidate.c
 * only because hoisting the PREV address as well buys nothing (its consumer is
 * the very next statement) and adds a second local for no measured effect.
 * Artifacts: tmp/grind/func_80057CC8/s31/v5.hon.s */
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
    s16 *pp;
    s16 *np;

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
    pp = table + pi * 2;
    np = table + ni * 2;
    ang_prev = ratan2(pp[0] - (s16) cx, pp[1] - (s16) cy) & 0xFFF;
    ang_next = ratan2(np[0] - (s16) cx, np[1] - (s16) cy) & 0xFFF;

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
