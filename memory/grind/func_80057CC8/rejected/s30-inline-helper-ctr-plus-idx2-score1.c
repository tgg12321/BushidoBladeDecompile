static inline s32 vert_angle_57CC8(u8 *arg0, s32 idx, s16 cx, s16 cy) {
    s16 *t = *(s16 **)(arg0 + 4);
    return ratan2(t[idx * 2] - cx, t[idx * 2 + 1] - cy) & 0xFFF;
}

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
    u16 cy;

    prev_idx = arg1 - 1;
    {
        s16 *ctr = *(s16 **)(arg0 + 4) + arg1 * 2;
        cx = ctr[0];
        cy = ctr[1];
    }

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

    ang_prev = vert_angle_57CC8(arg0, (s16) prev_idx, (s16) cx, (s16) cy);
    ang_next = vert_angle_57CC8(arg0, (s16) next_idx, (s16) cx, (s16) cy);

    if (ang_next < ang_prev) {
        base = ang_prev + 0x800;
        half = (s32)(ang_prev - ang_next) / 2;
        ang_mid = base - half;
    } else {
        ang_mid = ((s32)(ang_next - ang_prev) / 2) + ang_prev;
    }

    scale = arg0[2] * 40;
    *arg2 = cx + ((scale * (s32)(*(&Judge + (ang_mid & 0xFFF)))) >> 12);
    *arg3 = cy + ((scale * (s32)(*(&Judge + (((s16)ang_mid + 0x400) & 0xFFF)))) >> 12);
}
