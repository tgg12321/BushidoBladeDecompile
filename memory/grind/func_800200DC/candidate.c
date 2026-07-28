/* func_800200DC — floor-14 candidate (matches src/code6cac.c HEAD form, s1 re-verified) */
void func_800200DC(s32 *arg0, s32 *arg1, s32 arg2, s32 arg3, s32 *arg4) {
    s32 dx = arg1[0] - arg0[0];
    s32 dz = arg1[2] - arg0[2];
    s32 dist = func_8007E11C(dx * dx + dz * dz);

    if (dist == 0) {
        arg4[2] = 0;
        arg4[0] = 0;
        return;
    }

    {
        s32 dy = arg1[1] - arg0[1];

        if (dy == 0) {
            s32 neg = -arg3;
            s32 denom = arg2 * 2;
            arg4[0] = (neg * dx) / denom;
            arg4[2] = (neg * dz) / denom;
        } else {
            s32 dy2 = dy * 2;
            s32 disc = arg2 * arg2 + arg3 * dy2;
            s32 a0;

            if (disc >= 0) {
                s32 sq = func_8007E11C(disc << 10);
                s32 a2 = arg2 << 5;
                a0 = ((a2 + sq) * dist) / dy2 / 32;

                if (a0 < 0) {
                    a0 = ((a2 - sq) * dist) / dy2 / 32;
                }
            } else {
                a0 = 300;
            }

            if (a0 >= 301) {
                a0 = 300;
            }

            arg4[0] = (a0 * dx) / dist;
            arg4[2] = (a0 * dz) / dist;
        }
    }
}
