void func_80022224(s32 arg0, s32 *arg1, s32 *arg2) {
    s32 dists[6];
    s16 *base;
    s16 *p;
    s32 *d;
    s32 dx;
    s32 dz;
    s32 i;
    s32 best;
    s32 *r;
    s32 *w;

    base = (s16 *)(stage_GetDataPtr() + (D_800A36A4 * 3) * 0x10);
    i = 0;
    p = base;
    d = dists;
    do {
        dx = p[3] - arg2[0];
        dz = p[5] - arg2[2];
        *d = dx * dx + dz * dz;
        i++;
        d++;
        p += 6;
    } while (i < 4);

    best = 0;
    for (i = 1; i < 4; i++) {
        if (dists[i] < dists[best]) {
            best = i;
        }
    }
    dists[best] = -1;

    best = 0;
    for (i = 1; i < 4; i++) {
        if (dists[i] > dists[best]) {
            best = i;
        }
    }
    dists[best] = -1;

    r = dists;
    i = 0;
    {
        s32 stop = -1;
        w = r;
        for (; i < 4; i++) {
            if (*r != stop) {
                w[4] = i;
                w++;
            }
            r++;
        }
    }

    base += dists[4 + (func_80079154() & 1)] * 6 + 3;
    arg1[0] = base[0];
    arg1[1] = base[1];
    arg1[2] = base[2];
}
