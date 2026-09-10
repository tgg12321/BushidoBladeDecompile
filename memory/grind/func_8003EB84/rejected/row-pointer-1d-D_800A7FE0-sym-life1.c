/* REJECTED (s1, 2026-09-10): row = &D_800A7FE0[t4*0x20] (1-D decl): expand emits the (mult t4 64) shift BEFORE the symbol load, sym life 1 -> loop.c 'not desirable' (58*1*1 < 90), sym stays in-loop and the t4*64+sym giv is not formed; score 49. The 2-D decl D_800A7FE0[32][32] is what forms the t6/t3 givs. */
s32 *func_8003EB84(s32 a0, s32 a1, s32 *out) {
    s32 sp[0x21];
    s32 mask;
    s32 i;
    s32 t4;
    s32 t1;
    s32 t2;
    s16 *row;
    s16 temp_v0;
    s32 va1;
    s16 vidx;
    u16 t0;
    s32 v1;
    s32 a3;
    u8 *e;
    u8 *e2;
    u8 *base;
    u8 e7val;
    s32 *list;
    s32 vflag;

    if (a0 >= 0) {
        mask = -1;
        if (a0 >= 0x20) {
            goto skip;
        }
        goto calc;
    } else {
        mask = -1;
        if (-a0 < 0x20) {
        calc:
            if (a0 < 0) {
                mask = -1U >> (a0 + 0x1F);
            } else if (a0 == 0) {
                mask = 0;
            } else {
                mask = -1 << (0x20 - a0);
            }
        }
    }
skip:
    for (i = 0; i < 0x20; i++) {
        if (i >= a1 && i < a1 + 0x1F) {
            sp[i] = mask;
        } else {
            sp[i] = -1;
        }
    }

    for (t4 = 0; t4 < 0x20; t4++) {
        t2 = sp[t4];
        if (t2 != 0) {
            row = &D_800A7FE0[t4 * 0x20];
            for (t1 = 0; t1 < 0x20; t1++) {
                if (t2 < 0) {
                    temp_v0 = row[t1];
                    va1 = temp_v0;
                    if (temp_v0 >= 0) {
                        v1 = t4 << 5;
                        a3 = D_800A8FB0[v1 + t1];
                        vidx = va1;
                        do {
                            t0 = D_800A87E0[vidx];
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
            }
        }
    }

    return out;
}
