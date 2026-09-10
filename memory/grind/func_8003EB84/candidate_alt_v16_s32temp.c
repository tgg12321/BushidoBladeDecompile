/* ALT (s1): identical to candidate.c except `s32 temp_v0` and `s32 sp[0x21]`; scores 12 (register permutation vidx/a3/t0 only, 143 insns). Kept as the closest structural sibling for comparison. */
s32 *func_8003EB84(s32 a0, s32 a1, s32 *out) {
    s32 sp[0x21];
    s32 mask;
    s32 i;
    s32 t4;
    s32 t1;
    s32 t2;
    s32 temp_v0;
    s16 vidx;
    u16 t0;
    s32 v1;
    s32 a3;
    u8 *e;
    u8 *e2;
    s32 *list;

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
            for (t1 = 0; t1 < 0x20; t1++) {
                if (t2 < 0) {
                    temp_v0 = D_800A7FE0[t4][t1];
                    vidx = temp_v0;
                    if (temp_v0 >= 0) {
                        a3 = D_800A8FB0[t4 * 0x20 + t1];
                        do {
                            t0 = D_800A87E0[vidx++];
                            v1 = t0 & 0x7FFF;
                            if (v1 < D_800A3368) {
                                e = &D_800A4750[v1 * 0x10];
                                e[6] = a3 & 3;
                                if ((a3 & 8) || ((a3 & 4) && (e[7] & 8))) {
                                    e[7] |= 1;
                                } else {
                                    e[7] &= 0xFE;
                                }
                                list = (s32 *)D_800A3820;
                                D_800A3820 = (s32)(list + 1);
                                *list = (s32)e;
                            } else {
                                e2 = &D_800A6690[(v1 - D_800A3368) * 0x68];
                                if (e2[0x58] == 0) {
                                    *out = (s32)e2;
                                    out += 1;
                                    e2[0x58] = 1;
                                }
                            }
                        } while (!(t0 & 0x8000));
                    }
                }
                t2 = t2 << 1;
            }
        }
    }

    return out;
}
