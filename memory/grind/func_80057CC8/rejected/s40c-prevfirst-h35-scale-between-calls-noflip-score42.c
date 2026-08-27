/* REJECTED s40c -- score 42 at 108 insns.  Same h35 prev-test-first layout as
 * s40c-prevfirst-h35-scale-after-angnext-noflip.c but with `scale = arg0[2] * 40;`
 * placed BETWEEN the two ratan2 calls, the earliest placement that still leaves arg0
 * crossing a call.  This is the SHORTEST arg0 live range reachable in the prev-first
 * regime: arg0 72 = 5 refs / 37 insns -> priority 2*5/37 = 0.2703, against the address
 * pseudo 88 = 4 refs / 29 insns -> 2*4/29 = 0.2759.  The seat still does NOT flip --
 * the prev-first regime misses the target's $s2/$s3 map by TWO PERCENT of one
 * allocno_compare priority, and both live ranges shrink together (arg0's range strictly
 * contains the address's), so no further statement move can widen the ratio. */
void func_80057CC8(u8 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    unsigned short prev_idx;
    s32 ang_prev;
    s32 ang_next;
    s32 ang_mid;
    s32 scale;
    s32 base;
    s32 half;
    u16 cx;
    s16 new_var;
    s32 pi;
    u16 cy;
    s16 *table;
    s16 *next_vert;

    prev_idx = arg1 - 1;
    table = *(s16 **)(arg0 + 4);
    next_vert = (s16 *)((arg1 + 1) * 4 + (s32)table);
    cx = *(u16 *)((s32)table + arg1 * 4 + 0);
    cy = *(u16 *)((s32)table + arg1 * 4 + 2);

    if ((s16) prev_idx < 0) {
        prev_idx = arg0[3] - 1;
    }

    {
        s32 tmp = arg1 + 1;
        if ((s16) tmp >= (s32)arg0[3]) {
            next_vert = table;
        }
    }

    pi = (s16) prev_idx;
    ang_prev = ratan2(table[pi * 2] - (s16) cx, table[pi * 2 + 1] - (s16) cy) & 0xFFF;
    scale = arg0[2] * 40;
    ang_next = ratan2(next_vert[0] - (s16) cx, next_vert[1] - (s16) cy) & 0xFFF;

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
