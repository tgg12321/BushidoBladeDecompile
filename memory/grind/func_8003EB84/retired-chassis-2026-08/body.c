s32 *func_8003EB84(s32 a0, s32 a1, s32 *out) {
    s32 sp[0x21];
    s32 mask;
    s32 *p;
    s32 *lo_end;
    s32 *hi_start;
    s32 cmp;
    s32 t4;
    s32 t1;
    s32 t2;
    s16 *t6;
    s16 *t3;
    s32 *t5;
    s16 temp_v0;
    s32 va1;
    s16 vidx;
    u16 t0;
    s32 v1;
    u8 a3;
    u8 *e;
    u8 *e2;
    u8 *base;
    u8 e7val;
    s32 *list;
    s32 vflag;

    if (a0 >= 0) {
        mask = -1;
        if (a0 < 0x20) {
            goto block_4;
        }
    } else {
        mask = -1;
        if (-a0 < 0x20) {
        block_4:
            if (a0 < 0) {
                mask = -1U >> (a0 + 0x1F);
            } else if (a0 == 0) {
                mask = 0;
            } else {
                mask = -1 << (0x20 - a0);
            }
        }
    }

    p = sp;
    lo_end = &sp[a1];
    hi_start = &sp[a1 + 0x1F];
    cmp = (s32)p < (s32)lo_end;
    do {
        if (cmp == 0 && (s32)p < (s32)hi_start) {
            *p = mask;
        } else {
            *p = -1;
        }
        p++;
        cmp = (s32)p < (s32)lo_end;
    } while ((s32)p < (s32)&sp[0x20]);

    t4 = 0;
    t6 = D_800A7FE0;
    t5 = sp;
    do {
        t2 = *t5;
        if (t2 != 0) {
            t1 = 0;
            t3 = t6;
            do {
                if (t2 < 0) {
                    temp_v0 = *t3;
                    va1 = temp_v0;
                    if (temp_v0 >= 0) {
                        v1 = t4 << 5;
                        a3 = D_800A8FB0[v1 + t1];
                        vidx = va1;
                        do {
                            base = (u8 *)&D_800A87E0[vidx];
                            t0 = *(u16 *)base;
                            v1 = t0 & 0x7FFF;
                            va1 += 1;
                            if (v1 < D_800A3368) {
                                base = D_800A4750;
                                e = base + v1 * 0x10;
                                e[6] = a3 & 3;
                                if ((a3 & 8) || ((a3 & 4) && (e[7] & 8))) {
                                    e7val = e[7] | 1;
                                } else {
                                    e7val = e[7] & 0xFE;
                                }
                                e[7] = e7val;
                                list = (s32 *)D_800A3820;
                                D_800A3820 = (s32)(list + 1);
                                *list = (s32)e;
                                goto block_30;
                            }
                            base = D_800A6690;
                            e2 = base + (v1 - D_800A3368) * 0x68;
                            vflag = t0 & 0x8000;
                            if (e2[0x58] == 0) {
                                *out = (s32)e2;
                                out += 1;
                                vflag = 1;
                                e2[0x58] = vflag;
                            block_30:
                                vflag = t0 & 0x8000;
                            }
                            vidx = va1;
                        } while (vflag == 0);
                    }
                }
                t2 = t2 << 1;
                t1 += 1;
                t3 += 1;
            } while (t1 < 0x20);
        }
        t6 += 0x20;
        t4 += 1;
        t5 += 1;
    } while (t4 < 0x20);

    return out;
}
